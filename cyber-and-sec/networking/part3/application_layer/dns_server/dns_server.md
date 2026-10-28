
# Reciewer: Romy

# Comprehensive Guide: Setting Up and Securing a BIND9 DNS Server in Docker

This guide walks through the process of deploying a BIND9 authoritative DNS server inside a Docker container, resolving host port conflicts, configuring custom zones, enabling forwarding, and securing the server against unauthorized zone transfers (AXFR).

---

## Prerequisites
- A Linux host environment (Ubuntu recommended).
- Docker and Docker Compose installed.
- Sudo privileges.

---

## Step 1: Project Directory Structure
Create a dedicated working directory for your DNS server lab:
```bash
mkdir -p ~/path/to/dns_server/config
cd ~/path/to/dns_server

```

---

## Step 2: Creating the Configuration Files

### 1. Main Configuration File (`config/named.conf`)

This file sets the global options, enables recursion, configures forwarders (e.g., Google and Cloudflare DNS), and includes local zone files.

```text
options {
    directory "/var/cache/bind";
    allow-query { any; };
    recursion yes;
    
    forwarders {
        8.8.8.8;
        1.1.1.1;
    };
    forward first;
};

include "/etc/bind/named.conf.local";

```

### 2. Local Zone Declaration (`config/named.conf.local`)

Define your custom master zone and initially allow zone transfers.

```text
zone "student.student" {
    type master;
    file "/etc/bind/db.student.student";
    allow-transfer { any; };
};

```

### 3. Zone Data File (`config/db.moshe.yifrah`)

Define the DNS records (SOA, NS, A records) for your domain.

```text
$TTL    604800
@       IN      SOA     ns1.student.student. admin.student.student. (
                        3       ; Serial
                    604800      ; Refresh
                     86400      ; Retry
                   2419200      ; Expire
                    604800 )    ; Negative Cache TTL
;
@       IN      NS      ns1.student.student.
ns1     IN      A       10.0.0.90
@       IN      A       10.0.0.1
admin   IN      A       10.0.0.2
backup  IN      A       10.0.0.3

```

---

## Step 3: Handling Host Port 53 Conflicts

By default, modern Linux distributions run `systemd-resolved` or a local BIND instance that binds to port 53, conflicting with Docker.

1. **Stop and disable host DNS conflicting services:**
```bash
sudo systemctl stop systemd-resolved
sudo systemctl disable systemd-resolved
sudo systemctl stop named
sudo systemctl disable named

```


2. **Verify that port 53 is completely free:**
```bash
sudo ss -tulpn | grep :53

```


*(Ensure no lingering processes are occupying port 53 on `127.0.0.1`).*

---

## Step 4: Docker Compose Deployment

Create a `docker-compose.yml` file to run the BIND9 container:

```yaml
version: '3.8'

services:
  bind9:
    image: ubuntu/bind9:latest
    container_name: bind9-server
    ports:
      - "127.0.0.1:53:53/tcp"
      - "127.0.0.1:53:53/udp"
    volumes:
      - ./config:/etc/bind
    restart: unless-stopped

```

Bring up the container:

```bash
docker compose up -d

```

---

## Step 5: Testing and Verification

### 1. Query Specific Records (`nslookup`)

Verify that your local BIND server correctly resolves custom A records:

```bash
nslookup admin.student.student 127.0.0.1
nslookup backup.student.student 127.0.0.1

```

*Expected output should return the corresponding IP addresses (`10.0.0.2`, `10.0.0.3`).*

### 2. Perform a Zone Transfer (`dig axfr`)

Test if the server allows full zone transfers (AXFR) while `allow-transfer` is set to `any`:

```bash
dig axfr student.student @127.0.0.1

```

*Expected output should list all records defined in the zone file.*

---

## Step 6: Securing the Zone (Disabling Zone Transfer)

To prevent unauthorized users from downloading your entire DNS zone structure, disable the transfer capability.

1. **Update `config/named.conf.local` to block transfers:**
```text
zone "student.student" {
    type master;
    file "/etc/bind/db.moshe.yifrah";
    allow-transfer { none; };
};

```


2. **Restart the container to apply changes:**
```bash
docker compose restart

```


3. **Verify the security restriction:**
```bash
dig axfr student.student @127.0.0.1

```


*Expected output:* `; Transfer failed.` indicating that the security policy is successfully blocking zone enumeration.

```

