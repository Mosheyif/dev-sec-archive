
GPG Cryptography Guide

This guide provides a comprehensive walkthrough for performing fundamental cryptographic operations using GnuPG (GPG).

Part 1: Theoretical Background (Questions a & b)

a. What is GPG?

GPG (GNU Privacy Guard) is a free, open-source implementation of the OpenPGP standard (RFC 4880). It provides a robust suite of cryptographic software tools used for:

Encryption: Ensuring that only the intended recipient can read a message.

Digital Signatures: Verifying the authenticity of a message and ensuring it has not been tampered with.

Key Management: Creating, distributing, and revoking public/private key pairs.

b. Difference between Encryption and Signing

Encryption: The goal is confidentiality. You encrypt a message using the recipient's public key. This ensures that only the holder of the corresponding private key can decrypt and read the message.

Signing: The goal is integrity and authenticity. You sign a message using your private key. Anyone with your public key can verify that the message originated from you and has not been altered since it was signed.

Part 2: Hands-on Lab Operations (Questions c & d)

c. Generate a GPG Key

To generate a key pair, run the following command:

gpg --full-generate-key


Steps: Choose RSA/ECC, set key size, set expiration, and provide your User ID details.

Troubleshooting: If the system hangs, it is waiting for entropy (random data). Move your mouse, open other applications, or run find / in another terminal to generate disk I/O and randomness.

d. Performing File Operations

Create a base file for testing: echo "Sensitive Data" > secret.txt

1. Encrypt the file

gpg -e -a -r "Recipient Name" -o encrypted.asc secret.txt


-e (encrypt): Performs encryption.

-a (armor): Creates ASCII-armored output (readable text rather than binary).

-r (recipient): Specifies the key to encrypt for.

2. Sign with compression and verify

gpg --sign -o signed_comp.gpg secret.txt
gpg --verify signed_comp.gpg


--sign: Creates a "clearsigned" or binary signature that includes the original data.

3. Sign without compression and verify

gpg --sign --compress-algo none -o signed_nocomp.gpg secret.txt
gpg --verify signed_nocomp.gpg


--compress-algo none: Prevents GPG from compressing the data before signing.

4. Detached signature and verify

gpg -b -o signature.sig secret.txt
gpg --verify signature.sig secret.txt


-b (detach-sign): Creates a signature file separate from the source. The original file remains untouched.

Part 3: Comparison and Use Cases (Question d, last point)

Encryption: Sending confidential documents or emails where privacy is paramount.

Combined Sign (w/ Comp): Default behavior. Use when you need to send a self-contained file with authentication that is also optimized for size.

Sign (No Comp): Use when exact bit-level integrity is required or when the file type (like already compressed images) makes further compression useless.

Detached Signature: Use for software distribution or large archives. You distribute the original file (e.g., an ISO) and a separate .sig file. This allows users to download the original file without needing GPG to access it.

Part 4: Troubleshooting Permissions

GPG expects strict file permissions on its configuration directory. If you encounter errors, use:

# Set directory to 700 (owner access only)
chmod 700 ~/.gnupg

# Ensure files inside are readable only by owner
chmod 600 ~/.gnupg/*

# Correct ownership if files were created by root
sudo chown -R <user>:<user> ~/.gnupg

Note: Always ensure your private keys remain private; never share the ~/.gnupg folder content.
