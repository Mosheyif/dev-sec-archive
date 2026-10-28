# *********************************************************
#
# Description: Implementation of an DNS enumerator tool
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import os
import sys
import argparse
import dns.resolver
import dns.exception
import concurrent.futures

# --- Functions ---

def check_single_subdomain(subdomain: str, target_domain: str) -> str:
    """
    Worker function: Resolves a single FQDN.
    Returns a formatted string if found, or None if it doesn't exist.
    """
    fqdn = f"{target_domain}.{subdomain}"
    resolver = dns.resolver.Resolver()

    # Strict timeouts to prevent threads from hanging on dropped UDP packets
    resolver.timeout = 2
    resolver.lifetime = 2

    try:
        answers = resolver.resolve(fqdn, "A")
        ips = ", ".join([ip.to_text() for ip in answers])
        return f"[+] FOUND: {fqdn} -> {ips}"

    except dns.resolver.NXDOMAIN:
        return None

    except dns.resolver.NoAnswer:
        return f"[+] FOUND (No IP): {fqdn} exists but has no A record."

    except dns.exception.DNSException:
        return None


# *********************************************************

def parallel_subdomain_check(target_domain: str, subdomains_list: list, max_threads: int = 50) -> None:
    """
    Distributes the DNS queries across a bounded Thread Pool,
    managing graceful teardown upon interruption.
    """
    print(f"[*] Starting concurrent check for {target_domain} with {max_threads} threads...\n")

    executor = concurrent.futures.ThreadPoolExecutor(max_workers=max_threads)

    try:
        future_to_sub = {executor.submit(check_single_subdomain, sub, target_domain): sub for sub in subdomains_list}

        for future in concurrent.futures.as_completed(future_to_sub):
            result = future.result()
            if result:
                print(result)

        executor.shutdown(wait=True)

    except KeyboardInterrupt:
        print("\n[!] Search interrupted by user (Ctrl+C). Initiating graceful shutdown...")
        executor.shutdown(wait=False, cancel_futures=True)
        sys.exit(0)


# *********************************************************

def load_wordlist(filepath: str) -> list:
    """
    Reads a wordlist from a file, safely stripping newlines and empty lines.
    """
    if not os.path.isfile(filepath):
        print(f"[-] Error: Wordlist file '{filepath}' not found.")
        exit(1)

    with open(filepath, 'r', encoding='utf-8') as file:
        return [line.strip() for line in file if line.strip()]


# *********************************************************


# --- Main function ---

def main():
    """Main workflow of DNS enumerator"""

    parser = argparse.ArgumentParser(description="Concurrent DNS Subdomain Enumerator")

    parser.add_argument("-d", "--domain", required=True, help="Target domain (e.g., example.com)")
    parser.add_argument("-w", "--wordlist", required=True, help="Path to the dictionary text file")
    parser.add_argument("-t", "--threads", type=int, default=50, help="Number of concurrent threads (default: 50)")

    args = parser.parse_args()

    dictionary = load_wordlist(args.wordlist)
    if not dictionary:
        print("[-] Error: The provided wordlist is empty.")
        exit(1)

    print(f"[*] Loaded {len(dictionary)} words from {args.wordlist}")

    parallel_subdomain_check(args.domain, dictionary, max_threads=args.threads)

if __name__ == "__main__":
    main()
