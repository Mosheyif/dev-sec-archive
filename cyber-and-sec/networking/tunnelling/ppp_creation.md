

## Exercise Overview: PPP Tunneling over SSH
This exercise demonstrates how to establish a direct, logical Point-to-Point Protocol (PPP) tunnel between two Linux machines using SSH as the secure transport layer. We will create virtual network interfaces (`ppp0`), verify isolated routing, and explore how Maximum Transmission Unit (MTU) limitations affect packet delivery at the kernel level.

---

### Phase 1: Prerequisites and Installation
Before establishing the tunnel, the PPP daemon must be present on both the Host and the Guest machines. The daemon (`pppd`) is responsible for managing the connection, authentication, and the creation of the virtual network interfaces.

Run this on both the Host and the Guest VM:
```bash
sudo apt update && sudo apt install ppp -y

```

### Phase 2: Establishing the PPP Tunnel

To create the tunnel, we execute `pppd` on the Host and instruct it to use an SSH connection to spawn a corresponding `pppd` process on the Guest.

Run the following command on the Host machine:

```bash
sudo pppd nodetach pty "ssh user@<GUEST_MACHINE_IP> sudo pppd notty noauth" 10.0.0.1:10.0.0.2 noauth local nocrtscts

```

**Understanding the flags and architecture:**

* `nodetach`: Keeps the process running in the foreground. This is critical for debugging and prevents the tunnel from failing silently in the background.
* `pty "ssh ..."`: Instructs `pppd` to use a pseudo-terminal running an SSH session as its transport medium, rather than a physical serial port.
* `notty`: Passed to the remote Guest daemon, telling it not to allocate a terminal since it's receiving a raw binary stream.
* `10.0.0.1:10.0.0.2`: Manually assigns the logical IP addresses for the tunnel endpoints (Host Tunnel IP : Guest Tunnel IP).
* `noauth`: Disables standard PPP authentication (like CHAP) because the underlying SSH tunnel already provides secure authentication.
* `local` & `nocrtscts`: Ignores legacy modem control lines and disables hardware flow control.

> **Common Pitfall & Solution: The "Invisible" Interface**
> If you run the command in the background and immediately check `ip a`, you might see `Device "ppp0" does not exist`. This happens because `pppd` expects a pure binary stream. If SSH prompts for a First-Time Key Fingerprint (`yes/no`), an SSH password, or a `sudo` password, it injects plain text into the stream, causing the PPP negotiation to crash silently.
> **The Fix:** Using the `nodetach` flag allows you to see these prompts and interact with them. Alternatively, establish a standard SSH connection first to save the fingerprint, and set up SSH Keys and `NOPASSWD` sudo rules to ensure a completely silent, automated login.

### Phase 3: Verifying Connectivity

Once the connection is established (leave the previous terminal open), open a new terminal on the Host to verify the overlay network.

1. **Inspect the virtual interface:**

```bash
ip a show ppp0

```

You should see the interface in a `POINTOPOINT` state, bound to the `10.0.0.1` IP address and peered with `10.0.0.2`.

2. **Test the tunnel routing:**

```bash
ping -c 4 10.0.0.2

```

This confirms that ICMP traffic is successfully being routed through the encrypted `ppp0` interface.

3. **Test the underlay physical network:**

```bash
ping -c 4 <GUEST_MACHINE_IP>

```

This proves that the standard network interface remains fully operational and acts as the carrier for our overlay tunnel.

### Phase 4: Exploring MTU Limitations

The Maximum Transmission Unit (MTU) defines the absolute largest packet size (in bytes) that an interface can process. Let's manipulate it to see how the kernel handles oversized or undersized packets.

1. **Restrict the MTU size:**

```bash
sudo ip link set dev ppp0 mtu 1000

```

2. **Force an oversized packet:**
Send a ping with a payload of 1200 bytes, and explicitly instruct the kernel not to fragment it (`-M do`):

```bash
ping -M do -s 1200 10.0.0.2

```

*Result:* The kernel immediately rejects the command with a `Message too long` error, proving the interface enforces the 1000-byte limit.

3. **Break the interface completely:**
Lower the MTU to an extreme value:

```bash
sudo ip link set dev ppp0 mtu 60
ping 10.0.0.2

```

*Result:* The ping hangs or results in 100% packet loss.
*Why?* The IPv4 protocol requires a strict mathematical minimum of 68 bytes just to construct standard network headers (20 bytes for IPv4 + 8 bytes for ICMP + options). By forcing the MTU to 60, the OS kernel is physically incapable of assembling a legal IP packet, causing all traffic to drop before it even leaves the machine.

### Phase 5: Cleanup and Reverting

Because `ppp0` is a virtual interface bound directly to the `pppd` daemon, cleanup is completely automated.

1. Go back to the terminal running the `pppd nodetach` command.
2. Press `Ctrl+C` to kill the process.
3. The SSH connection will sever, the daemon will terminate, and the kernel will automatically destroy the `ppp0` virtual interfaces and their associated routing rules on both machines.

```

