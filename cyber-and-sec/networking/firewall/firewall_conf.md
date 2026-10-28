
# Basic Firewall Configuration Exercises (Part 2: Advanced Host-only Topology)

## 6. Advanced Networking: Edge Router, NAT & DMZ Simulation

This exercise configures a physical Host machine to act as an Edge Router and Firewall for an isolated Guest VM (`192.168.56.2`). The process is documented chronologically, detailing the real-world debugging steps required to establish network connectivity, bypass local OS overrides, enforce firewall rules, and ultimately securely tunnel through them.

**Environment Assumptions (Generic):**
* **Host External Interface:** `eth0` (Public/LAN IP: `203.0.113.50`)
* **Host Internal Interface:** `vboxnet0` (IP: `192.168.56.1`)
* **Guest Interface:** `enp0s3` (Target IP: `192.168.56.2`)
* **External Client:** A separate machine on the `203.0.113.x` network.

---

### Phase 1: Interface Initialization and Conflict Resolution

**Objective:** Establish a clean Layer 2/Layer 3 connection between the Host and the Guest.

**Bug 1: Interface Down**
Initially, querying `ip a` on the Host showed `vboxnet0` in a `DOWN` state.
* **Fix:** Virtual interfaces require an active hypervisor attachment. The Guest VM must be actively powered on for the host kernel to initialize the link.

**Bug 2: IP Address Conflict (Local Loop)**
After bringing the interface up, traffic failed to route. Inspection revealed both the Host (`vboxnet0`) and the Guest (`enp0s3`) were defaulting to the exact same IP address (`192.168.56.1`), causing packets to drop into a local loopback blackhole.
* **Fix (Executed on Guest):** Flush the interface and manually assign a unique IP and default gateway.
```bash
# 1. Flush the conflicting IP
sudo ip addr flush dev enp0s3

# 2. Assign the new unique Guest IP
sudo ip addr add 192.168.56.2/24 dev enp0s3

# 3. Set the Host as the default gateway router
sudo ip route add default via 192.168.56.1
```

---

### Phase 2: Implementing Outbound NAT (Guest Internet Access)

**Objective:** Allow the isolated Guest to reach the external internet by using the Host as a NAT router.

**Explanation:** We modify the Host's kernel parameters to enable IP forwarding, authorize the traffic flow in the `FORWARD` chain, and apply `MASQUERADE` to dynamically rewrite the Guest's private source IP to the Host's external IP (`eth0`).

**Configuration (Executed on Host):**
```bash
# 1. Enable Kernel IP Forwarding
sudo sysctl -w net.ipv4.ip_forward=1

# 2. Allow traffic to flow across the virtual interface
sudo iptables -I FORWARD 1 -i vboxnet0 -j ACCEPT
sudo iptables -I FORWARD 1 -o vboxnet0 -j ACCEPT

# 3. Apply Masquerade on the outbound external interface
sudo iptables -t nat -A POSTROUTING -o eth0 -j MASQUERADE
```

---

### Phase 3: The DNS & NTP Hurdles (Guest OS Debugging)

**Objective:** Install the `apache2` web server on the Guest. 

**Bug 3: OS Resolver Overrides (DNS Failure)**
Running `apt update` failed with `Temporary failure resolving 'archive.ubuntu.com'`. Native system utilities (`systemd-resolved`) were overriding `/etc/resolv.conf` with a local loopback (`127.0.0.53`) that lacked upstream connectivity.
* **Fix (Executed on Guest):** Destroy the symlink, hardcode an external DNS server, and lock the file at the filesystem level (`chattr +i`) to prevent background processes from overwriting it again.
```bash
sudo rm -f /etc/resolv.conf
sudo bash -c 'echo "nameserver 8.8.8.8" > /etc/resolv.conf'
sudo chattr +i /etc/resolv.conf
```

**Bug 4: Cryptographic Signature Rejection (Time Sync)**
A subsequent `apt update` failed with an `invalid for another 3d` error. The hardware clock of the Guest VM was significantly out of sync, causing the package manager to reject the repository's SSL certificates as "future" events.
* **Fix (Executed on Guest):** Force a network time synchronization.
```bash
sudo timedatectl set-ntp true
```

**Resolution:**
```bash
sudo apt clean
sudo apt update && sudo apt install apache2 -y
sudo systemctl start apache2
```

---

### Phase 4: Inbound Access (Port Forwarding / DNAT)

**Objective:** Allow external clients to view the Apache server hosted on the internal Guest.

**Explanation:** We apply Destination NAT (`DNAT`). When an external request hits the Host's public IP (`203.0.113.50`) on port 80, the Host's `PREROUTING` chain intercepts it, rewrites the destination IP to the Guest's internal IP (`192.168.56.2`), and passes it inward.

**Configuration (Executed on Host):**
```bash
sudo iptables -t nat -A PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.56.2:80
```

**Testing (Executed from External Client):**
```bash
curl http://203.0.113.50
# Successfully returns the Apache default page from the Guest.
```

---

### Phase 5: Securing the DMZ (Firewall Block)

**Objective:** Block direct external HTTP access to the internal server, simulating a secured, closed environment.

**Explanation:** Because the `DNAT` rule (from Phase 4) changes the destination IP *before* routing decisions occur, the packet traverses the `FORWARD` chain, not the `INPUT` chain. We insert a `DROP` rule at the top of the `FORWARD` chain to kill these requests.

**Configuration (Executed on Host):**
```bash
sudo iptables -I FORWARD 1 -p tcp -d 192.168.56.2 --dport 80 -j DROP
```

**Testing (Executed from External Client):**
```bash
curl --connect-timeout 5 http://203.0.113.50
# Connection times out due to the DROP rule.
```

---

### Phase 6: Bypassing the Firewall via SSH Tunneling

**Objective:** Securely bypass the firewall block to access the web server.

**Explanation:** Local Port Forwarding (`-L`) circumvents the firewall. The external client connects securely to the Host's SSH daemon (port 22, which remains open). The SSH daemon, running locally on the Host, extracts the HTTP payload and initiates a *new* connection to the Guest. Because this new connection originates locally from the Host itself, it travels through the `OUTPUT` chain, entirely avoiding the `FORWARD` chain's `DROP` rule.

**Execution (Executed from External Client):**
```bash
# 1. Establish the encrypted tunnel (binds local port 8080 to Guest port 80 via the Host)
ssh -L 8080:192.168.56.2:80 user@203.0.113.50

# 2. Access the server through the tunnel (in a separate terminal on the client)
curl http://localhost:8080
# Successfully returns the Apache page, bypassing the firewall block.
```

---

### Phase 7: Environment Cleanup (Revert to Original State)

Execute these commands to remove all routing modifications, firewall rules, and NAT configurations.

**Cleanup (Executed on Host):**
```bash
# 1. Remove the firewall DROP block
sudo iptables -D FORWARD -p tcp -d 192.168.56.2 --dport 80 -j DROP

# 2. Remove the Port Forwarding (DNAT) rule
sudo iptables -t nat -D PREROUTING -p tcp --dport 80 -j DNAT --to-destination 192.168.56.2:80

# 3. Remove the Outbound NAT (MASQUERADE) rule
sudo iptables -t nat -D POSTROUTING -o eth0 -j MASQUERADE

# 4. Remove the FORWARD authorization rules
sudo iptables -D FORWARD -i vboxnet0 -j ACCEPT
sudo iptables -D FORWARD -o vboxnet0 -j ACCEPT

# 5. Disable Kernel IP Forwarding
sudo sysctl -w net.ipv4.ip_forward=0
```

**Cleanup (Executed on Guest):**
```bash
# 1. Stop and disable Apache (Optional)
sudo systemctl stop apache2
sudo systemctl disable apache2

# 2. Unlock and restore the default DNS resolver symlink
sudo chattr -i /etc/resolv.conf
sudo rm -f /etc/resolv.conf
sudo ln -s /run/systemd/resolve/stub-resolv.conf /etc/resolv.conf
```

