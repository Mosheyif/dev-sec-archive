# IPv6 Practice: Step-by-Step Guide

Brief guide on executing IPv6 tasks from the assignment using Linux commands.

## 1. Identify Your IPv6 Network, Address, and Provider

```bash
ip -6 address show
# or: ip -6 a
```

Look for lines starting with `inet6`:

- The long hexadecimal string is your **address**.
- The number after `/` (e.g., `/64`) is the **network prefix**.
- Addresses starting with `fe80::` are self-generated **Link-Local** addresses.
- All other addresses are typically provided by your router or ISP via SLAAC or
  DHCPv6.

## 2. Configure a New IPv6 Address

```bash
sudo ip -6 address add fd00:1111::1/64 dev eth0
```

Replace `eth0` with your actual interface name. Verify it was added:

```bash
ip -6 a
ping6 fd00:1111::1
```

## 3. Ping Another Student's Machine

```bash
ping <student_ipv6_address>
```

> **Note:** For `fe80::` Link-Local addresses, specify the interface using `%`:
>
> ```bash
> ping fe80::a1b2%eth0
> ```

## 4. Direct Physical Connection and Network Setup

Connect two machines directly with an Ethernet cable and put them on the same
custom IPv6 subnet:

```bash
# On Machine A
sudo ip -6 address add fd00:cafe:1::1/64 dev eth0

# On Machine B
sudo ip -6 address add fd00:cafe:1::2/64 dev eth0

# Verify the direct link from Machine A
ping fd00:cafe:1::2
```

## 5. Define Routes and Observe Packets

**Add a static route:**

```bash
sudo ip -6 route add <destination_network>/64 via <gateway_ip> dev <interface>
# Example:
sudo ip -6 route add fd00:cafe:2::/64 via fd00:cafe:1::2 dev eth0
```

**Observe ICMPv6 traffic with tcpdump:**

```bash
sudo tcpdump -i eth0 -n icmp6
```

While `tcpdump` is running, send a ping to the routed destination. You will see
the ICMPv6 Echo Request and Reply packets flowing through the interface.
