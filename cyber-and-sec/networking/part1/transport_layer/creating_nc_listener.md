# Setting Up a Client-Server Connection with Netcat

The `nc` (Netcat) tool allows you to easily create a TCP connection between two
shells. One machine acts as the server (listener) and the other as the client.

## Server Side (Setting Up the Listener)

Use the `-l` (Listen) flag to open a port and wait for incoming connections:

```bash
nc -l <ip> <port>
```

> **Tip:** Add `-v` (verbose) and `-n` (no DNS resolution) flags in real-world
> use to get better output and avoid slow DNS lookups:
>
> ```bash
> nc -nlv <ip> <port>
> ```

## Client Side (Connecting to the Listener)

In a second shell, or from a different machine on the network, connect to the
address and port where the server is listening:

```bash
nc <ip> <port>
```

Once connected, anything typed on either side is transmitted to the other.
