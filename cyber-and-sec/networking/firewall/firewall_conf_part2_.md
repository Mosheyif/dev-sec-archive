

## 6. Advanced Networking (Host-only Adapter)

This exercise configures the Host machine to act as an Edge Router and Firewall for an isolated Guest VM (`192.168.56.2`). It covers outbound NAT, inbound Port Forwarding, DMZ-style firewall blocking, and secure bypassing via SSH tunneling.

### The Troubleshooting & Debugging Journey
During the implementation of this topology, several real-world architectural issues were encountered and resolved:
* **Interface Initialization:** The Host's `vboxnet0` interface remained `DOWN` until the VM was actively powered on, emphasizing that virtual interfaces require active hypervisor attachments.
* **IP Address Conflict (Local Loop):** The Guest's interface (`enp0s3`) was initially assigned `192.168.56.1`, establishing a direct conflict with the Host's virtual interface. This caused packets to loop locally. We resolved this by flushing the Guest's IP (`ip addr flush dev enp0s3`) and statically assigning `192.168.56.2/24`.
* **DNS Resolver Override:** The Guest failed to resolve package repositories (`archive.ubuntu.com`). The native `systemd-resolved` service continuously overwrote our custom DNS settings. We permanently bypassed this by forcing a static configuration: `sudo rm -f /etc/resolv.conf`, writing `nameserver 8.8.8.8` directly into it, and locking it at the filesystem level using `chattr +i`.
* **NTP & Cryptographic Validation:** `apt update` failed with an `invalid for another 3d` error. The VM's hardware clock was desynchronized, causing the package manager to reject the repository's cryptographic signatures as "future" events. This was fixed by forcing a network time sync via `timedatectl set-ntp true`.

---

### a. Implement NAT using iptables (Guest Internet Access)

**Explanation:**
The Guest is on an isolated network (`vboxnet0`) and cannot reach the internet. We modify the Host's kernel parameters to enable packet forwarding. Then, we use the `MASQUERADE` target to rewrite the Guest's private source IP to the Host's public IP before the packets leave the physical network interface. 

**Command Breakdown:**
* `sysctl -w net.ipv4.ip_forward=1`: Modifies the Linux kernel parameter at runtime to allow the machine to route packets between different interfaces.
* `-I FORWARD 1`: Inserts the rule at index 1 (the very top) of the `FORWARD` chain, ensuring it is evaluated before any default `DROP` policies.
* `-t nat`: Specifies that we are manipulating the Network Address Translation (`nat`) table, rather than the default `filter` table.
* `-A POSTROUTING`: Appends the rule to the chain that processes packets just *before* they leave the machine.
* `-o enx0c3796398d14`: Matches only packets leaving via this specific outbound physical interface.
* `-j MASQUERADE`: Dynamically changes the source IP of the packet to match the IP of the outbound interface.

**Configuration:**
```bash
# Host
sudo sysctl -w net.ipv4.ip_forward=1
sudo iptables -I FORWARD 1 -i vboxnet0 -j ACCEPT
sudo iptables -I FORWARD 1 -o vboxnet0 -j ACCEPT
sudo iptables -t nat -A POSTROUTING -o enx0c3796398d14 -j MASQUERADE

# Guest
sudo ip route add default via 192.168.56.254

```

---

### b. Enable external browsing to the Guest server via Port Forwarding

**Explanation:**
To allow external clients to access the Guest's Apache server, we apply Destination NAT (`DNAT`). When an external request hits the Host's public IP on port 80, the Host intercepts it before routing, rewrites the destination IP to the Guest's internal IP, and passes it inward.

**Command Breakdown:**

* `-A PREROUTING`: Intercepts packets immediately as they arrive at the network interface, *before* the kernel makes any routing decisions.
* `-p tcp --dport 80`: Targets only TCP traffic destined for the standard HTTP port.
* `-j DNAT --to-destination 192.168.56.2:80`: Changes the packet's destination header to point to the isolated Guest VM.

**Configuration:**

```bash
sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.56.2:80

```

---

### c. Block access to the guest server on the Host firewall

**Explanation:**
To simulate a secured DMZ, we block direct external HTTP access to the internal server. Because the `DNAT` rule (from step B) changes the destination IP before the routing decision, the packet traverses the `FORWARD` chain instead of the `INPUT` chain.

**Command Breakdown:**

* `-I FORWARD 1`: Forces this `DROP` rule to the top of the chain. If placed lower, a general `ACCEPT` rule might permit the traffic first.
* `-d 192.168.56.2`: Specifically targets packets whose ultimate destination is the Guest VM.

**Configuration:**

```bash
sudo iptables -I FORWARD 1 -p tcp -d 192.168.56.2 --dport 80 -j DROP

```

---

### d. Enable browsing to the server using SSH tunneling

**Explanation:**
Local Port Forwarding (`-L`) circumvents the firewall block. By connecting to the Host via SSH (port 22, which is allowed), the SSH daemon on the Host decrypts the encapsulated HTTP request and generates a *new* connection to the Guest. Because this new connection originates locally from the Host itself, it travels through the `OUTPUT` chain, entirely bypassing the `FORWARD` chain's `DROP` rule.

**Command Breakdown:**

* `-L 8080:192.168.56.2:80`: The core tunneling flag. It binds local port `8080` on the client's machine, encrypts the traffic, sends it to the SSH server on the Host (`10.1.0.20`), which then forwards the raw data to `192.168.56.2` on port `80`.

**Execution (From an external client machine):**

```bash
# 1. Establish the encrypted tunnel
ssh -L 8080:192.168.56.2:80 moshe@10.1.0.20

# 2. Access the server through the tunnel (in a separate terminal)
curl http://localhost:8080

```

```
