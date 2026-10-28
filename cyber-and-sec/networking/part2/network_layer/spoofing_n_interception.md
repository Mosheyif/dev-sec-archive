# Network Security Lab: ARP Spoofing & Interception Sandbox

## 1. Lab Objectives

The primary objective of this architecture is to deploy and validate a custom Layer 2 ARP spoofing tool. The tool is designed with dynamic network resolution and allows the operator to toggle Linux IP Forwarding, seamlessly switching the attack vector between a Man-in-the-Middle (MiTM) interception and a Denial of Service (DoS). To test this safely, an isolated, declarative Docker-based sandbox network is provisioned.

---

## 2. Tool Architecture & Defensive Concepts

### Logical Flow (`arp_spoofer.py`)

The Python implementation orchestrates Layer 2 interception by poisoning the ARP cache of both the target and the gateway. It handles dynamic configuration of the Linux kernel's Netfilter and routing tables to process intercepted traffic, ensuring state backups and graceful rollbacks upon termination.

```text
[Initialization] --> [Network Discovery (IP/MAC)] --> [Kernel Config (sysctl/iptables)]
                                                                |
                                                                v
[Teardown & Cleanup] <-- [Interrupt Signal] <-- [Bi-directional ARP Poisoning Loop]

```

*(Reference: `arp_spoofer.py` execution logic).*

### Stealth and Detection

During an active MiTM attack with kernel IP forwarding enabled, the victim experiences no disruption; application-layer internet connectivity operates normally. However, the attack leaves explicit low-level footprints:

* **Host-level Detection:** Querying the local ARP cache via `ip neigh` or `arp -a` on Linux environments will reveal duplicate MAC addresses (the gateway's IP will share the identical MAC address of the attacker).
* **Network-level Detection:** Analyzing traffic with tools like Wireshark will expose an abnormally high rate of unsolicited `ARP Reply` packets mapping a single physical MAC to multiple logical IP addresses.

### Mitigation

ARP spoofing exploits the inherently stateless and unauthenticated nature of the ARP protocol in IPv4. At the infrastructure level, enterprise switches mitigate this using **Dynamic ARP Inspection (DAI)**. DAI validates incoming ARP packets against a trusted binding database (such as DHCP snooping tables), dropping malicious or spoofed ARP responses before they reach the endpoints.

---

## 3. Sandbox Infrastructure & Provisioning

The test environment relies on a flat Layer 2 topology provisioned via Docker Compose. All nodes share a single broadcast domain (`10.0.0.0/24`), which is a strict prerequisite for local ARP manipulation. The attacker container operates with elevated privileges (`privileged: true`) to permit raw socket operations and dynamic kernel modifications.

```text
+-----------------------------------------------------------+
|                  Docker Bridge: mitm_lab                  |
|                      (10.0.0.0/24)                        |
+-----------------------------------------------------------+
         |                       |                       |
+-----------------+     +-----------------+     +-----------------+
|  Gateway Node   |     |   Victim Node   |     |  Attacker Node  |
|  (10.0.0.254)   |     |   (10.0.0.20)   |     |   (10.0.0.10)   |
+-----------------+     +-----------------+     +-----------------+

```

*(Reference: Environment defined via `docker-compose.yml`).*

**Provisioning the Environment:**
To build the architecture, resolve dependencies, and start the containers in the background, navigate to the directory containing the `docker-compose.yml` file and execute:

```bash
docker-compose up -d

```

---

## 4. Execution & Traffic Analysis

The following procedures validate the MiTM interception capabilities utilizing unencrypted (Cleartext) application-layer protocols.

### Scenario A: Cleartext Image Interception (HTTP GET)

1. **Initiate Packet Capture (Attacker):**
```bash
docker exec -it attacker tshark -i eth0 -f "tcp port 80" -w /opt/scripts/http_image.pcap

```


2. **Fetch Resource (Victim):**
```bash
docker exec -it victim wget http://neverssl.com/favicon.ico -O /tmp/image.ico

```


3. **Analysis (Host Machine):**
Terminate the `tshark` capture. Open the resulting `http_image.pcap` in Wireshark. Navigate to **File -> Export Objects -> HTTP**, select the intercepted `.ico` file, and save it to the host filesystem to reconstruct the binary payload.

### Scenario B: Credential Extraction (HTTP POST)

1. **Deploy Dummy Server (Gateway):**
```bash
docker exec -it gateway bash -c "nc -l -p 8080"

```


2. **Initiate Packet Capture (Attacker):**
```bash
docker exec -it attacker tshark -i eth0 -f "tcp port 8080" -w /opt/scripts/http_login.pcap

```


3. **Transmit Credentials (Victim):**
```bash
docker exec -it victim curl -X POST http://10.0.0.254:8080/login -d "username=admin&password=MySuperSecretPassword123"

```


4. **Analysis (Host Machine):**
Terminate the capture. Open `http_login.pcap` in Wireshark and apply the filter `http.request.method == "POST"`. Inspect the packet details under the **HTML Form URL Encoded: application/x-www-form-urlencoded** tree to view the parsed username and password variables in absolute cleartext.

---

## 5. Environment Teardown

To release hardware resources and ensure a clean state for future deployments, the virtual environment must be destroyed once testing is complete. This removes the containers, the virtual switch, and the allocated subnets.

**Destroying the Environment:**
Execute the following command in the directory containing the `docker-compose.yml` file:

```bash
docker-compose down

```
