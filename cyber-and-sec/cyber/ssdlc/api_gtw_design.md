

# Exercise 4: API Gateway vs. Backend Security Controls

## 1. Security Controls Configured at the API Gateway
The gateway acts as the first line of defense (perimeter security) handling 
stateless, high-volume checks before traffic reaches the internal network:
*   **Authentication Verification:** Validating the structural integrity, 
    cryptographic signature, and expiration of the JWT (JSON Web Token).
*   **Rate Limiting & Throttling:** Restricting the number of `POST /api/transfer` 
    requests per IP or User ID to prevent DDoS and credential stuffing.
*   **TLS Termination:** Ensuring all data in transit is encrypted using 
    TLS 1.3 before decrypting it for internal routing.
*   **Basic Input Sanitization (WAF):** Blocking generic malicious payloads 
    (e.g., obvious SQLi strings or extremely oversized request bodies) at 
    the edge.

## 2. Checks Required in the Backend Service (And Why)
The backend must enforce stateful, business-specific logic that the gateway 
cannot evaluate because the gateway lacks access to the database:
*   **Authorization (IDOR/BOLA Protection):** The backend must verify that the 
    `user_id` extracted from the token is the actual mathematical owner of the 
    `source_account` provided in the JSON payload. (The gateway doesn't know 
    who owns which account).
*   **Business Logic Validation:** Checking if the account has sufficient funds 
    and verifying that the target account actually exists. (The gateway cannot 
    query the database).
*   **Parameterized Queries:** Safely binding the validated input to the SQL 
    execution plan to prevent SQL Injection during the final data transition.
*   **Audit Logging:** Recording the immutable transaction details with the 
    exact user context for non-repudiation.

## 3. OWASP API Security Top 10 Risk Guarded Against
**API1:2023 - Broken Object Level Authorization (BOLA)** 
*(Formerly known as IDOR - Insecure Direct Object Reference).* 
This design explicitly guards against BOLA by ensuring the backend does not 
blindly trust the `source_account` sent in the request, but rather validates 
it against the authenticated user's session state.
