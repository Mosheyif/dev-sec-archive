

# Guide: Distributed Network Execution & Broadcasting

## Overview
This guide demonstrates how to transition from local network testing (`localhost`) to a distributed physical or virtual network architecture. It covers executing basic Ping-Pong interactions across different computers, testing the multi-threaded capabilities of our TCP server against multiple remote clients, and validating UDP broadcasting across a local subnet.

For these examples, assume the following network topology:
*   **Machine A (Server/Broadcaster):** IP `192.168.1.100`
*   **Machine B (Client 1):** IP `192.168.1.101`
*   **Machine C (Client 2):** IP `192.168.1.102`
*   **Machine D (Client 3):** IP `192.168.1.103`

---

## Scenario 1: Local Execution vs. Remote Execution

Before distributing the system, we verify that the network stack routes locally using the loopback interface, and then we move the client to a separate physical machine.

### Part A: Executing Locally (Loopback)
Both programs run on the same computer using `127.0.0.1`.

**Terminal 1 (Server on Machine A):**
```bash
user@machine-A:~/tcp$ ./tcp_pong 5000
Waiting for TCP connections on port 5000...

```

**Terminal 2 (Client on Machine A):**

```bash
user@machine-A:~/tcp$ ./tcp_ping 127.0.0.1 5000
Connecting to 127.0.0.1:5000...
Connection established. Type messages to send at any time:

```

### Part B: Executing Remotely (LAN)

The server remains on Machine A, but the client is executed from Machine B, targeting Machine A's LAN IP.

**Terminal 1 (Server on Machine A):**

```bash
user@machine-A:~/tcp$ ./tcp_pong 5000
Waiting for TCP connections on port 5000...
[System]: New client connected (Slot 0).

```

**Terminal 2 (Client on Machine B):**

```bash
user@machine-B:~/tcp$ ./tcp_ping 192.168.1.100 5000
Connecting to 192.168.1.100:5000...
Connection established. Type messages to send at any time:
Hello from Machine B!

```

---

## Scenario 2: Multi-Threaded TCP Server with 3 Remote Clients

This scenario tests the `pthreads` architecture of our TCP server. We will boot the server on Machine A and connect three distinct physical/virtual machines to it simultaneously. The server will assign a dedicated thread to each client.

**Terminal 1 (TCP Server on Machine A):**
Start the server and wait for incoming connections.

```bash
user@machine-A:~/tcp$ ./tcp_pong 5000
Waiting for TCP connections on port 5000...
Server running. Type messages at any time to broadcast to all clients:

[System]: New client connected (Slot 0).

[System]: New client connected (Slot 1).

[System]: New client connected (Slot 2).

[Client sock 4]: Ping from B
[Client sock 5]: Ping from C
[Client sock 6]: Ping from D

Global Broadcast Message

```

**Terminal 2 (Client 1 on Machine B):**

```bash
user@machine-B:~/tcp$ ./tcp_ping 192.168.1.100 5000
Connecting to 192.168.1.100:5000...
Connection established. Type messages to send at any time:
Ping from B

[Server]: Global Broadcast Message

```

**Terminal 3 (Client 2 on Machine C):**

```bash
user@machine-C:~/tcp$ ./tcp_ping 192.168.1.100 5000
Connecting to 192.168.1.100:5000...
Connection established. Type messages to send at any time:
Ping from C

[Server]: Global Broadcast Message

```

**Terminal 4 (Client 3 on Machine D):**

```bash
user@machine-D:~/tcp$ ./tcp_ping 192.168.1.100 5000
Connecting to 192.168.1.100:5000...
Connection established. Type messages to send at any time:
Ping from D

[Server]: Global Broadcast Message

```

---

## Scenario 3: UDP Broadcast to Multiple Listeners

In this scenario, we test the `SO_BROADCAST` implementation. Three different machines will act as passive listeners (using `udp_pong` which binds to `0.0.0.0` to receive all incoming datagrams on a specific port). A single server will blast a broadcast message to the entire subnet (`255.255.255.255`).

**Terminal 1 (Listener 1 on Machine B):**

```bash
user@machine-B:~/udp$ ./udp_pong 5000
Waiting for first message on port 5000...

[Client]: ANNOUNCEMENT: Broadcast test from node
[Client]: ANNOUNCEMENT: Broadcast test from node

```

**Terminal 2 (Listener 2 on Machine C):**

```bash
user@machine-C:~/udp$ ./udp_pong 5000
Waiting for first message on port 5000...

[Client]: ANNOUNCEMENT: Broadcast test from node
[Client]: ANNOUNCEMENT: Broadcast test from node

```

**Terminal 3 (Listener 3 on Machine D):**

```bash
user@machine-D:~/udp$ ./udp_pong 5000
Waiting for first message on port 5000...

[Client]: ANNOUNCEMENT: Broadcast test from node
[Client]: ANNOUNCEMENT: Broadcast test from node

```

**Terminal 4 (Broadcaster on Machine A):**
Execute the broadcast tool. It will utilize the LAN's switch to duplicate the packets to all machines on the subnet.

```bash
user@machine-A:~/udp$ ./udp_broadcast 5000
Starting UDP broadcast on port 5000...
Broadcast message sent.
Broadcast message sent.

```

```
