

# Lab Guide: Manual Router Configuration and Virtual Networks

## Phase 1: Host-Only Network Adapter Creation

Configure the physical host to manage an isolated virtual network.

* Open VirtualBox and navigate to **File > Host Network Manager** (or **Network Manager**).
* Create a new Host-Only network adapter (e.g., `vboxnet0`).
* Access the **Adapter** properties and set the IPv4 Address to `192.168.10.254` (Host 1) or `192.168.20.254` (Host 2) with a Subnet Mask of `255.255.255.0`.
* Navigate to the **DHCP Server** tab and disable the server completely.

## Phase 2: Host as a Router (IP Forwarding)

To allow the host to route packets between its physical interface and the virtual network interface, kernel-level IP forwarding must be enabled.

* Enable temporary IP forwarding via the terminal:
```bash
sudo sysctl -w net.ipv4.ip_forward=1

```


* **Architectural Meaning:** By default, a Linux host drops packets not destined for its own IP addresses. Enabling IP forwarding changes the system's behavior to act as a router (Layer 3 gateway), allowing it to accept packets on one interface (e.g., the virtual `vboxnet0`) and forward them to another (e.g., the physical LAN interface).

## Phase 3: Guest Virtual Machine Setup

Attach the virtual machine to the newly created routing interface and configure it statically.

* In VirtualBox, open the VM **Settings > Network**.
* Set the adapter to **Host-only Adapter** and select `vboxnet0`.
* Under **Advanced**, ensure **Cable Connected** is checked.
* Boot the VM and configure the static IP and default gateway via the guest terminal:
```bash
sudo ip link set enp0s3 up
sudo ip addr add 192.168.10.1/24 dev enp0s3
sudo ip route add default via 192.168.10.254

```



## Phase 4: VirtualBox Network Interfaces Theory

Analyze the primary network configurations available in VirtualBox.

**NAT (Network Address Translation)**

* **High-level overview:** A unidirectional outbound access configuration acting as the default setting.
* **In-depth mechanics:** The VirtualBox engine acts as a virtual router performing Source NAT on outbound packets. The guest receives a virtual internal IP, and traffic is masqueraded behind the host's physical IP. Inbound connections are blocked without explicit port forwarding rules.
* **Practical application/examples:** Fetching OS updates or browsing the web from a guest without exposing it to the local LAN.
* **Summary:** An isolated network allowing outbound internet access via host masquerading.

**Bridged Adapter**

* **High-level overview:** A transparent Layer 2 connection directly to the host's physical network.
* **In-depth mechanics:** The virtualization engine bridges the virtual network interface directly to the physical adapter. The guest communicates with the physical router's DHCP server and receives a valid subnet IP, bypassing host routing entirely.
* **Practical application/examples:** Deploying a guest web server that must be directly accessible by other physical devices on the local office network.
* **Summary:** The virtual machine operates as an independent peer node on the physical local area network.

**Host-Only Adapter**

* **High-level overview:** A closed-loop virtual network connecting only the host and its localized guests.
* **In-depth mechanics:** A dedicated virtual interface is instantiated on the host operating system. It lacks a default gateway to the external network, confining traffic to the host and attached guests unless explicit IP forwarding and routing rules are injected into the kernel.
* **Practical application/examples:** Constructing isolated malware analysis environments or local networking labs.
* **Summary:** A hermetically sealed virtual network restricted to host-to-guest communication on the same physical machine.

## Phase 5: Inter-Host Routing Configuration

Establish static routes between the two physical hosts to bridge the isolated virtual networks. Ensure you execute these commands on the physical host machines, not the virtual guests.

* Identify the physical LAN IP addresses of both Host 1 (e.g., `10.0.0.1`) and Host 2 (e.g., `10.0.0.2`).
* On Host 1, route traffic destined for Guest 2's network (`192.168.20.0/24`) via Host 2's physical IP:
```bash
sudo ip route add 192.168.20.0/24 via 10.0.0.2

```


* On Host 2, route traffic destined for Guest 1's network (`192.168.10.0/24`) via Host 1's physical IP:
```bash
sudo ip route add 192.168.10.0/24 via 10.0.0.1

```


* Test connectivity from Guest 1 to Guest 2 to verify routing success:
```bash
ping 192.168.20.1

```



## Common Issues & Troubleshooting (Optional)

* **VirtualBox Error: `E_ACCESSDENIED` when creating Host-Only adapter.**
* **Cause:** Recent VirtualBox security policies restrict custom IP ranges outside `192.168.56.0/21`.
* **Fix:** Authorize broader IP ranges by creating a global configuration file on the host:
```bash
sudo mkdir -p /etc/vbox
echo "* 10.0.0.0/8 192.168.0.0/16" | sudo tee /etc/vbox/networks.conf

```




* **Guest Ping returns `Destination Host Unreachable` to its own gateway.**
* **Cause:** The host's `vboxnet0` interface is down, or the virtual cable is disconnected in VM settings.
* **Fix:** Ensure the host interface is active on the physical machine:
```bash
sudo ip link set vboxnet0 up
sudo ip addr add 192.168.10.254/24 dev vboxnet0

```




* **Routing Error: `Nexthop has invalid gateway`.**
* **Cause:** Attempting to route traffic through a gateway IP that does not exist on the current physical subnet.
* **Fix:** The `via` address in the `ip route` command must strictly be the peer's physical IP (e.g., the Wi-Fi IP), not the peer's virtual `vboxnet` IP.


* **Routing Error: `File exists`.**
* **Cause:** Attempting to add a route to a local subnet that the kernel has already mapped directly.
* **Fix:** Delete the conflicting manual route (`sudo ip route del <subnet>`) and ensure you are routing to the remote guest's network, not the local guest's network.
