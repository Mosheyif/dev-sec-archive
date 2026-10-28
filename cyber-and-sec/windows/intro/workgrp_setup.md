

## Exercise 4: Network Configuration and Workgroup Setup

**Objective:** Establish network connectivity between two Windows environments,
configure them to the exact same Workgroup, and verify communication via ICMP
(Ping) and NetBIOS Name Resolution.

*Note on Deployment Architecture:* This exercise conceptually requires two
separate Windows environments. This can be seamlessly achieved using two
physical computers or two concurrent Virtual Machines (VMs). However, due to
hardware resource limitations that prevented the concurrent execution of two
intensive Windows 11 VMs, this guide documents the alternative Host-to-VM
architecture using a VirtualBox Host-Only Adapter. This method achieves the
exact same networking objectives.

### Step 1: Acquire the Windows Image
1. Download the official Windows 11 ISO (Multi-Edition, approximately 8.4GB)
from the Microsoft Evaluation Center or the official Windows download page.

### Step 2: Provision the Virtual Machine (VM)
1. Open **Oracle VM VirtualBox**.
2. Click **New** and configure the VM settings (allocating at least 4096MB RAM
and 2 CPUs).
3. Ensure **TPM 2.0** and **EFI (special OSes only)** are enabled in the
System/Motherboard settings to meet Windows 11 installation prerequisites.
4. Attach the downloaded Windows 11 `.iso` file to the Optical Drive under
the Storage settings.
5. Start the VM and proceed through the Unattended or manual Windows
installation until you reach the Desktop.

### Step 3: Configure the Network (Host-to-VM)
To create an isolated virtual switch between the Host computer and the VM
without relying on an external DHCP server:
1. In VirtualBox, navigate to the VM's **Settings** > **Network**.
2. Change **Attached to** to **Host-only Adapter** (e.g., `VirtualBox
Host-Only Ethernet Adapter`).
3. Start the VM.
4. Open an **Elevated PowerShell (Run as Administrator)** in the VM and
assign a static IP address in the same subnet as the Host's VirtualBox
adapter (e.g., `192.168.56.X`):
   ```cmd
   netsh interface ip set address name="Ethernet" static 192.168.56.10 ^
     255.255.255.0

    ```

### Step 4: Configure the Workgroup and Firewall

Both the Host and the VM must be joined to the exact same Workgroup and have
their firewalls configured to allow inbound ICMP traffic.

1. **Allow ICMP/SMB Traffic** (Execute in an Elevated PowerShell on *both*
the Host and the VM):

```powershell
Set-NetFirewallRule -DisplayGroup "File and Printer Sharing" -Enabled True

```

2. **Set the Workgroup** (Execute in an Elevated PowerShell on *both* the Host
and the VM):

```powershell
Add-Computer -WorkGroupName "INFINITY_LABS" -Restart

```

### Step 5: Verification and Name Resolution

After both systems reboot, verify that all layers of connectivity are
functional.

1. Open PowerShell on the **Host machine** and ping the VM by its allocated
hostname (e.g., `win-pc-1`):

```powershell
ping win-pc-1

```

2. Open PowerShell on the **Virtual Machine** and ping the Host by its
hostname (e.g., `win-pc-2`):

```powershell
ping win-pc-2

```

A successful `Reply from...` with `0% loss` in both directions confirms that
IP routing, Firewall rules, and Workgroup Name Resolution are fully
operational.
