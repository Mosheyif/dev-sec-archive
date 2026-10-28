

# Exercise 3: Secure Code Review (Login Handler)

**1. SQL Injection (SQLi)**
*   **Vulnerability:** The database query is constructed using raw string 
    concatenation with the `username` and `password` inputs. An attacker 
    can input a payload like `admin' --` to prematurely terminate the 
    string, comment out the password check, and bypass authentication 
    entirely.
*   **Fix:** Replace all string concatenation with **Parameterized Queries** 
    (Prepared Statements) to ensure user input is treated strictly as data, 
    not as executable SQL commands.

**2. Insecure Client-Side Trust (Privilege Escalation)**
*   **Vulnerability:** The application sets the user's authorization role 
    (`user.role`) in a plain client-side cookie. Because the client has 
    full control over their browser, an attacker can simply modify this 
    cookie from "customer" to "admin" to escalate their privileges.
*   **Fix:** Never store authorization state on the client. Manage user 
    sessions on the backend (Server-side Sessions) or issue a 
    cryptographically signed token (like a **JWT**) that the client cannot 
    tamper with.

**3. Reflected Cross-Site Scripting (XSS)**
*   **Vulnerability:** If the user is not found, the raw `username` input 
    is concatenated directly into the returned error message. If an 
    attacker crafts a malicious link containing a JavaScript payload in the 
    username parameter, it will execute in the victim's browser.
*   **Fix:** Implement strict **Output Encoding** (e.g., HTML Entity 
    Encoding) on all user-supplied data before reflecting it back in the 
    HTTP response to neutralize executable scripts.
