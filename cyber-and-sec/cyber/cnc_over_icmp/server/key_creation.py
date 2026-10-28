#!/usr/bin/env python3

# *********************************************************
#
# Description: Generates an Ed25519 server identity keypair and writes the
#              private key to .env.server for use by cnc_handler.py.
# Usage: python3 key_creation.py
# Author: Moshe Yifrah
#
# *********************************************************

# --- Modules ---

import os
from pathlib import Path

from cryptography.hazmat.primitives.asymmetric import ed25519

# ==============================================================================

# --- Functions ---

def generate_server_keys():
    """Generate an Ed25519 keypair and write the private key to .env.server."""
    server_private_key = ed25519.Ed25519PrivateKey.generate()
    server_public_key = server_private_key.public_key()

    priv_hex = server_private_key.private_bytes_raw().hex()
    pub_hex = server_public_key.public_bytes_raw().hex()

    env_content = f"SERVER_PRIVATE_HEX={priv_hex}\n"
    env_path = Path(__file__).parent / ".env.server"

    with open(env_path, "w") as f:
        f.write(env_content)

    os.chmod(env_path, 0o600)

    print("Server keys generated successfully.")
    print(f"Written to: {env_path}")
    print(
        "Server Public Key "
        "(hardcode in agent_session.py SERVER_IDENTITY_PUB_HEX):"
    )
    print(pub_hex)

# ==============================================================================

if __name__ == "__main__":
    generate_server_keys()
