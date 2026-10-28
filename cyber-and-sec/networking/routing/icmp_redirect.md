

**ICMP Redirect Analysis**

* **High-level overview:** An ICMP Redirect (Type 5) is a control message sent by a router to a host, instructing the host to update its routing table with a more direct path to a specific destination.
* **In-depth mechanics:** If a router receives a packet and determines that the next-hop gateway for the destination resides on the exact same network segment (interface) from which the packet originated, it forwards the packet but also sends an ICMP Redirect back to the source. This informs the sender to bypass the current router and send future packets directly to the more optimal gateway.
* **Summary:** It is an automatic routing optimization mechanism that eliminates unnecessary physical or logical hops on the same local subnet.

**Risks of ICMP Redirects**

* **Man-in-the-Middle (MitM) Attacks:** A malicious actor on the local network can forge ICMP Redirect packets, tricking a victim host into routing all its traffic through the attacker's machine for interception.
* **Denial of Service (DoS):** Attackers can maliciously redirect a host's traffic to a non-existent or inactive IP address, effectively dropping all connectivity to the target destination.

**Triggering ICMP Redirects from the Router (Host)**
You can explicitly enable or ensure your Linux host (acting as the router) sends these messages by modifying the kernel network parameters:

```bash
sudo sysctl -w net.ipv4.conf.all.send_redirects=1
sudo sysctl -w net.ipv4.conf.default.send_redirects=1

```

**Guest Acceptance of ICMP Redirects**
Modern Linux distributions (including your Ubuntu guest) **ignore** ICMP redirect messages by default specifically to mitigate the security risks mentioned above.

To verify if the guest is currently dropping them, check the parameter:

```bash
sysctl net.ipv4.conf.all.accept_redirects

```

To force the guest to actively listen and alter its routing table based on received ICMP redirects, enable it manually:

```bash
sudo sysctl -w net.ipv4.conf.all.accept_redirects=1

```
