

# Guide: DNS Subdomain Enumeration Tool

## 1. Description
This project implements a concurrent DNS Subdomain Enumerator in Python. Operating on the Application Layer, the tool maps the external network infrastructure (Attack Surface) of a target domain. It utilizes the `dnspython` library to query 'A' records and the `concurrent.futures` module to bypass the Python GIL via a Thread Pool, allowing for highly efficient, asynchronous I/O-bound network requests. The tool is fully dynamic, accepting configuration parameters (target domain, wordlist path, thread count) via standard CLI arguments.

---

## 2. Program Flow (Pseudo-code)

The architecture is divided into CLI parsing, file I/O operations, and concurrent network resolution.

```text
Function main():
    Parse CLI arguments (domain, wordlist_path, threads)
    dictionary = load_wordlist(wordlist_path)
    If dictionary is empty: exit(1)
    
    Spawn ThreadPoolExecutor(max_workers=threads)
    For each word in dictionary:
        Submit check_single_subdomain(word, domain) to ThreadPool
    
    As each thread completes (as_completed):
        If result is not None:
            Print result

Function load_wordlist(filepath):
    Open filepath
    Read all lines
    Strip newline ('\n') characters and ignore empty lines
    Return cleaned list of words

Function check_single_subdomain(subdomain, domain):
    fqdn = subdomain + "." + domain
    Initialize DNS Resolver (timeout = 2s)
    
    Try:
        Query DNS for 'A' record of fqdn
        Return formatted string with discovered IP addresses
    Catch NXDOMAIN:
        Return None (Subdomain does not exist)
    Catch NoAnswer:
        Return formatted string (Subdomain exists, but lacks an 'A' record)

```

---

## 3. Step-by-Step Testing Guide

To properly test the DNS enumerator, we must set up an isolated Python virtual environment, prepare a dictionary file, and execute the script using the correct CLI flags.

### Step 1: Environment Setup

To avoid system-wide package conflicts (PEP 668), create and activate a Python virtual environment, then install the required `dnspython` library.

**Terminal Command:**

```bash
python3 -m venv venv
source venv/bin/activate
pip install dnspython

```

### Step 2: Script Execution

Assuming your Python script is saved as `dns_enum.py`, execute it by passing the required arguments: `-d` for the domain and `-w` for the wordlist. You can also specify the number of threads using `-t`.

**Terminal Command:**

```bash
python3 dns_enum.py -d example.com -w subdomains.txt -t 10

```

### Step 4: Expected Output

The script will load the dictionary, dispatch the threads, and asynchronously print valid subdomains as soon as the DNS server responds.

**Terminal Output:**

```bash
[*] Loaded 6 words from subdomains.txt
[*] Starting concurrent check for example.com with 10 threads...

[+] FOUND: [www.example.com](https://www.example.com) -> 93.184.215.14
[+] FOUND (No IP): mail.example.com exists but has no A record.
[+] FOUND: dev.example.com -> 93.184.215.15

```

*(Note: "invalid-test-subdomain" triggers an NXDOMAIN exception in the background and is correctly ignored by the output formatter).*

```

