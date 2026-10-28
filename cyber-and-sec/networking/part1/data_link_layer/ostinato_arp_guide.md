# Discovering MAC Addresses via ARP Using Ostinato

**Objective:** Manually craft a custom ARP request using Ostinato (a network
packet crafter) to discover the physical MAC address of a target machine on the
LAN, using only its known IP address.

## Theory: How Do We Get a MAC from an IP?

Network switches (Layer 2) only understand MAC addresses, but applications
communicate using IP addresses (Layer 3). To bridge this gap, the **Address
Resolution Protocol (ARP)** is used.

To find a machine's MAC address from its IP, you send an **ARP Request** — a
broadcast that asks: *"Who has IP `10.1.x.x`? Send your MAC to me."* The
target responds with an **ARP Reply** containing its MAC. In this exercise, we
forge this request manually instead of letting the OS handle it.

## Step 1: Configure the Forged Packet in Ostinato

Launch Ostinato and create a new stream, built layer by layer:

### Layer 2 — Ethernet II

| Field               | Value                                    |
|---------------------|------------------------------------------|
| Destination MAC     | `FF:FF:FF:FF:FF:FF` (Broadcast)          |
| Source MAC          | Your machine's real MAC address          |
| EtherType           | `0x0806` (ARP)                           |

### ARP Layer

| Field       | Value                                            |
|-------------|--------------------------------------------------|
| Opcode      | `1` (ARP Request)                                |
| Sender MAC  | Your MAC address                                 |
| Sender IP   | Your IP address                                  |
| Target MAC  | `00:00:00:00:00:00` (unknown — what we seek)     |
| Target IP   | The known IP address of the target               |

## Step 2: Transmit the Packet

1. Apply the configuration to the stream in Ostinato.
2. Start Wireshark on the relevant network interface.
3. Click **Play/Transmit** to inject the packet into the network.

## Step 3: Observe the Result in Wireshark

- **Outgoing Request:** Your ARP Request appears with Info:
  `Who has 10.1.x.x? Tell 10.1.x.x`. Destination is Broadcast.
- **Incoming Reply:** The target responds with an ARP Reply. Select it and
  expand the **Address Resolution Protocol (reply)** section. The
  **Sender MAC address** field contains the target's physical MAC address.

## Step 4: Practical Result

Based on execution of this exercise:

- **Target IP:** `10.1.x.x`
- **Discovered MAC:** `40:c2:ba:XX:XX:XX` (Vendor: Compal Information)

We successfully resolved a Layer 3 IP address to a Layer 2 MAC address,
fulfilling the core objective.
