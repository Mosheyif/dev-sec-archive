# Port Scanning with Netcat

Netcat (`nc`) can be used as a basic and fast port scanner to check which
services (ports) are open and listening on a target machine.

## TCP Port Scan

The `-z` flag tells Netcat to only attempt a connection (Zero-I/O, without
sending data). Use `-v` (verbose) to display results on screen.

```bash
nc -zv <Target_IP> <Start_Port>-<End_Port>
```

## Filtering Results (Open Ports Only)

By default, closed ports are also printed ("Connection refused"). To show only
open ports, redirect stderr to stdout and filter with `grep`:

```bash
nc -zv 192.168.1.50 20-1000 2>&1 | grep "succeeded"
```

## UDP Port Scan

Add the `-u` flag to scan UDP ports instead of TCP:

```bash
nc -zvu 192.168.1.50 50-60
```

> **Note:** UDP scanning is inherently less reliable than TCP scanning because
> UDP does not send acknowledgment packets.
