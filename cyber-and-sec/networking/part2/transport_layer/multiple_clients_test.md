

# Guide: Running Multiple TCP/UDP Clients

## Overview
This guide explores how the operating system and our specific server implementations handle multiple client applications attempting to communicate with a single server simultaneously. The results highlight the fundamental differences between connection-oriented (TCP) and connectionless (UDP) protocols, as well as the capabilities of a multi-threaded server architecture.

---

## Scenario 1: TCP Clients (Connection-Oriented, Multi-Threaded)

TCP requires a dedicated, stateful connection between the client and the server (the 3-way handshake). Our updated `tcp_pong` server utilizes a multi-threaded architecture (`pthreads`) to handle concurrent connections. An Acceptor thread continuously listens for incoming connections and spawns a dedicated Client Handler thread for each new client. The Main thread allows the server to broadcast messages to all active clients simultaneously.

Because each client has its own dedicated thread and socket descriptor, multiple clients can connect, send messages, and receive broadcasts concurrently without experiencing blocking or connection drops.

**Terminal 1 (TCP Server):**
Starts the multi-threaded server. It successfully accepts three clients, assigns them to slots, receives their individual messages, broadcasts "Bye" to all of them, and registers their disconnections.
```bash
user@demo:~/path/to/tcp$ ./tcp_pong 5000
Waiting for TCP connections on port 5000...
Server running. Type messages at any time to broadcast to all clients:

[System]: New client connected (Slot 0).

[System]: New client connected (Slot 1).

[System]: New client connected (Slot 2).

[Client sock 4]: Hello1

[Client sock 5]: Hello2

[Client sock 6]: Hello3
Bye

[System]: Client on sock 6 disconnected.

[System]: Client on sock 5 disconnected.

[System]: Client on sock 4 disconnected.

```

**Terminal 2 (TCP Client 1):**
Successfully connects, sends its message, and receives the server's broadcast.

```bash
user@demo:~/path/to/tcp$ ./tcp_ping 127.0.0.1 5000
Connecting to 127.0.0.1:5000...
Connection established. Type messages to send at any time:
Hello1

[Server]: Bye

```

**Terminal 3 (TCP Client 2):**
Successfully connects alongside Client 1, sends its message, and receives the broadcast.

```bash
user@demo:~/path/to/tcp$ ./tcp_ping 127.0.0.1 5000
Connecting to 127.0.0.1:5000...
Connection established. Type messages to send at any time:
Hello2

[Server]: Bye

```

**Terminal 4 (TCP Client 3):**
Successfully connects alongside the other clients, sends its message, and receives the broadcast.

```bash
user@demo:~/path/to/tcp$ ./tcp_ping 127.0.0.1 5000
Connecting to 127.0.0.1:5000...
Connection established. Type messages to send at any time:
Hello3

[Server]: Bye

```

---

## Scenario 2: UDP Clients (Connectionless)

UDP does not establish a persistent connection. The server simply listens on a port and receives independent datagrams from anyone who sends them using `recvfrom()`.

Because our `udp_pong` server continuously loops on `recvfrom()`, it can receive messages from multiple clients seamlessly. However, it updates its destination address (`client_addr`) with the identity of the *most recent* sender. Therefore, any reply typed by the server will only be routed to the last client that sent a message.

**Terminal 1 (UDP Server):**
The server successfully receives datagrams from three different clients concurrently. When the server replies "Bye", it sends it to the last known sender (Client 3).

```bash
user@demo:~/path/to/udp$ ./udp_pong 5000
Waiting for first message on port 5000...
Client connected. You can now type messages at any time:

[Client]: Hello1

[Client]: Hello2

[Client]: Hello3
Bye


```

**Terminal 2 (UDP Client 1):**
Sends the first message. Does not receive a reply because the server's target address was overwritten by subsequent clients.

```bash
user@demo:~/path/to/udp$ ./udp_ping 127.0.0.1 5000
Ready. Type messages to send at any time:
Hello1


```

**Terminal 3 (UDP Client 2):**
Sends the second message.

```bash
user@demo:~/path/to/udp$ ./udp_ping 127.0.0.1 5000
Ready. Type messages to send at any time:
Hello2


```

**Terminal 4 (UDP Client 3):**
Sends the third message. Because this was the last datagram received by the server, this client successfully receives the server's reply.

```bash
user@demo:~/path/to/udp$ ./udp_ping 127.0.0.1 5000
Ready. Type messages to send at any time:
Hello3

[IP: 127.0.0.1]: Bye


```

```
