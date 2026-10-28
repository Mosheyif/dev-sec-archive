# Packet Capture and Promiscuous Mode in Wireshark

**Objective:** Learn how to initiate a packet capture using Wireshark and
understand the critical difference between Normal Mode and Promiscuous Mode at
the NIC level.

## Step 1: Initiate a Packet Capture

1. **Launch Wireshark** (often requires `sudo` on Linux to access interfaces).
2. **Select the Interface:** On the welcome screen, choose the interface that
   shows a sparkline (active network activity).
3. **Start Capturing:** Double-click the active interface or click the blue
   "Shark Fin" icon. Wireshark immediately displays captured frames in
   real-time.

## Step 2: Capture Modes — Promiscuous vs. Normal

### Normal Mode (Non-Promiscuous)

Your NIC reads the Destination MAC of every incoming frame and only passes
the packet to the OS if:

- The Destination MAC exactly matches the NIC's own MAC (unicast)
- It is a Broadcast frame (`FF:FF:FF:FF:FF:FF`)
- It is a Multicast frame for a group the host has joined

**Result in Wireshark:** You see only traffic meant for your machine, plus
general network broadcasts.

### Promiscuous Mode ("Sniffer" Mode)

The NIC's hardware filter is disabled. Every frame physically received on the
wire is passed to Wireshark, regardless of Destination MAC.

**Result in Wireshark:** You see your traffic, broadcasts, and traffic
intended for other computers on the network.

> Toggle this in Wireshark via **Capture → Options → Enable promiscuous mode
> on all interfaces**.

## Step 3: The "Modern Switch" Limitation

Promiscuous Mode alone does not guarantee you'll see all network traffic:

- **Hubs** (legacy): all traffic is broadcast to every port — promiscuous mode
  reveals everything.
- **Switches** (modern): learn MAC addresses and create isolated unicast
  connections between ports. Even in promiscuous mode, your NIC only physically
  receives:
  - Your own traffic
  - Broadcast/Multicast traffic
  - Other machines' traffic only if **Port Mirroring (SPAN)** is configured, or
    if an attacker uses **ARP Spoofing** to redirect traffic.

## Step 4: Stop and Save the Capture

1. Click the red **Stop** button (square icon) in the toolbar.
2. Go to **File → Save As…** to save as `.pcapng` for later analysis.

## Step 5: Analyzing Ethernet Headers (Ping & ARP Exercise)

**Scenario:** Pinging a neighbor's IP `10.1.x.x` from our host `10.1.x.x`.

When pinging a local IP, Wireshark captures both ARP (address resolution) and
ICMP (ping) packets.

### ARP Packet

| Field           | Value                                               |
|-----------------|-----------------------------------------------------|
| EtherType       | `0x0806` (ARP)                                      |
| Source MAC      | `0c:37:96:XX:XX:XX` (our machine)                   |
| Destination MAC | `40:c2:ba:XX:XX:XX` (the neighbor's machine)        |

> **Advanced observation:** Typically the initial ARP Request goes to Broadcast
> (`FF:FF:FF:FF:FF:FF`). Here the Destination MAC is a specific unicast address
> because the OS already has the neighbor's MAC in its ARP cache and is sending
> a Unicast ARP Request to verify/refresh before the entry expires.

### ICMP Packet (Echo Request)

| Field           | Value                                               |
|-----------------|-----------------------------------------------------|
| EtherType       | `0x0800` (IPv4)                                     |
| Source MAC      | `0c:37:96:XX:XX:XX` (our machine)                   |
| Destination MAC | `40:c2:ba:XX:XX:XX` (the neighbor's machine)        |

The Ethernet header handles node-to-node physical delivery using MAC addresses.
The EtherType field acts as a "traffic sign," directing the payload to the
correct higher-level protocol (ARP vs. IPv4).
