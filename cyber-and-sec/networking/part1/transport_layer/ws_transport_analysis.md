# Network Traffic Analysis Guide (Wireshark)

This guide covers the analysis of common network protocols using Wireshark for
educational and diagnostic purposes.

## 1. TCP Analysis and the Three-Way Handshake

**Wireshark filter:**

```
tcp
```

The TCP Three-Way Handshake establishes a reliable connection. You will see
three sequential packets:

- **SYN:** The client (`192.168.1.10`) initiates the request. The `SYN` flag
  is set in the TCP header.
- **SYN-ACK:** The server (`172.16.5.5`) acknowledges and sends its own
  sequence number. Both `SYN` and `ACK` flags are set.
- **ACK:** The client acknowledges the server's response. The `ACK` flag is
  set.

## 2. UDP and Live Streaming

UDP is connectionless — there is no formal handshake.

**Wireshark filter:**

```
udp
```

When capturing live stream traffic (e.g., streaming sites), packets are
typically encrypted (SRTP or DTLS). You will see a continuous stream of packets,
and the "Data" section in Wireshark will appear as obfuscated binary. You can
still analyze packet size, frequency, and source/destination ports to confirm
the stream is active.

## 3. Non-TCP/UDP Traffic (Layer 3/2)

**Wireshark filter:**

```
!tcp && !udp
```

Some packets do not use a transport layer protocol because they operate at lower
layers (Network or Data Link) and are meant for infrastructure maintenance.

| Protocol | Purpose                               | Filter |
|----------|---------------------------------------|--------|
| ICMP     | Connectivity testing (Ping)           | `icmp` |
| ARP      | Resolves IP addresses to MAC addresses| `arp`  |

These packets do not require a "port" — they are encapsulated in Ethernet frames
and processed directly by the OS network stack or network hardware.

## 4. TCP Keep-Alive Analysis

TCP Keep-Alive packets are sent to ensure a connection stays open during
inactivity.

**Identification in Wireshark:**

- A Keep-Alive packet is typically an empty TCP segment (0 bytes of data).
- It often appears as an `ACK` packet.
- The Acknowledgment number remains the same as the previous segment, and the
  Sequence number often repeats the last byte to provoke an acknowledgement from
  the peer.

---

*Note: All IP addresses and figures used in this guide are for illustrative
purposes only.*
