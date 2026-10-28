

# Guide: Running Multiple TCP/UDP Servers

## Overview
This guide demonstrates the behavior of the operating system's network stack when attempting to run multiple TCP or UDP servers simultaneously. Specifically, it explores the results of binding to the same port versus binding to different ports. 

When a server application attempts to listen for incoming connections, it must execute the `bind()` system call to claim a specific port. By default, the operating system prevents multiple processes from binding to the exact same port and protocol combination concurrently. This prevents routing ambiguity—if two applications listened on the same port, the OS would not know which application should receive the incoming network packets.

---

## Scenario 1: Attempting to Bind to the Same Port

If you attempt to launch a second server on a port that is already in use by another active server, the OS will reject the request, and the `bind()` system call will fail. 

**Terminal 1 (Server 1):**
Start the first server on port `5000`.
```bash
user@demo:~/path/to/tcp$ ./tcp_pong 5000
Waiting for TCP connection on port 5000...

```

**Terminal 2 (Server 2):**
Attempt to start a second server on the exact same port (`5000`).

```bash
user@demo:~/path/to/tcp$ ./tcp_pong 5000
bind() failed: Address already in use

```

*Note: The exact same `Address already in use` error applies to the UDP implementation (`./udp_pong 5000`) when the port is occupied.*

---

## Scenario 2: Binding to Different Ports

To successfully run multiple servers on the same machine, each server must be assigned a unique port number. This allows the OS to route traffic appropriately based on the destination port found in the packet header.

**Terminal 1 (Server 1):**
Successfully running and listening on port `5000`.

```bash
user@demo:~/path/to/tcp$ ./tcp_pong 5000
Waiting for TCP connection on port 5000...
Client connected! You can now type messages at any time:

[Client]: Hello from Client 1
Reply for Client 1

Client closed the connection.

```

**Terminal 2 (Server 2):**
Successfully running concurrently on port `5001`.

```bash
user@demo:~/path/to/tcp$ ./tcp_pong 5001
Waiting for TCP connection on port 5001...
Client connected! You can now type messages at any time:

[Client]: Hello from Client 2
Reply for Client 2

Client closed the connection.

```

**Terminal 3 (Client 1):**
Connects to Server 1 via port `5000`.

```bash
user@demo:~/path/to/tcp$ ./tcp_ping 127.0.0.1 5000
Connecting to 127.0.0.1:5000...
Connection established. Type messages to send at any time:
Hello from Client 1

[Server]: Reply for Client 1

```

**Terminal 4 (Client 2):**
Connects to Server 2 via port `5001`.

```bash
user@demo:~/path/to/tcp$ ./tcp_ping 127.0.0.1 5001
Connecting to 127.0.0.1:5001...
Connection established. Type messages to send at any time:
Hello from Client 2

[Server]: Reply for Client 2

```

```
