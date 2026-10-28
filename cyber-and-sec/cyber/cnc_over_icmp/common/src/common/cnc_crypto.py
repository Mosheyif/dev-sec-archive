#!/usr/bin/env python3

# *********************************************************
#
# Description: Ephemeral key exchange (X25519), Ed25519 authentication,
#              HKDF session key derivation, and RAM teardown.
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

from __future__ import annotations

from typing import Tuple
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ed25519, x25519
from cryptography.hazmat.primitives.kdf.hkdf import HKDF

# --- Constants & State ---

_handshake_psk: bytearray = bytearray(32)   # set by init_handshake_psk at startup
_active_session_psk: bytearray = bytearray(32)

# ==============================================================================

# --- Functions ---

def init_handshake_psk(identity_pub_bytes: bytes) -> None:
    """Derive and store the bootstrap PSK from the server's Ed25519 public key.

    Must be called once at startup on both endpoints before any handshake.
    The client passes SERVER_IDENTITY_PUB_HEX; the server passes the public
    key it derives from its loaded private key.  Both sides produce the same
    32-byte PSK via HKDF-SHA256, so the value is never transmitted or stored.
    """
    derived = HKDF(
        algorithm=hashes.SHA256(),
        length=32,
        salt=None,
        info=b"cnc-handshake-bootstrap-psk-v1",
    ).derive(identity_pub_bytes)
    _handshake_psk[:] = derived
    _active_session_psk[:] = derived


def get_handshake_psk() -> bytes:
    """Return the derived bootstrap PSK (used for HS_INIT / HS_RESP frames)."""
    return bytes(_handshake_psk)


def get_session_psk() -> bytes:
    """Return the active symmetric session key stored in RAM."""
    return bytes(_active_session_psk)

# ==============================================================================

def generate_ephemeral_keypair() -> Tuple[x25519.X25519PrivateKey, bytes]:
    """Generate an in-memory X25519 ephemeral keypair for a single session."""
    priv_key = x25519.X25519PrivateKey.generate()
    pub_bytes = priv_key.public_key().public_bytes_raw()
    return priv_key, pub_bytes

# ==============================================================================

def sign_ephemeral_key(identity_priv_hex: str, ephemeral_pub: bytes) -> bytes:
    """Sign an ephemeral public key using the server's Ed25519 private key."""
    priv_bytes = bytes.fromhex(identity_priv_hex.strip())
    identity_priv = ed25519.Ed25519PrivateKey.from_private_bytes(priv_bytes)
    return identity_priv.sign(ephemeral_pub)

# ==============================================================================

def verify_ephemeral_key(
    identity_pub_hex: str, ephemeral_pub: bytes, signature: bytes
) -> bool:
    """Verify the server Ed25519 signature over its ephemeral public key."""
    try:
        pub_bytes = bytes.fromhex(identity_pub_hex.strip())
        identity_pub = ed25519.Ed25519PublicKey.from_public_bytes(pub_bytes)
        identity_pub.verify(signature, ephemeral_pub)
        return True
    except (ValueError, InvalidSignature, TypeError):
        return False

# ==============================================================================

def derive_and_set_session_key(
    my_ephemeral_priv: x25519.X25519PrivateKey, peer_ephemeral_pub: bytes
) -> None:
    """Compute X25519 shared secret and derive the session key via HKDF."""
    global _active_session_psk
    peer_pub = x25519.X25519PublicKey.from_public_bytes(peer_ephemeral_pub)
    shared_secret = my_ephemeral_priv.exchange(peer_pub)

    derived_key = HKDF(
        algorithm=hashes.SHA256(),
        length=32,
        salt=None,
        info=b"cnc-session-layer-v1",
    ).derive(shared_secret)

    _active_session_psk[:] = derived_key
    del shared_secret

# ==============================================================================

def destroy_session_crypto() -> None:
    """Zeroize the session key in RAM and restore the handshake PSK."""
    for index in range(len(_active_session_psk)):
        _active_session_psk[index] = 0
    _active_session_psk[:] = _handshake_psk
