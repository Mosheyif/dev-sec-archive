# Lab Guide: Hierarchical DNS Infrastructure

The goal is to configure a hierarchical DNS infrastructure with a primary root
domain (TLD) and several subdomain servers. One student sets up the TLD, each
subsequent student sets up a zone for a subdomain under their friend's domain,
and the last student stores a specific TXT record.

## 1. Installation (All Students)

Install the BIND9 DNS server and its utilities on all participating machines:

```bash
sudo apt update
sudo apt install bind9 bind9-utils
```

## 2. Zone Configuration (Role-Based)

Every student must define their zone in `/etc/bind/named.conf.local` and create
a corresponding forward zone file in `/etc/bind/`.

### Role A: The TLD / Root Server (Student 1)

You are responsible for the base domain. Configure an `NS` record for yourself
and a delegation (`NS` record and glue `A` record) pointing to Student 2.

`/etc/bind/named.conf.local`:

```text
zone "student1" {
    type master;
    file "/etc/bind/db.student1";
};
```

`/etc/bind/db.student1`:

```dns
$TTL 300
@   IN  SOA ns.student1. admin.student1. ( 2026083101 3600 1800 604800 300 )
@           IN  NS  ns.student1.
ns          IN  A   <YOUR_IP>

; Delegation to Student 2
student2    IN  NS  ns.student2.student1.
ns.student2 IN  A   <STUDENT_2_IP>
```

### Role B: Intermediate Subdomains (Students 2 to N-1)

You manage a subdomain chained to the previous student. Define your own `NS`
record and delegate the next level to the next student in line.

`/etc/bind/named.conf.local`:

```text
zone "student2.student1" {
    type master;
    file "/etc/bind/db.student2";
};
```

`/etc/bind/db.student2`:

```dns
$TTL 300
@   IN  SOA ns.student2.student1. admin.student2.student1. ( 2026083101 3600 1800 604800 300 )
@           IN  NS  ns.student2.student1.
ns          IN  A   <YOUR_IP>

; Delegation to Next Student
student3    IN  NS  ns.student3.student2.student1.
ns.student3 IN  A   <NEXT_STUDENT_IP>
```

### Role C: The Final Server (Last Student)

You manage the deepest domain in the hierarchy. You do **not** delegate any
further domains. Instead, store a TXT record with "You have succeeded!".

Zone file snippet:

```dns
$TTL 300
@   IN  SOA ns.student5.student4... admin.student5... ( 2026083101 3600 1800 604800 300 )
@   IN  NS  ns.student5.student4...
ns  IN  A   <YOUR_IP>

; Final Required Record
@   IN  TXT "You have succeeded!"
```

## 3. Validation & Application (All Students)

Before testing the chain, every student must verify their local syntax and
restart the daemon:

```bash
sudo named-checkconf
sudo named-checkzone <YOUR_ZONE_NAME> /etc/bind/db.<YOUR_ZONE_FILE>
sudo systemctl restart bind9
```

## 4. Hierarchical Testing

Use `nslookup`, `dig`, or `host` to perform DNS queries. Test the setup by
querying the TLD NS (Student 1) from any machine:

```bash
dig @<STUDENT_1_IP> TXT student5.student4.student3.student2.student1 +short
```

If the glue records and delegations are correct, the query will recursively
traverse the hierarchy and return the TXT record on screen.
