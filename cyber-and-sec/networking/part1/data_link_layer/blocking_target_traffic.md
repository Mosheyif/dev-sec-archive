# Blocking a Target's Network Traffic via ARP Spoofing

**Exercise:** Knowing only its IP address, how would you block all network
traffic of another machine in your LAN?

## Theory

To block a specific machine's traffic by IP alone, you use **ARP Spoofing**
(ARP Poisoning) to execute a Denial of Service attack — often called
"Blackholing." Because ARP is stateless and lacks authentication, devices
blindly accept ARP replies even without requesting them (Gratuitous ARP).

## Process (Theoretical, using Ostinato)

1. **Discover MAC Addresses:** Use the technique in `ostinato_arp_guide.md` to
   discover the MAC address of the target and the default gateway (router).

2. **Craft Poisoned ARP Replies:** Create a new stream in Ostinato configured
   to send continuous ARP Replies (Opcode 2).

3. **Poison the Target:**
   - Ethernet Destination MAC → Target's MAC
   - ARP Sender IP → Gateway's IP
   - ARP Sender MAC → a non-existent (fake) MAC address

4. **Poison the Gateway:**
   - Ethernet Destination MAC → Gateway's MAC
   - ARP Sender IP → Target's IP
   - ARP Sender MAC → a non-existent (fake) MAC address

**Result:** The target updates its ARP cache believing the fake MAC is the
router. All outbound traffic goes into a black hole. The router similarly sends
return traffic into the void. The target is effectively isolated.

## Practical Implementation (arpspoof)

During testing, manual ARP poisoning via Ostinato streams did not reliably
update the target's ARP cache. The `arpspoof` tool from the `dsniff` suite was
used instead:

```bash
# Disable IP forwarding (do not act as MITM, just drop packets)
sudo sysctl -w net.ipv4.ip_forward=0

# Spoof both victim and gateway simultaneously (creates the "black hole")
sudo arpspoof -i <interface> -t <target_ip> -r <gateway_ip>
```

> **Disclaimer:** This information is for educational purposes only. Executing
> ARP spoofing on networks without explicit authorization is illegal and
> unethical.
