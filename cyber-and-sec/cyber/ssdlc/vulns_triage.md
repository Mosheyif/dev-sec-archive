

# Exercise 2: Vulnerability Triage and Remediation Plan

## Prioritized Vulnerability Ranking

**1. SQL injection in the public login endpoint (CVSS 9.1)**
*   **Reasoning:** This is an unauthenticated, internet-facing vulnerability. It
    provides an attacker with immediate, direct access to the database, allowing
    for total authentication bypass and complete data exfiltration. This is a
    "game over" scenario and the absolute highest priority.

**2. Broken access control: user can view others' invoices (IDOR) (CVSS 8.2)**
*   **Reasoning:** Although this requires an attacker to be authenticated, it
    directly exposes sensitive customer financial data. In production, this
    guarantees a data breach and severe compliance violations (e.g., GDPR,
    PCI-DSS). The blast radius is massive because any valid user can scrape
    the entire customer base's invoices.

**3. Outdated library with critical CVE, vulnerable function not called
(CVSS 9.8)**
*   **Reasoning:** This carries the highest CVSS score but is prioritized third
    because the vulnerable execution path is not currently active and it resides
    in an internal service. However, due to dynamic linking and the evolving
    nature of codebases, a developer could inadvertently call this function
    tomorrow, or an attacker could use Return-Oriented Programming (ROP) to
    reach it. It is a ticking time bomb that must be patched before it becomes
    exploitable.

**4. Weak password policy (min 6 chars) (CVSS 5.9)**
*   **Reasoning:** This flaw is internet-facing and actively facilitates
    brute-force and credential stuffing attacks. While it requires more effort
    to exploit than a direct SQL injection, it directly compromises user
    accounts and must be addressed to protect the external attack surface.

**5. Verbose error messages reveal stack traces (CVSS 5.3)**
*   **Reasoning:** This is an Information Disclosure vulnerability. While it
    does not allow direct system compromise, leaking stack traces provides
    attackers with a precise blueprint of the backend architecture, framework
    versions, and database schemas, significantly lowering the barrier for
    discovering subsequent exploits.

**6. Missing HTTP security header (X-Frame-Options) (CVSS 4.3)**
*   **Reasoning:** The absence of this header allows the application to be
    embedded in external `iframes`, enabling Clickjacking attacks. It is ranked
    last because successful exploitation requires active social engineering
    (tricking a user into clicking a disguised link) and has the lowest direct
    technical impact.

---

## Developer Remediation Summaries (Top 2 Priorities)

### Priority 1: Public SQL Injection (Login Endpoint)
**Context:** Our external security scan identified a critical SQL Injection flaw
in the public-facing login API. Because the endpoint is unauthenticated, any
internet user can send maliciously crafted SQL payloads to bypass the password
check entirely or dump the contents of the database tables.

**Action Required:** Immediately refactor the authentication module's database
queries. You must replace all dynamic string concatenations with **Parameterized
Queries (Prepared Statements)** or utilize the framework's native ORM safely.
Parameterization ensures that user input is strictly treated as literal data by
the database engine, never as executable SQL commands, neutralizing the
injection vector entirely.

### Priority 2: Broken Access Control / IDOR (Invoice Endpoint)
**Context:** The endpoint responsible for fetching user invoices
(`/api/invoices/{id}`) suffers from an Insecure Direct Object Reference (IDOR).
Currently, an authenticated user can simply increment the `id` parameter in
their HTTP request and successfully view the private invoices of other
customers. This constitutes an active data breach.

**Action Required:** Implement strict **Role-Based Access Control (RBAC)** at
the controller level. Fetching the invoice ID from the request is not enough;
the backend must independently extract the `user_id` from the verified session
context (e.g., the cryptographically signed JWT) and execute a validation check
against the database to confirm that the requesting user mathematically owns
the target invoice before returning the data.
