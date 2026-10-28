

# Network Intrusion Detection: Suricata IDS & Evasion

**Overview:** 
This technical guide provides a step-by-step walkthrough for deploying the
Suricata Intrusion Detection System (IDS), authoring custom signatures across
multiple network layers, validating alerts via synthetic traffic generation,
and analyzing signature bypass techniques using encrypted and uniquely encoded
payloads.

---

## 1. Installation and Environment Configuration

**Objective:** Install the Suricata engine, initialize the rule repositories,
and bind the capture threads to the active egress network interface.

### Package Installation and Rule Initialization
Install the Suricata daemon and run the update utility to compile the default
community ruleset into `/var/lib/suricata/rules/suricata.rules`.

```bash
student@host:~$ sudo apt update && sudo apt install suricata -y
student@host:~$ sudo suricata-update
```

### Identify the Active Egress Interface
Determine which physical or virtual Network Interface Card (NIC) the Linux
kernel utilizes to route outbound traffic:

```bash
student@host:~$ ip route show default
```

**Output:**

```text
default via 10.1.0.1 dev eth0 proto dhcp metric 100
```

### Master YAML Configuration (`/etc/suricata/suricata.yaml`)
When Suricata runs with the default `--af-packet` flag, it parses the master
YAML file to allocate capture sockets and load signature files. Open
`/etc/suricata/suricata.yaml` and configure three critical blocks:

1. **Subnet Definition (`HOME_NET`):** Ensure your host's local subnet (e.g.,
   `10.0.0.0/8` or `192.168.0.0/16`) is included in the `HOME_NET` array.
2. **Interface Binding & Checksum Bypass (`af-packet`):** Locate the
   `af-packet:` block, set your active interface, and disable checksum
   validation:

```yaml
af-packet:
  - interface: eth0
    checksum-checks: no
```

> **Common Pitfall (Hardware Checksum Offloading):**
> When generating synthetic traffic locally (e.g., via `curl` or `ping`), the
> Linux kernel allocates the socket buffer (`sk_buff`) with empty TCP/IP
> checksum fields, delegating the calculation to the physical NIC hardware.
> Because Suricata's `AF_PACKET` socket intercepts frames *before* they reach
> the NIC driver, it sees invalid checksums and silently drops the packets.
> Setting `checksum-checks: no` forces the engine to inspect local egress
> traffic.

3. **Custom Rule File Linkage (`rule-files`):** Suricata will not parse custom
   `.rules` files on disk unless they are explicitly referenced in the
   `rule-files:` array. Append the absolute path to `local.rules`:

```yaml
default-rule-path: /var/lib/suricata/rules
rule-files:
  - suricata.rules
  - /etc/suricata/rules/local.rules
```

---

## 2. Authoring Custom Detection Signatures

**Objective:** Construct custom signatures in `/etc/suricata/rules/local.rules`
to detect malicious patterns across Layer 3 (ICMP), Layer 4 (TCP), and Layer 7
(HTTP/TLS).

**Execution:**
Populate `/etc/suricata/rules/local.rules` with the following multi-layer
signatures (using `\` for line continuation):

```bash
student@host:~$ sudo tee /etc/suricata/rules/local.rules << 'EOF'
# 1. Layer 4: Inbound TCP x86 NOP Sled Shellcode Detection
alert tcp $EXTERNAL_NET any -> $HOME_NET any ( \
    msg:"EXPLOIT x86 NOP Sled Detected"; \
    content:"|90 90 90 90 90 90 90 90|"; \
    classtype:shellcode-detect; sid:1000001; rev:1;)

# 2. Layer 7: Outbound HTTP Malicious User-Agent C2 Beacon
alert http $HOME_NET any -> $EXTERNAL_NET any ( \
    msg:"C2 BEACON Malicious User-Agent"; \
    http.user_agent; content:"MaliciousBot/1.0"; \
    classtype:trojan-activity; sid:1000002; rev:1;)

# 3. Layer 4: TCP SYN/FIN Header Flag Anomaly
alert tcp $EXTERNAL_NET any -> $HOME_NET any ( \
    msg:"SCAN Suspicious TCP SYN-FIN Flags"; \
    flags:SF; classtype:attempted-recon; sid:1000003; rev:1;)

# 4. Layer 7: TLS Malicious Server Name Indication (SNI)
alert tls $HOME_NET any -> $EXTERNAL_NET any ( \
    msg:"C2 BEACON Known Malicious SNI"; \
    tls.sni; content:"c2.com"; \
    classtype:trojan-activity; sid:1000005; rev:1;)

# 5. Layer 7: TLS Malicious JA3 Client Fingerprint
alert tls $HOME_NET any -> $EXTERNAL_NET any ( \
    msg:"C2 BEACON Malicious JA3 Fingerprint"; \
    ja3.hash; content:"e7d705a3286e19ea42f587b344ee6865"; \
    classtype:trojan-activity; sid:1000006; rev:1;)

# 6. Layer 3: Outbound ICMP Echo Request C2 Beacon
alert icmp $HOME_NET any -> $EXTERNAL_NET any ( \
    msg:"C2 BEACON ICMP Payload"; \
    itype:8; content:"BEACON"; \
    classtype:trojan-activity; sid:1000007; rev:1;)
EOF
```

* **Rule Mechanics Breakdown:**
  * `http.user_agent`: Directs the Layer 7 HTTP parser to isolate the
    `User-Agent` header buffer prior to executing string comparison.
  * `flags:SF`: Executes a bitwise evaluation on the TCP header struct to flag
    packets with both SYN and FIN bits asserted simultaneously.
  * `itype:8`: Restricts inspection strictly to outbound ICMP Echo Requests,
    ignoring Echo Replies (`itype:0`) to conserve CPU cycles.

---

## 3. Engine Validation and Synthetic Triggering

**Objective:** Verify that the daemon compiled the custom signatures into its
Aho-Corasick pattern-matching tree and trigger alerts using synthetic packets.

### Validate Configuration and Restart Daemon
Test the YAML configuration for syntax errors, then restart the service to
rebind the `AF_PACKET` sockets and load the new rules into memory:

```bash
student@host:~$ sudo suricata -T -c /etc/suricata/suricata.yaml -v
student@host:~$ sudo systemctl restart suricata
```

### Verify Socket Binding and Rule Compilation
Audit the initialization log to confirm the capture thread bound to the NIC
and compiled your custom Signature IDs (SIDs):

```bash
student@host:~$ sudo grep -i "af-packet" /var/log/suricata/suricata.log
student@host:~$ sudo grep "1000007" /var/log/suricata/suricata.log
```

### Trigger and Monitor Alerts
Open a dedicated terminal to follow the real-time alert log:

```bash
student@host:~$ sudo tail -f /var/log/suricata/fast.log
```

In a second terminal, inject synthetic HTTP and ICMP C2 payloads. For ICMP,
the `-p` flag injects the hexadecimal representation of the ASCII string
`BEACON` (`424541434f4e`) into the raw data buffer:

```bash
# Trigger HTTP User-Agent Rule (sid:1000002)
student@host:~$ curl -A "MaliciousBot/1.0" http://example.com

# Trigger ICMP Beacon Rule (sid:1000007)
student@host:~$ ping -c 1 -p 424541434f4e 8.8.8.8
```

**Observed Output (`/var/log/suricata/fast.log`):**

```text
10/26/2026-13:12:44.007764  [**] [1:1000007:1] C2 BEACON ICMP Payload [**] \
[Classification: A Network Trojan was detected] [Priority: 1] {ICMP} \
10.1.0.20:8 -> 8.8.8.8:0
```

---

## 4. Signature Evasion and Counter-Detection

**Objective:** Analyze how static content rules can be bypassed through
encryption and custom encoding, and refine signatures to detect obfuscated
traffic.

### Bypassing Static ASCII Signatures (`sid:1000007`)
Rule `1000007` relies on an exact ASCII match (`content:"BEACON"`). Threat
actors easily evade static signature matching using two primary methods:

* **Unique Custom Encoding (XOR / Byte Rotation):**
  If the malware encodes its payload before transmission—for example, shifting
  each ASCII byte forward by `0x01` (rotating `BEACON` to `CFBDPO`, hex
  `43464244504f`) or applying a single-byte XOR key—the pattern matcher fails
  to match the expected string.

  ```bash
  # Evasion Test: Sending ROT+1 encoded "BEACON" bypasses sid:1000007
  student@host:~$ ping -c 1 -p 43464244504f 8.8.8.8
  ```

* **Payload Encryption (AES / RC4 / TLS):**
  Encrypting the ICMP payload with a symmetric cipher (like AES-CBC with a
  random IV) produces high-entropy ciphertext that changes with every packet,
  eliminating static byte sequences entirely. Similarly, wrapping HTTP traffic
  inside TLS (HTTPS) encrypts the application headers, blinding the
  `http.user_agent` parser in `sid:1000002`.

### Defensive Adaptation: Detecting Encoded & Encrypted C2
When payloads are encrypted or uniquely encoded, defenders shift from scanning
for readable ASCII strings to targeting protocol anomalies, cryptographic
metadata, and deterministic byte offsets:

**Matching Custom Hex Headers with `offset` and `depth`**

Custom encoding schemes and encrypted C2 protocols frequently rely on static
"magic bytes" or fixed-offset command headers to tell the C2 server how to
decode the packet. If reverse engineering reveals the beacon places the hex
marker `0xDE 0xAD 0xBE 0xEF` exactly 4 bytes into the ICMP payload, we can
constrain the pattern matcher to that exact window:

```bash
student@host:~$ sudo tee -a /etc/suricata/rules/local.rules << 'EOF'
alert icmp $HOME_NET any -> $EXTERNAL_NET any ( \
    msg:"C2 BEACON Custom Hex ICMP Header"; itype:8; \
    content:"|de ad be ef|"; offset:4; depth:8; \
    classtype:trojan-activity; sid:1000008; rev:1;)
EOF
student@host:~$ sudo systemctl restart suricata
```

**Validation Command (4 pad bytes + `deadbeef` magic bytes):**

```bash
student@host:~$ ping -c 1 -p 00000000deadbeef0000000000000000 8.8.8.8
```

**Targeting Unencrypted Handshake Metadata (SNI & JA3)**

When HTTP traffic is encrypted via TLS to bypass `http.user_agent`
inspection, defenders utilize rules `1000005` (`tls.sni`) and `1000006`
(`ja3.hash`). Even though the HTTP payload is encrypted, the initial TLS
`Client Hello` handshake exposes the target domain in the SNI extension and
allows Suricata to hash the client's cipher suite negotiation into a JA3
fingerprint, catching the malware before encryption is established.
