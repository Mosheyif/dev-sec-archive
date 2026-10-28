

# HTTPS & PKI Implementation Guide: Secure Web Server & CA Architecture

**Overview:** 
This exercise focuses on the practical and theoretical aspects of securing web communication. It guides you through establishing a local Public Key Infrastructure (PKI) by creating a custom Certificate Authority (CA), generating and signing digital certificates, and deploying a secure HTTPS server in C. Additionally, it explores core cryptographic concepts such as Trust Stores, private key protection, and Hardware Security Modules (HSM).

---

## Part 1: Theoretical Questions

**1. Why is HTTPS important?**
HTTPS secures communication over computer networks by providing three critical layers of protection (the CIA triad):

* **Confidentiality:** Encrypts the data payload (via TLS), preventing Man-in-the-Middle (MitM) attackers from reading sensitive information.
* **Integrity:** Uses Message Authentication Codes (MACs) to ensure data cannot be modified or corrupted in transit without detection.
* **Authentication:** Utilizes digital certificates to verify the server's identity, ensuring the client is communicating with the intended destination and not an impostor.

**2. What happens when the CA is not approved by the client?**
When the client (e.g., a web browser or `curl`) encounters a certificate signed by a Certificate Authority that does not exist in its local Trust Store, the TLS handshake fails. The connection is aborted by default because the client cannot establish a Root of Trust. 
*   **Browser-Specific Behavior:** Different browsers handle this validation breakdown with varying UX patterns. For example, Firefox uses its own network stack and displays specific error codes like `SEC_ERROR_UNKNOWN_ISSUER` with an explicit warning page. Chromium-based browsers (like Chrome or Edge) rely on the operating system's trust store (or their own built-in stores) and present an `ERR_CERT_AUTHORITY_INVALID` screen.
*   **User Bypass & Exception Handling:** Browsers generally provide a manual override mechanism. In Firefox, clicking "Advanced" often reveals an "Accept the Risk and Continue" button. In Chrome, typing `thisisunsafe` or clicking "Advanced" -> "Proceed to localhost (unsafe)" allows the user to explicitly force a temporary security exception, trusting the self-signed certificate for the current session.

**3. How do you protect a key with a password? Why would you do this?**
You protect a private key by encrypting it with a symmetric cipher (e.g., AES-256) before writing it to the disk.
This is strictly necessary for the Root CA private key. Since the CA key is the ultimate anchor of trust in your PKI, encrypting it ensures that even if the physical file is compromised or stolen, the attacker cannot sign fraudulent certificates without knowing the passphrase.

**4. What is a HSM? What are its benefits?**
A Hardware Security Module (HSM) is a dedicated, physical computing device designed to safeguard and manage digital keys and perform cryptographic operations.

* **Benefits:** It is tamper-resistant (physically destroys keys if opened), ensures private keys never leave the hardware boundary (cannot be copied via OS vulnerabilities), and offloads heavy cryptographic processing from the main server CPU.

---

## Part 2: Practical Exercises

### Prerequisites

Ensure the necessary cryptographic and testing tools are installed on your Linux environment:

```bash
sudo apt-get update
sudo apt-get install openssl curl build-essential libssl-dev

```

### Exercise A: Planning and Creating a Secure Website (HTTPS)

**Step 1: Generate the Certificate Authority (CA)**
To act as our own issuer, we must create a CA private key and a Root Certificate.

```bash
# Generate CA Private Key (AES-256 encrypted)
openssl genrsa -aes256 -out ca.key 4096

# Generate CA Public Certificate
openssl req -x509 -new -nodes -key ca.key -sha256 -days 3650 -out ca.crt

```

**Step 2: Generate the Server Key and CSR**
Create the server's private key (unencrypted, to allow daemonized startup) and a Certificate Signing Request (CSR).

```bash
# Generate Server Key
openssl genrsa -out server.key 2048

# Generate CSR
openssl req -new -key server.key -out server.csr

```

**Step 3: Define Subject Alternative Name (SAN)**
*Problem:* Modern browsers reject certificates that only use the Common Name (CN).
*Solution:* Create an extension configuration file named `ext.cnf` to define SANs.

```ini
authorityKeyIdentifier=keyid,issuer
basicConstraints=CA:FALSE
keyUsage = digitalSignature, nonRepudiation, keyEncipherment, dataEncipherment
subjectAltName = @alt_names

[alt_names]
DNS.1 = localhost
IP.1 = 127.0.0.1

```

**Step 4: Sign the Server Certificate**
Use the CA to sign the CSR and output the final `server.crt`.

```bash
openssl x509 -req -in server.csr -CA ca.crt -CAkey ca.key -CAcreateserial -out server.crt -days 825 -sha256 -extfile ext.cnf

```

**Step 5: Organize Files in the Directory Structure**
Organize the generated PKI assets into a clean, dedicated directory tree within the project:

```bash
mkdir -p certs/ca certs/server

mv ca.* ext.cnf certs/ca/
mv server.crt server.key certs/server/
rm server.csr  # No longer needed

```

**Step 6: Understanding and Executing the Hybrid Server**

To transform a standard plaintext HTTP implementation into a secure HTTPS service without code duplication, we designed a modular C architecture utilizing conditional compilation (`#ifdef USE_SSL`). This approach allows a single source codebase (`https_server.c`) to operate in dual-mode, cleanly separating core networking logic from cryptographic operations.

*   **Module Breakdown:**
    *   `https_server.c`: Manages socket creation, binding, listening, and the primary request-handling loop. It adapts its network I/O behavior dynamically based on the build configuration.
    *   `ssl_handler.c` / `ssl_handler.h`: Encapsulates all OpenSSL lifecycle routines, including context initialization (`SSL_CTX`), certificate verification, TLS handshake execution, and secure wrapper functions for reading and writing encrypted streams.
*   **Execution Flow:** Upon startup, the server initializes a TCP socket and listens for incoming connections. When compiled in secure mode, it loads the server certificate and private key into memory. Once a client connects, the server performs the TLS Handshake to negotiate cryptographic parameters and establish a secure tunnel before processing application-layer data.

**Running the Secure Server:**
Because the HTTPS server requires identity verification assets at runtime, you must execute the binary while providing the relative paths to the server certificate and private key as command-line arguments:
```bash
./bin/https_server certs/server/server.crt certs/server/server.key

```

**Project Directory Hierarchy:**

```text
https_server/
├── bin/
│   ├── http_server
│   └── https_server
├── certs/
│   ├── ca/
│   │   ├── ca.crt
│   │   ├── ca.key
│   │   ├── ca.srl
│   │   └── ext.cnf
│   └── server/
│       ├── server.crt
│       └── server.key
├── include/
│   └── ssl_handler.h
├── Makefile
└── src/
    ├── https_server.c
    └── ssl_handler.c

```

```

**Step 7: Client Verification**

* **CLI Test (Failure case):** Attempting to connect without providing the CA will fail.
```bash
curl -v https://localhost:8000
# Output includes: SSL certificate problem: unable to get local issuer certificate

```


* **CLI Test (Success case):** Inject the CA into curl's trust store.
```bash
curl -v --cacert certs/ca/ca.crt https://localhost:8000
# Output includes: SSL connection using TLSv1.3... Hello World

```


* **Browser Validation:** Open Firefox/Chrome settings, search for "Certificates" -> "Authorities", and import `ca.crt`. Navigate to `https://localhost:8000`. The browser will now show a secure padlock icon.

### Exercise B: Protecting a Key with a Password

If you generated a key without a password (or need to change the cipher), you can encrypt it retroactively.

**Step 1: Apply AES-256 Encryption**

```bash
openssl rsa -aes256 -in certs/ca/ca.key -out certs/ca/ca_secure.key
mv certs/ca/ca_secure.key certs/ca/ca.key

```

*Architectural Note on Server Keys:*
*Problem:* If you run the above command on `server.key`, the C HTTP server will hang during execution, waiting for standard input (stdin) to decrypt the key.
*Solution:* Do not encrypt `server.key`. Instead, protect it using strict OS-level permissions:

```bash
chmod 400 certs/server/server.key

```
