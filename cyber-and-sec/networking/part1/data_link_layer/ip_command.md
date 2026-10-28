
# How-To Guide: Extracting Network Interface Data in Linux

**Objective:** Understand and utilize the `ip` command to retrieve essential network parameters across OSI Layers 2 and 3 (Physical MAC addresses, Logical IP addresses, and Subnet masks) for all network interfaces on the machine.

---

## Step 1: Running the Command

Open your terminal and execute the following command:

```bash
ip a

```

*(Note: The command `ip a` is shorthand for `ip address show`)*

This command will output a list of all network interfaces (both virtual and physical) currently configured on the machine, along with their respective network data.

---

## Step 2: Decoding the Output (Data Extraction Guide)

The output is divided into numbered blocks, where each block represents a different network interface. Within each block, look for these two key lines to extract the required data:

1. **Layer 2 (Data Link):** Look for the line starting with **`link/ether`**. The sequence immediately following it is the physical **MAC address**.
2. **Layer 3 (Network):** Look for the line starting with **`inet`** (for IPv4). The sequence immediately following it is the logical **IP address**, and the number after the slash (`/`) is the **Subnet mask** (in CIDR notation).

---

## Practical Examples (Based on System Output):

Below is the extracted data from a machine connected to both wired and wireless networks simultaneously:

### 1. Wired Network Interface (Ethernet)

In our output, this interface is identified as `enx0c3796XXXXXX`.
Extracting data from its respective block yields:

- **MAC Address (Physical):** `0c:37:96:XX:XX:XX`
- **IP Address (Logical):** `10.1.x.x`
- **Subnet Mask:** `/16` (Decimal equivalent: `255.255.0.0`)

### 2. Wireless Network Interface (Wi-Fi)

In our output, this interface is identified as `wlo1`.
Extracting data from its respective block yields:

- **MAC Address (Physical):** `38:fc:98:XX:XX:XX`
- **IP Address (Logical):** `10.10.x.x`
- **Subnet Mask:** `/22` (Decimal equivalent: `255.255.252.0`)

### 3. Internal Interface (Loopback)

Every machine has a virtual interface named `lo`, used strictly for internal
communication (localhost):

- **MAC Address:** `00:00:00:00:00:00` (Virtual, no physical hardware)
- **IP Address:** `127.0.0.1`
- **Subnet Mask:** `/8`
