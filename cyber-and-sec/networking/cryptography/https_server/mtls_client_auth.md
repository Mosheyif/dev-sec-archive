

# mTLS & Selective Client Authentication Guide

**Overview:**
This exercise focuses on implementing Mutual TLS (mTLS) to enforce client authentication for specific server routes. It demonstrates how to configure an OpenSSL context to request, but not strictly require, a client certificate, allowing the application layer to implement dynamic, path-based access control. It also covers extracting and validating the Subject Name from a presented X.509 certificate to restrict access based on specific identity attributes.

---

## Part 1: Theoretical Background

**1. What is Mutual TLS (mTLS)?**
Standard TLS only authenticates the server to the client. mTLS requires both parties to authenticate each other using digital certificates. The server cryptographically verifies that the client holds a valid private key associated with a certificate signed by a trusted Certificate Authority (CA).

**2. How can a site be partially protected by mTLS?**
At the transport/TLS layer, the server is configured to *request* a client certificate using the `SSL_VERIFY_PEER` flag, but it does not instantly fail the handshake if the client omits it. The TLS connection is established regardless. Enforcement is deferred to the application layer (HTTP logic), which checks the requested URI and verifies if a valid certificate was provided in the SSL context before serving protected routes.

**3. What is Subject Name validation?**
An X.509 certificate contains a Subject field detailing the entity it was issued to (e.g., Common Name, Organization). Application logic can parse this field to enforce granular access control, ensuring that not only is the client trusted by the CA, but they also possess specific attributes (like a specific domain string) required for access.

---

## Part 2: Practical Implementation

### Step 1: Generate Client Certificates
Generate two distinct client certificates signed by the Root CA created in the previous exercise. Client A represents a standard user, and Client B represents a privileged user with a specific string in its Common Name (CN).

```bash
# Client A: Standard Certificate
openssl genrsa -out certs/clientA.key 2048
openssl req -new -key certs/clientA.key -out certs/clientA.csr -subj "/C=IL/O=Infinity Labs/CN=Standard Client"
openssl x509 -req -in certs/clientA.csr -CA certs/ca/ca.crt -CAkey certs/ca/ca.key -CAcreateserial -out certs/clientA.crt -days 365 -sha256

# Client B: Restricted Certificate (Contains specific string 'admin-infinity')
openssl genrsa -out certs/clientB.key 2048
openssl req -new -key certs/clientB.key -out certs/clientB.csr -subj "/C=IL/O=Infinity Labs/CN=admin-infinity"
openssl x509 -req -in certs/clientB.csr -CA certs/ca/ca.crt -CAkey certs/ca/ca.key -CAcreateserial -out certs/clientB.crt -days 365 -sha256

# Cleanup intermediate CSR files
rm certs/*.csr

```

### Step 2: Configure OpenSSL Context for Optional mTLS
*(Reference `src/ssl_handler.c`)*
The OpenSSL context initialization was expanded to include the CA trust store, establishing a Root of Trust for incoming clients. To maintain the hybrid nature of the server, the context is configured to request client certificates (`SSL_VERIFY_PEER`) but explicitly avoids strictly enforcing them at the transport layer. This architectural decision ensures the TLS handshake completes successfully even for anonymous clients, preserving access to public web pages.

### Step 3: Implement Certificate Validation Logic
*(Reference `src/ssl_handler.c` and `include/ssl_handler.h`)*
A new abstraction layer was introduced via a validation wrapper function. This module dynamically extracts the X.509 peer certificate from the active SSL session. Depending on the route's security policy, it either validates the mere existence of a CA-signed certificate (satisfying requirement *a*) or actively parses the certificate's Subject Name to verify the presence of a specific substring constraint (satisfying requirement *b*). Dynamic memory allocated during the X.509 retrieval is strictly freed within the function to prevent memory leaks during long-running server processes.

### Step 4: Enforce Application-Layer Routing
*(Reference `src/https_server.c`)*
The core request-handling loop was updated to incorporate a lightweight URI parser, acting as an application-level firewall layered on top of the existing HTTP implementation. Upon receiving a GET request, the server inspects the requested path:
*   **Public Routes:** Bypass client validation entirely.
*   **Protected Routes (`/secure-a`, `/secure-b`):** Trigger the validation wrapper from Step 3. If the client fails the cryptographic check or lacks the required Subject Name string, the server intercepts the request and proactively returns an HTTP `403 Forbidden` response, dropping the connection before any restricted content is processed.

```

### Step 5: Test Access Controls via cURL

Recompile the server using the existing `make https` target, run the binary, and test the routes using different client profiles.

```bash
# 1. Access public page (Should Succeed)
curl -k https://localhost:8000/

# 2. Access /secure-a without a certificate (Should Fail - 403 Forbidden)
curl -k https://localhost:8000/secure-a

# 3. Access /secure-a with Client A (Should Succeed)
curl --cert certs/clientA.crt --key certs/clientA.key --cacert certs/ca/ca.crt https://localhost:8000/secure-a

# 4. Access /secure-b with Client A (Should Fail - String not in Subject)
curl --cert certs/clientA.crt --key certs/clientA.key --cacert certs/ca/ca.crt https://localhost:8000/secure-b

# 5. Access /secure-b with Client B (Should Succeed - Contains 'admin-infinity')
curl --cert certs/clientB.crt --key certs/clientB.key --cacert certs/ca/ca.crt https://localhost:8000/secure-b

```
