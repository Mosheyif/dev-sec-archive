
# WAF Products: Capabilities and Comparison

**Overview:**
A Web Application Firewall (WAF) inspects HTTP/HTTPS traffic and blocks
malicious requests before they reach the application. This document covers
two widely used WAF products — ModSecurity and AWS WAF — their capabilities,
and key differences.

---

## 1. ModSecurity

**Type:** Open-source, self-hosted WAF module.

**Deployment:** Runs as a module embedded inside a web server (Apache, Nginx,
or IIS). Traffic is inspected inline by the web server process before reaching
the application.

**Capabilities:**

* **Rule-Based Inspection:** Detection logic is defined in `.conf` rule files.
  The industry-standard ruleset is the OWASP Core Rule Set (CRS), which covers
  the OWASP Top 10 attack classes out of the box.
* **Attack Coverage:** SQL Injection (SQLi), Cross-Site Scripting (XSS), Local
  and Remote File Inclusion (LFI/RFI), Remote Code Execution (RCE), and HTTP
  protocol anomalies.
* **Operating Modes:**
  * *Detection Mode* — logs matched requests without blocking (safe for initial
    tuning).
  * *Prevention Mode* — actively blocks or returns a 403 on rule matches.
* **Full Customization:** Rules can be written, modified, or suppressed at any
  granularity — per path, per parameter, or per client IP.
* **Audit Logging:** Detailed transaction logs capture the full request and the
  specific rules that triggered, enabling forensic analysis.

---

## 2. AWS WAF

**Type:** Cloud-native, managed WAF service.

**Deployment:** Attaches directly to AWS resources — CloudFront distributions,
Application Load Balancers (ALB), API Gateways, and AppSync APIs. No agent or
module installation required.

**Capabilities:**

* **Web ACLs and Rule Groups:** Traffic is filtered by Web Access Control Lists
  (ACLs) composed of managed or custom rule groups evaluated in priority order.
* **Managed Rule Groups:** AWS and third-party vendors (e.g., CrowdStrike,
  Imperva) publish pre-built rule groups that are automatically updated — no
  manual ruleset maintenance.
* **Attack Coverage:** SQLi, XSS, known bad IP reputation lists, rate-based
  rules (brute force / DDoS mitigation), and bot control.
* **Rate Limiting:** Rules can count requests from a single IP over a time
  window and automatically block when a threshold is exceeded.
* **Geo-Blocking:** Requests can be blocked or allowed based on the originating
  country.
* **Logging and Metrics:** Full request logs delivered to S3, CloudWatch Logs,
  or Kinesis Firehose for SIEM integration.

---

## 3. Comparison

| Property | ModSecurity | AWS WAF |
|---|---|---|
| **Deployment** | Self-hosted (web server module) | Cloud-managed (AWS service) |
| **Customization** | Full — write any rule in any condition | Moderate — custom rules within AWS schema |
| **Ruleset Maintenance** | Manual — operator updates CRS | Automatic — AWS-managed rule groups |
| **Cost** | Free (operational overhead applies) | Pay-per-use (per ACL, rule, and request volume) |
| **Integration** | Apache, Nginx, IIS | CloudFront, ALB, API Gateway, AppSync |
| **Scalability** | Scales with the web server | Scales automatically with AWS infrastructure |
| **Visibility** | Local audit log files | CloudWatch, S3, Kinesis Firehose |

**Key Takeaway:** ModSecurity gives operators maximum control and is ideal for
on-premise or custom server environments. AWS WAF trades customization depth
for ease of deployment and zero maintenance, making it the natural choice for
cloud-native architectures already running on AWS.
