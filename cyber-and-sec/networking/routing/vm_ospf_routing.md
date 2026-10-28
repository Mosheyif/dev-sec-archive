

# Lab Guide: Dynamic Routing with OSPF

## Phase 1: Revert Static Routing Configurations
Remove the manual static routes configured in the previous exercise to ensure traffic is routed strictly via the dynamic OSPF protocol.
*   On Host 1, execute:
    ```bash
    sudo ip route del 192.168.69.0/24
    ```
*   On Host 2, execute the corresponding command to remove the route to Host 1's guest network:
    ```bash
    sudo ip route del 192.168.10.0/24
    ```

## Phase 2: Install and Enable the Routing Daemon
Deploy FRRouting (FRR) to manage the OSPF protocol on both physical host machines.
*   Install the FRR package:
    ```bash
    sudo apt update
    sudo apt install frr -y
    ```
*   Enable the OSPF daemon by editing the configuration file:
    ```bash
    sudo nano /etc/frr/daemons
    ```
*   Locate the line `ospfd=no` and change it to `ospfd=yes`. Save and exit.
*   Restart the FRR service to apply the daemon configuration:
    ```bash
    sudo systemctl restart frr
    ```

## Phase 3: OSPF Configuration and Router ID Assignment
Configure the OSPF process on both hosts using the `vtysh` routing shell. Each host must be assigned a unique Router ID and advertise its connected networks.

**Host 1 Configuration:**
```bash
sudo vtysh
conf t
router ospf
ospf router-id 1.1.1.1
network 10.1.0.0/24 area 0
network 192.168.10.0/24 area 0
exit
exit
write
exit

```

**Host 2 Configuration:**

```bash
sudo vtysh
conf t
router ospf
ospf router-id 2.2.2.2
network 10.1.0.0/24 area 0
network 192.168.69.0/24 area 0
exit
exit
write
exit

```

*(Note: Replace `10.1.0.0/24` with the actual physical subnet shared by the hosts if it differs).*

## Architectural Explanation: OSPF Router ID

* **High-level overview:** A Router ID (RID) is a 32-bit integer (formatted identically to an IPv4 address) that uniquely identifies a specific router within an OSPF domain.
* **In-depth mechanics:** The OSPF protocol utilizes the Shortest Path First (SPF) algorithm to construct a precise topological map of the network. To build the Link State Database (LSDB) accurately, every participating node must have an absolute, unique identifier. The Router ID prevents database conflicts, facilitates the exchange of Link State Advertisements (LSAs), and dictates critical election processes (such as selecting the Designated Router on a multi-access network). If two routers share the same ID, OSPF neighbor adjacencies will fail to form, breaking the routing topology.
* **Summary:** Manually assigning a unique Router ID prevents automatic allocation conflicts and ensures stable, predictable OSPF neighbor adjacencies across the network.

```

