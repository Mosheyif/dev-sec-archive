# *********************************************************
#
# Description: Implementation of a port scanner using socket module
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import ipaddress
import socket
import errno
import sys
import argparse
from concurrent.futures import ThreadPoolExecutor

# --- Constants ---

BANNER_LENGTH = 60
TIMEOUT_SEC = 0.5
MAX_THREADS = 1000

# --- Functions ---


def get_port_state(target_ip, port):
    """
    Executes a full TCP connection attempt and maps the
    OS response to a standardized network state.
    """
    sock = None
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(TIMEOUT_SEC)

        result = sock.connect_ex((target_ip, port))

        if result == 0:
            return port, "OPEN (SYN-ACK received)"
        elif result == errno.ECONNREFUSED:
            return port, "CLOSED (TCP RST received)"
        elif result in (errno.EHOSTUNREACH, errno.ENETUNREACH):
            return port, "FILTERED (ICMP Unreachable received)"
        elif result in (errno.EAGAIN, errno.EWOULDBLOCK):
            return port, "FILTERED (Resource temporarily unavailable / Rate-limited)"
        else:
            return port, f"UNKNOWN ERROR (Errno: {result})"

    except socket.timeout:
        return port, "FILTERED (Silent Drop / Timeout)"

    except Exception as e:
        return port, f"ERROR ({type(e).__name__})"

    finally:
        if sock is not None:
            sock.close()


# ====================================================


def port_scanner(target_ip, target_ports, show_closed, show_filtered, show_all):
    """
    Executes concurrent port scanning and returns a structured list of results
    based on the visibility flags, strictly avoiding direct STDOUT printing.
    """
    executor = ThreadPoolExecutor(max_workers=MAX_THREADS)
    valid_results = []  # Data structure to hold the filtered output

    try:
        futures = [executor.submit(get_port_state, target_ip, port)
                   for port in target_ports]

        for future in futures:
            port, status = future.result()

            display = False

            if "OPEN" in status:
                display = True
            elif show_all:
                display = True
            elif show_closed and "CLOSED" in status:
                display = True
            elif show_filtered and ("FILTERED" in status or "ERROR" in status):
                display = True

            if display:
                valid_results.append((port, status))

        return valid_results

    except KeyboardInterrupt:
        print("\n[!] Keyboard Interrupt received. Halting scan...")
        executor.shutdown(wait=False, cancel_futures=True)
        sys.exit(0)

# ====================================================

def parse_ports(port_string):
    """
    Parse port range by argument.
    """
    ports = set()

    try:
        tokens = port_string.split(',')

        for token in tokens:
            token = token.strip()

            if '-' in token:
                start, end = token.split('-')
                start_port = int(start)
                end_port = int(end)

                if start_port < 1 or end_port > 65535 or start_port > end_port:
                    raise ValueError(f"Invalid range bounds: {token}")

                ports.update(range(start_port, end_port + 1))

            else:
                port = int(token)
                if port < 1 or port > 65535:
                    raise ValueError(f"Invalid port number: {token}")
                ports.add(port)

    except ValueError as e:
        print(f"[!] Parsing Error: {e}", file=sys.stderr)
        sys.exit(1)

    return sorted(list(ports))

# ====================================================


def validate_target_ip(ip):
    """Validate target IPv4 address."""
    try:
        ipaddress.IPv4Address(ip)
    except ValueError:
        print(f"\nError: '{ip}' Invalid IPv4 address")
        sys.exit(1)


# ====================================================

def print_banner(input_str):
    """Print standard formatted banner"""
    if not input_str:
        print("\n" + "=" * BANNER_LENGTH)
    else:
        print("\n" + input_str.center(BANNER_LENGTH, "="))


# ====================================================

def parse_arguments():
    """Parse command line arguments"""
    parser = argparse.ArgumentParser(description="Advanced Network Port Scanner.")
    parser.add_argument(
        "-p", "--ports",
        type=str,
        default="1-65535",
        help="Ports to scan (e.g., 22,80,443 or 1-1000 or 22,80-100)"
    )
    # Output filtering flags
    parser.add_argument("--show-closed", action="store_true", help="Display CLOSED ports")
    parser.add_argument("--show-filtered", action="store_true", help="Display FILTERED and ERROR ports")
    parser.add_argument("--show-all", action="store_true", help="Display all port states unconditionally")
    parser.add_argument("target", type=str, help="Target IP address")

    return parser.parse_args()

# --- Main Execution ---

def main():
    """Get and parse input"""

    args = parse_arguments()

    validate_target_ip(args.target)

    target_ports = parse_ports(args.ports)

    print_banner(" INFO ")
    print(f"[*] Target IP: {args.target}")
    print(f"[*] Total ports to scan: {len(target_ports)}")

    print_banner(" Start Scanning ")
    print_banner(f" Target IP: {args.target} ")

    scan_result = port_scanner(
        args.target,
        target_ports,
        args.show_closed,
        args.show_filtered,
        args.show_all
    )

    if not scan_result:
        print(f"No matching ports found for {args.target} based on current filters.")
    else:
        for p, status in scan_result:
            formatted_string = f"\nPort {p: <5} : {status}"
            print(formatted_string)

if __name__ == '__main__':
    main()