

# Lab Exercise: VirtualBox Network Address Translation (NAT) Isolation

**Objective**
Establish and analyze network connectivity between two Linux guest virtual machines hosted on separate physical machines using VirtualBox NAT.

**Phase 1: Host Preparation & VirtualBox Installation**
1. **Install VirtualBox:** Ensure both physical host machines have VirtualBox installed. For Linux hosts, install the latest version directly:
   ```bash
   sudo apt update
   sudo apt install virtualbox-7.2

```

2. **Download OS Image:** Download the latest Ubuntu Desktop or Server ISO (e.g., Ubuntu 24.04 LTS) to serve as the guest operating system:
```bash
wget [https://releases.ubuntu.com/noble/ubuntu-24.04.4-desktop-amd64.iso](https://releases.ubuntu.com/noble/ubuntu-24.04.4-desktop-amd64.iso)

```



**Phase 2: Virtual Machine Provisioning**

1. **Create the VM:** Open VirtualBox, click **New**, name the machine (e.g., `Guest1`), and select the downloaded Ubuntu ISO.
2. **Allocate Resources:** Assign adequate Base Memory (minimum 2048 MB) and virtual hard disk space (minimum 25 GB).
3. **Configure Display Settings:** Navigate to **Settings > Display** and max out the **Video Memory** to 128MB. This prevents black screen errors and ensures the graphical terminal loads correctly.
4. **Network Adapter Setup:** Navigate to **Settings > Network** for both VMs. Set the **Attached to** configuration to **NAT Network** or **NAT**.

**Phase 3: Guest Network Configuration**

1. **Boot and Install:** Power on both virtual machines and complete the basic Ubuntu installation.
2. **Install Network Utilities:** Once booted, open the terminal (`Ctrl + Alt + T` or `Right Ctrl + F3` for TTY) and install the required networking tools:
```bash
sudo apt update
sudo apt install net-tools

```


3. **Execute Interface Inspection:** Run the configuration command to identify the allocated IP addresses:
```bash
ifconfig

```



**Phase 4: Interface Analysis**

* **Result:** Both Guest 1 and Guest 2 are assigned the exact same IP address (typically **10.0.2.15**).
* **Explanation:** VirtualBox's NAT engine deploys an independent, isolated virtual router and DHCP server for each host. Since the instances run on separate physical machines, they exist in parallel virtual networks and receive the default starting IP from their respective local DHCP servers.

**Phase 5: Connectivity Test**

* Attempt to send ICMP echo requests from Guest 1 to Guest 2:
```bash
ping 10.0.2.15

```


* **Result:** The ping fails (100% packet loss).

**Architectural Explanation**

* **High-level overview:** The virtual machines are isolated in private subnets created by the NAT engine on their respective physical hosts.
* **In-depth mechanics:** A packet destined for `10.0.2.15` never leaves the origin host. The virtual NAT router attempts to resolve the address within its own isolated subnet. Even if external physical IPs were targeted, NAT acts as a one-way outbound gateway by default. Without explicit routing rules or port forwarding configured on the host's NAT engine, inbound traffic cannot traverse the host to reach the isolated guest OS.
* **Practical application/examples:** This simulates a scenario where two users in different homes (behind different home routers) try to ping each other's private IP (`192.168.1.5`). The traffic drops because private subnets are non-routable over the broader network.
* **Summary:** The ping fails because the NAT layer completely isolates the guest machine from external inbound traffic without explicit port forwarding.

**Resolution (Bridging the Gap)**
To establish direct communication, bypass the NAT isolation by attaching the VMs directly to the physical local area network:

1. Power off both virtual machines.
2. Change the Network Adapter setting from **NAT** to **Bridged Adapter**.
3. Select your active physical network interface (e.g., your Wi-Fi card) in the 'Name' dropdown.
4. Reboot and run `ifconfig`. Each VM will now pull a unique, routable IP address directly from the physical LAN's DHCP server, enabling successful ICMP routing between them.

```

