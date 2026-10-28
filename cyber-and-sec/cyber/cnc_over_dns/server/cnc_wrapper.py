#!/usr/bin/env python3

# *********************************************************
#
# Description: Supervisor that guarantees iptables cleanup even on SIGKILL.
# Usage: sudo python3 server/cnc_wrapper.py --port 53 -- \
#            python3 server/cnc_handler.py -i eth0 -d <CLIENT_IP>
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import signal
import subprocess
import sys

# ==============================================================================

# --- Functions ---

# ==============================================================================


def _iptables_delete(port: int) -> None:
    """Remove the INPUT DROP rule for *port* if it exists (idempotent)."""
    subprocess.run(
        ["iptables", "-D", "INPUT",
         "-p", "udp", "--dport", str(port), "-j", "DROP"],
        capture_output=True,
    )


# ==============================================================================


def _parse_args() -> tuple[int, list[str]]:
    """Parse wrapper CLI arguments; return (port, child_command)."""
    parser = argparse.ArgumentParser(
        description=(
            "Supervisor: cleans iptables rules when cnc_handler.py exits, "
            "including after SIGKILL of the child."
        )
    )
    parser.add_argument(
        "--port", type=int, required=True,
        help="UDP port claimed by cnc_handler.py (cleaned up on child exit)",
    )
    parser.add_argument(
        "cmd", nargs=argparse.REMAINDER,
        help="Command to run — prefix with -- to separate from wrapper flags",
    )
    args = parser.parse_args()
    cmd = args.cmd
    if cmd and cmd[0] == "--":
        cmd = cmd[1:]
    if not cmd:
        parser.error("No command supplied after --")
    return args.port, cmd


# ==============================================================================


def main() -> None:
    """Start child process; guarantee iptables cleanup on any exit."""
    if sys.platform != "linux":
        print(
            "[cnc_wrapper] iptables is Linux-only; running child directly.",
            file=sys.stderr,
        )

    port, cmd = _parse_args()
    child = subprocess.Popen(cmd)

    def _forward(signum: int, _frame) -> None:
        """Forward SIGTERM / SIGINT to the child so it can clean up first."""
        try:
            child.send_signal(signum)
        except ProcessLookupError:
            pass

    signal.signal(signal.SIGTERM, _forward)
    signal.signal(signal.SIGINT, _forward)

    exit_code = 0
    try:
        exit_code = child.wait()
    finally:
        # Runs on normal exit, SIGTERM-to-child, and child SIGKILL.
        # Does NOT run if THIS wrapper is SIGKILL'd — use ExecStopPost.
        if sys.platform == "linux":
            _iptables_delete(port)
            print(
                f"[cnc_wrapper] iptables cleanup done for port {port}.",
                file=sys.stderr,
            )

    sys.exit(exit_code)


# ==============================================================================

if __name__ == "__main__":
    main()
