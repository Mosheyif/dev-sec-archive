

# Infrastructure & Web Reconnaissance

**Overview:** 
This guide provides a comprehensive, step-by-step walkthrough for executing network reconnaissance and mapping a target's attack surface. It transitions from passive external DNS enumeration to active service identification, directory fuzzing, and custom dictionary generation.

---

## 1. Subdomain Mapping with `dnsenum`

**Objective:** Use `dnsenum` for mapping subdomains of `infinitylabs.co.il`.

**Execution:**
We utilize `dnsenum` with the `-f` flag to specify our dictionary file. This forces the resolution of hidden or unlinked subdomains that administrators might assume are undiscoverable.

```bash
student@host:~/recon$ dnsenum -f subdomains-top1million-110000.txt infinitylabs.co.il

```

**Output:**

```text
dnsenum VERSION:1.2.6

-----   infinitylabs.co.il   -----

Host's addresses:
__________________
infinitylabs.co.il.                      300      IN    A        104.21.11.11
infinitylabs.co.il.                      300      IN    A        172.67.147.130

Brute forcing with subdomains-top1million-110000.txt:
______________________________________________________

www.infinitylabs.co.il.                  300      IN    A        91.202.168.224
vpn.infinitylabs.co.il.                  300      IN    A        213.57.121.34
soc.infinitylabs.co.il.                  300      IN    CNAME    base44.onrender.com.

```

*(Output truncated for brevity)*

> **Real-World Impact (The SOC Platform Discovery):**
> Notice how this seemingly routine reconnaissance step directly exposed the `soc.infinitylabs.co.il` subdomain. This domain hosts the internal Incident Response (IR) and security management platform. This perfectly illustrates the critical failure of "Security by Obscurity." The platform was designed strictly for internal crisis management, but because its DNS record was publicly resolvable, a simple dictionary attack revealed its existence instantly.
> 
> 

* **Dictionary Used:** `subdomains-top1million-110000.txt`.


* **Alternative Dictionary:** `bitquark-subdomains-top100000.txt` or similar robust SecLists alternatives.

---

## 2. Web Content Discovery Concepts

**Objective:** Understand what `dirb` is used for and identify a suitable dictionary.

**Methodology:**
`dirb` is an active Web Content Scanner designed to brute-force web servers to uncover hidden directories, unlinked files, and administrative panels. Unlike DNS enumeration tools, it requires path-based dictionaries.

* **Suitable Dictionary:** `/usr/share/dirb/wordlists/common.txt` or `directory-list-2.3-medium.txt` from SecLists.



---

## 3. Active Port Scanning (TCP & UDP)

**Objective:** Use `nmap` to scan the revealed hosts on UDP and TCP.

**Execution:**
A comprehensive scan requires mapping both protocols. We use `-sS` for TCP SYN and `-sU` for UDP. To prevent the UDP scan from hanging excessively, optimization flags (`-T4`, `--top-ports 50`, `--max-retries 1`) are applied.

```bash
student@host:~/recon$ sudo nmap -sS -sU -T4 --top-ports 50 --max-retries 1 -v 216.24.57.16

```

**Output:**

```text
...
PORT     STATE         SERVICE
21/tcp   open          ftp
22/tcp   filtered      ssh
80/tcp   open          http
443/tcp  open          https
8080/tcp open          http-proxy
5060/tcp open          sip
8443/tcp open          https-alt
53/udp   open|filtered domain

```

---

## 4. Advanced Directory Fuzzing

**Objective:** Use `dirb` to find a valid URL inside one of the hosts you have found.

**Execution:**
We execute `dirb` against the target host. Because web servers often return unstable 30X redirects or false 404s, appending the `-f` (fine-tuning) flag helps calibrate the `NOT_FOUND` detection.

```bash
student@host:~/recon$ dirb [http://soc.infinitylabs.co.il](http://soc.infinitylabs.co.il) -f

```

**Output:**

```text
---- Scanning URL: [http://soc.infinitylabs.co.il/](http://soc.infinitylabs.co.il/) ----
> Testing: [http://soc.infinitylabs.co.il/alumni_info](http://soc.infinitylabs.co.il/alumni_info)                           
+ [http://soc.infinitylabs.co.il/api](http://soc.infinitylabs.co.il/api) (CODE:403|SIZE:154)
+ [http://soc.infinitylabs.co.il/healthz](http://soc.infinitylabs.co.il/healthz) (CODE:200|SIZE:2)

```

---

## 5. Internal Network Mapping (Ping Sweep)

**Objective:** Scan the internal network of `infinitylabs` and find live hosts.

**Execution:**
Once pivoted into an internal network (e.g., via VPN), a Ping Sweep (`-sn`) is the most efficient way to map live IP addresses across a subnet without the noise and delay of a full port scan.

```bash
student@host:~/recon$ sudo nmap -sn 10.10.10.0/24

```

**Output:**

```text
Starting Nmap 7.94SVN ( [https://nmap.org](https://nmap.org) )
Nmap scan report for 10.10.10.1
Host is up (0.0010s latency).
Nmap scan report for 10.10.10.45
Host is up (0.0021s latency).
Nmap scan report for 10.10.10.102
Host is up (0.0015s latency).
Nmap done: 256 IP addresses (3 hosts up) scanned in 2.15 seconds

```

---

## 6. Service & Version Detection

**Objective:** Pick up one of those hosts and list the open services on that host.

**Execution:**
Targeting a specific live internal host (`10.10.10.45`), we perform a Version Detection scan (`-sV`) across all 65,535 ports (`-p-`) to map the exact software and services running on the machine.

```bash
student@host:~/recon$ sudo nmap -sV -p- 10.10.10.45

```

**Output:**

```text
PORT     STATE SERVICE       VERSION
22/tcp   open  ssh           OpenSSH 8.4p1 Debian 5+deb11u1
80/tcp   open  http          nginx 1.18.0
3306/tcp open  mysql         MySQL 8.0.27

```

---

## 7. Custom Wordlist Generation (`crunch`)

**Objective:** Using `crunch`, create all the possible phone numbers in Israel, with and without `+972` prefix.

**Execution:**
Standard dictionaries often fail against localized targets. We use `crunch` with the `-t` flag to lock the known prefixes and replace the remaining characters with `%` (which dictates numerical generation).

```bash
# Without +972 prefix (10 digits starting with 05)
student@host:~/recon$ crunch 10 10 -t "05%%%%%%%%" -o il_phones_local.txt

# With +972 prefix (13 characters starting with +9725)
student@host:~/recon$ crunch 13 13 -t "+9725%%%%%%%%" -o il_phones_intl.txt

```

**Output:**

```text
Crunch will now generate the following amount of data: 1400000000 bytes
Crunch will now generate the following number of lines: 100000000

```

---

## 8. Passive Intelligence Gathering (OSINT)

**Objective:** Spend some time clicking around "OSINT Framework" website and exploring the various categories and tools available.

**Methodology:**
Navigating through `https://osintframework.com`, the focus shifts from active scanning (which leaves forensic logs on target servers) to passive reconnaissance.
Key categories explored:

1. **Domain Name -> Subdomain Search:** Identifying external platforms that index subdomains passively without running direct queries.


2. **Email Addresses:** Discovering tools to harvest employee emails, which are critical for building targeted phishing campaigns or credential stuffing attacks.
