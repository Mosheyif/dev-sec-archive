# Blocking a Website Using iptables

The assignment requires blocking access to a specific website using the
built-in Linux firewall, `iptables`.

`iptables` operates at the network and transport layers (Layer 3/4). It does
not understand domain names or full URLs — only IP addresses. The solution
consists of two steps.

## Step 1: Resolve the Website's IP Address

Use `host` or `nslookup` to resolve the domain to an IP:

```bash
host www.calcalist.co.il
```

The command returns the IP addresses of the website's servers. If multiple IPs
are returned, repeat Step 2 for each one.

## Step 2: Block Outbound Traffic in iptables

Add a rule to the OUTPUT chain to silently drop all outbound packets to that
IP:

```bash
sudo iptables -A OUTPUT -d <IP_ADDRESS> -j DROP
```

**Flags explained:**

- `sudo` — root privileges are required to modify firewall settings
- `-A OUTPUT` — append the rule to the outbound traffic chain
- `-d <IP_ADDRESS>` — match packets whose destination is this address
- `-j DROP` — silently discard the packet (the browser will show an infinite
  loading state)

## Step 3: Verify the Block

Try to access the website in a browser, or from the terminal:

```bash
curl -I https://www.calcalist.co.il
ping www.calcalist.co.il
```

The request will time out or return an error, confirming the block is active.

## Step 4: Remove the Block

Use the same command as Step 2, replacing `-A` (Append) with `-D` (Delete):

```bash
sudo iptables -D OUTPUT -d <IP_ADDRESS> -j DROP
```
