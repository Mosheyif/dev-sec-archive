# DNS Configuration: Finding, Understanding, and Changing DNS

This guide covers how to identify your current DNS servers, understand the
architecture behind modern Linux DNS resolvers, and manually change your DNS
to faster, more secure public servers (like Google's `8.8.8.8` or Cloudflare's
`1.1.1.1`).

## 1. How to Find Your Current DNS Server

**The Modern Way:** Use the systemd resolution tool:

```bash
resolvectl status
```

Scroll down to your active interface (e.g., `eth0` or `wlo1`) and look for the
`DNS Servers` entry.

**The Universal Way:**

```bash
nslookup google.com
```

## 2. Understanding the Linux DNS Architecture

When Linux users run `cat /etc/resolv.conf` or `nslookup`, they often see
`127.0.0.53` as their DNS server. However, `resolvectl status` shows a
completely different IP (like `192.168.1.1` or `10.1.0.254`).

Modern Linux distributions (like Ubuntu) use **systemd-resolved**, which
creates a chain of communication:

- **Stub Resolver (`127.0.0.53`):** A tiny, virtual DNS server running locally.
  When your browser needs a website, it asks this local stub first.
- **Upstream DNS (e.g., `10.1.0.254`):** The local stub forwards requests to
  the real upstream DNS server (usually your router, assigned via DHCP).

**Why this architecture?**

- **Caching:** If you visit `youtube.com` twice, the local stub remembers the
  IP and answers instantly the second time, saving network traffic.
- **Split DNS:** When connected to a corporate VPN, the stub routes company
  domains to the VPN's DNS server and public domains to your home router,
  without manual configuration.

## 3. How to Change Your DNS Server

Changing to a public provider (like `1.1.1.1` or `8.8.8.8`) can improve
browsing speed, bypass ISP censorship, and enhance privacy.

Since `/etc/resolv.conf` is dynamically managed, editing it manually is
useless — it will be overwritten on reboot. Use `nmcli` instead.

**Step 1:** Find your active connection name:

```bash
nmcli connection show
```

Look under the `NAME` column for your active connection (e.g.,
`"Wired connection 1"`).

**Step 2:** Set the new DNS servers (replace the connection name as needed):

```bash
sudo nmcli connection modify "Wired connection 1" ipv4.dns "1.1.1.1 1.0.0.1"
```

**Step 3:** Ignore the router's automatic DNS from DHCP (critical — otherwise
Linux uses both your custom DNS and the router's DNS simultaneously):

```bash
sudo nmcli connection modify "Wired connection 1" ipv4.ignore-auto-dns yes
```

**Step 4:** Restart the connection to apply changes:

```bash
sudo nmcli connection up "Wired connection 1"
```

**Step 5:** Verify the change:

```bash
resolvectl status
```
