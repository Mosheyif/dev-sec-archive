# Network Analysis: Ping, ARP, and Cache Manipulation

This document explains the technical flow of a ping operation between two hosts
on the same local network, analyzes the relevant protocol headers, and discusses
the impact of ARP cache manipulation.

## 1. The Ping Flow: ARP and ICMP

When you ping a neighbor's IP address (e.g., `10.10.10.5`), your computer
follows these steps:

1. **ARP Request:** Your computer checks its ARP Cache. If the MAC address for
   `10.10.10.5` is unknown, it broadcasts: *"Who has `10.10.10.5`? Tell
   `YourIP`."*
2. **ARP Reply:** The neighbor receives the broadcast, updates its cache, and
   responds (unicast): *"`10.10.10.5` is at `NeighborMAC`."*
3. **ICMP Echo Request:** Now that the MAC address is known, your computer sends
   the ICMP Echo Request.
4. **ICMP Echo Reply:** The neighbor responds.

## 2. Protocol Header Breakdown

### A. Ethernet II Header (Layer 2)

| Field           | Size   | Description                                              |
|-----------------|--------|----------------------------------------------------------|
| Destination MAC | 6 B    | Neighbor's MAC or Broadcast `FF:FF:FF:FF:FF:FF`          |
| Source MAC      | 6 B    | Your computer's MAC                                      |
| EtherType       | 2 B    | Protocol: `0x0806` (ARP) or `0x0800` (IPv4)              |

### B. ARP Header (Layer 2.5)

| Field                 | Description                                         |
|-----------------------|-----------------------------------------------------|
| Hardware Type         | e.g., Ethernet                                      |
| Protocol Type         | e.g., IPv4                                          |
| Opcode                | `1` = Request, `2` = Reply                          |
| Sender/Target MAC+IP  | Fills ARP tables on both ends                       |

### C. IPv4 Header (Layer 3)

| Field       | Description                              |
|-------------|------------------------------------------|
| Version     | `4`                                      |
| TTL         | Prevents infinite looping                |
| Protocol    | `1` (ICMP)                               |
| Source IP   | Your IP                                  |
| Destination | Target IP                                |

### D. ICMP Header (Layer 3/4)

| Field    | Description                                        |
|----------|----------------------------------------------------|
| Type     | `8` (Echo Request) or `0` (Echo Reply)             |
| Code     | `0`                                                |
| Checksum | Ensures data integrity                             |

## 3. ARP Cache Manipulation

The ARP Cache is a temporary table mapping IP addresses to MAC addresses.

```bash
# View the cache
arp -a

# Flush the cache
arp -d *
```

### Experiment: Clearing the Cache

1. Run `arp -d *` to flush all mappings.
2. Start Wireshark and filter by `arp`.
3. Perform the ping again.

**Observation:** An ARP Request appears immediately before the first ICMP Echo
Request. Because the cache was empty, the computer broadcasts to re-learn the
MAC address before sending the ICMP packet.

## Summary

The separation of Layer 2 (ARP/Ethernet) and Layer 3 (IP/ICMP) is essential
for local network communication. ARP resolves physical addresses; ICMP verifies
connectivity. Clearing the ARP cache forces re-discovery, proving that IP-to-MAC
resolution is a prerequisite for Layer 3 data transmission.
