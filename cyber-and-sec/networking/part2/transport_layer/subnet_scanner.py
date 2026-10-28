# *********************************************************
#
# Description: Implementation of a subnet specific port scanner
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from port_scanner import port_scanner, print_banner, parse_arguments
import ipaddress
import sys

# --- Functions ---

def generate_target_ips(cidr_target):
    """
    Parses a CIDR notation string and yields usable host IP addresses.
    """
    try:
        network = ipaddress.ip_network(cidr_target, strict=False)

        total_hosts = network.num_addresses - 2
        print(f"\n[*] Total usable hosts to scan: {total_hosts}")

        return network.hosts()

    except ValueError as e:
        print(f"[!] Invalid Network/IP syntax: {e}", file=sys.stderr)
        sys.exit(1)


# --- Main Execution ---

def main():
    args = parse_arguments()

    subnet = args.target
    port = int(args.ports)

    if port < 1 or port > 65535:
        print_banner(" Invalid port ")
        sys.exit(1)

    print_banner(" Subnet Scanner ")
    print_banner(f" Subnet: {subnet} ")
    print_banner(f" Port: {port} ")
    print_banner(" Scanning... ")

    for ip in generate_target_ips(subnet):
        scan_result = port_scanner(
            str(ip),
            [port],
            args.show_closed,
            args.show_filtered,
            args.show_all
        )

        if scan_result:
            print(f"\n[+] IP: {ip}")
            for p, status in scan_result:
                formatted_string = f"\nPort {p: <5} : {status}"
                print(formatted_string)


if __name__ == "__main__":
    main()


