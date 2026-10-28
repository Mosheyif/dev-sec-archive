
Arrowverse Network Lab: Step-by-Step Cisco Packet Tracer Guide

This document provides a comprehensive, step-by-step walkthrough of the network exercise based on the "Arrowverse" topology. It is specifically tailored for Cisco Packet Tracer, explaining exactly how to place devices, configure them through the GUI, and apply commands in the CLI.

1. Simple LAN (Team Arrow)

Task: Place 4 PCs and 1 switch. Assign IPs (192.168.1.0/24) and test connectivity.

Packet Tracer Steps:

Place Devices: From the bottom-left menu, select Network Devices -> Switches and drag a 2960 switch to the workspace. Change its display name to Team Arrow.

Place PCs: Select End Devices -> PC and drag 4 PCs. Name them: Oliver Queen, John Diggle, Felicity Smoak, and Roy Harper.

Connect Devices: Select Connections (the lightning bolt icon). Choose the Copper Straight-Through cable (solid black line). Connect each PC's FastEthernet0 port to the switch's FastEthernet0/1 through 0/4 ports.

Configure IPs: Click on each PC, go to the Desktop tab, and click IP Configuration. Set them as follows (leave Default Gateway empty for now):

PC Oliver Queen: 192.168.1.2 / 255.255.255.0

PC John Diggle: 192.168.1.3 / 255.255.255.0

PC Felicity Smoak: 192.168.1.4 / 255.255.255.0

PC Roy Harper: 192.168.1.5 / 255.255.255.0

Test Connectivity: Click on Oliver Queen, go to the Desktop tab -> Command Prompt. Type ping 192.168.1.4 and press Enter to verify they can communicate.

2. Server LAN (Team Flash)

Task: Create a second network (2 PCs + 1 Web Server) on a new switch. Use subnet 192.168.2.0/24. Configure server with HTTP and verify.

Packet Tracer Steps:

Place Devices: Drag another 2960 Switch to the workspace and name it Team Flash. Drag two PCs (Iris West-Allen, Barry Allen) and one Server (The Monitor).

Connect Devices: Use Copper Straight-Through cables to connect the PCs and the Server to the switch.

Configure IPs: Click each device -> Desktop tab -> IP Configuration:

Server The Monitor: 192.168.2.1 / 255.255.255.0

PC Iris West-Allen: 192.168.2.2 / 255.255.255.0

PC Barry Allen: 192.168.2.3 / 255.255.255.0

Configure HTTP Server: Click The Monitor Server, go to the Services tab, select HTTP from the left menu, and ensure both HTTP and HTTPS are set to On.

Test Website: Click on Iris West-Allen -> Desktop tab -> Web Browser. Type 192.168.2.1 in the URL bar and hit "Go". You should see the default Cisco web page.

3. Routing Between LANs

Task: Add a router with interfaces in both subnets. Configure IPs, enable ports, and test inter-LAN ping.

Packet Tracer Steps:

Place Router: Select Network Devices -> Routers and drag a 2911 Router to the top. Name it Arrowverse.

Connect to Switches: Use Copper Straight-Through cables. Connect GigabitEthernet0/0 on the Router to GigabitEthernet0/1 on the Team Arrow Switch. Connect GigabitEthernet0/1 on the Router to GigabitEthernet0/1 on the Team Flash Switch.

Configure Router: Click the Arrowverse router and go to the CLI tab. When asked "Would you like to enter the initial configuration dialog?", type no and press Enter. Then type:

enable
configure terminal

! Connect to Team Arrow LAN
interface GigabitEthernet0/0
 ip address 192.168.1.254 255.255.255.0
 no shutdown
 exit

! Connect to Team Flash Server LAN
interface GigabitEthernet0/1
 ip address 192.168.2.254 255.255.255.0
 no shutdown
 exit

Update Default Gateways: Go back to every PC and the Server -> Desktop -> IP Configuration.

Set the Default Gateway for the Team Arrow PCs to 192.168.1.254.

Set the Default Gateway for the Team Flash devices to 192.168.2.254.

4. VLAN Segmentation (Team Arrow LAN)

Task: Split the 4-PC switch into VLAN 10 and VLAN 20. Assign 2 PCs to each on access ports. Configure a trunk link to the router.

Packet Tracer Steps:

Click the Team Arrow Switch and go to the CLI tab. Press Enter and type:

enable
configure terminal

! Create VLANs
vlan 10
 name oli_and_dig
vlan 20
 name Felicity_and_Roy
exit

! Assign Oliver and John to VLAN 10
interface range fastEthernet 0/1-2
 switchport mode access
 switchport access vlan 10

! Assign Felicity and Roy to VLAN 20
interface range fastEthernet 0/3-4
 switchport mode access
 switchport access vlan 20
exit

! Configure the Trunk link to the Arrowverse Router
interface gigabitEthernet 0/1
 switchport mode trunk
 exit

5. Inter-VLAN to Server (Router-on-a-Stick)

Task: Ensure PCs in VLAN 10/20 can reach the Web Server. Configure router-on-a-stick.

Packet Tracer Steps:

Re-IP the VLAN PCs: Since they are now in segmented networks, go to the Desktop -> IP Configuration of the Team Arrow PCs and update them:

Oliver Queen: IP 192.168.10.2, Gateway 192.168.10.1

John Diggle: IP 192.168.10.3, Gateway 192.168.10.1

Felicity Smoak: IP 192.168.20.3, Gateway 192.168.20.1

Roy Harper: IP 192.168.20.4, Gateway 192.168.20.1

Configure Sub-interfaces: Click the Arrowverse Router, go to the CLI tab, and enter:

enable
configure terminal

! Create Sub-interface for VLAN 10
interface GigabitEthernet0/0.10
 encapsulation dot1Q 10
 ip address 192.168.10.1 255.255.255.0
 exit

! Create Sub-interface for VLAN 20
interface GigabitEthernet0/0.20
 encapsulation dot1Q 20
 ip address 192.168.20.1 255.255.255.0
 exit

6. Packet Flow Analysis

Task: Use Simulation mode to capture a packet from VLAN 10 to the Web Server. Observe MAC, IP, VLAN tags, and routing steps.

Packet Tracer Steps:

In the bottom-right corner of Packet Tracer, click on Simulation (or press Shift+S).

Click Show All/None in the Simulation Panel to hide all protocols, then click Edit Filters and select ICMP (IPv4).

Select the Add Simple PDU tool (the closed envelope icon on the top toolbar).

Click on Oliver Queen (Source), then click on The Monitor (Destination).

Click the Play button in the Simulation Panel and watch the packet travel. Click on the colored square next to the packet to inspect its layers.

What you will observe:

In PC (Layer 3): Oliver's PC compares the destination IP to its own subnet. Since they differ, it resolves the MAC address of its Default Gateway (192.168.10.1) via ARP.

In Switch (Layer 2): The Team Arrow switch receives the Ethernet frame on Fa0/1. As it forwards the frame out the Gig0/1 trunk port, it inserts an 802.1Q tag for VLAN 10.

In Router (Decapsulation): The Arrowverse router receives the tagged frame on Gig0/0.10. It strips the 802.1Q tag and the Ethernet header, exposing the IP packet.

In Router (Routing): The router checks its routing table for 192.168.2.0/24 and finds it directly connected to Gig0/1.

In Router (Re-encapsulation): It creates a new Ethernet frame. The Source MAC is now the router's Gig0/1 interface, and the Destination MAC is The Monitor server. It sends the untagged frame to the Team Flash switch.

Final Network Topology State (Logical Graph)

                                   [ ROUTER: Arrowverse ]
                                  /                      \
                (Gig0/1: 192.168.2.254)          (Gig0/0 Trunk: 192.168.1.254)
                       /                                   \
                      /                                (Gig0/0.10: 192.168.10.1)
                     /                                 (Gig0/0.20: 192.168.20.1)
                    /                                        \
       [ SWITCH: Team Flash ]                       [ SWITCH: Team Arrow ]
       (Subnet: 192.168.2.0/24)                     (VLAN 10 & VLAN 20)
           /       |       \                           /              \
          /        |        \                         /                \
      (Fa0/3)   (Fa0/1)   (Fa0/2)                [VLAN 10]          [VLAN 20]
        /          |          \                   /     \            /     \
   [Server]      [PC]        [PC]              [PC]     [PC]       [PC]    [PC]
     The         Iris        Barry            Oliver    John     Felicity   Roy
   Monitor    West-Allen     Allen            Queen    Diggle     Smoak    Harper
    (.2.1)      (.2.2)       (.2.3)          (.10.2)  (.10.3)    (.20.3)  (.20.4)

