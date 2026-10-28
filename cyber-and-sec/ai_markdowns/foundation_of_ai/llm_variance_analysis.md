# Analysis of LLM Variance in SSH Troubleshooting Responses

I ran the following prompt in the same model 5 times in 5 separate fresh chats:

**"List 5 likely root causes for a Linux host suddenly refusing all SSH
connections"**

## 1. What Stayed Stable Across Runs? What Changed?

- **The name of the chat remained the same across all of them**

- **Stable Core Causes Across All Runs:**
  - The SSH daemon (`sshd`) can be down or crashed due to the OOM killer or
    manual stoppage.
  - Firewall or network security group misconfigurations can block traffic.
  - Automated IP banning and intrusion prevention tools like Fail2Ban are
    frequently cited.
  - Resource exhaustion such as memory limits or `MaxStartups` connection caps
    appear consistently.
  - Configuration syntax errors or port modifications in `sshd_config` are
    common themes.

- **Changes and Unique Edge Cases Across Specific Runs:**
  - Network routing, interface failures, and DHCP lease expirations appear
    exclusively in Run 1.
  - Socket backlog queue exhaustion is unique to Run 2.
  - Duplicate IP address conflicts and ARP caching issues are exclusive to Run 4.
  - Root filesystem or `/var` partition being 100% full is unique to Run 5.
  - Strict file permission errors on directories like `/var/empty/sshd` or
    `~/.ssh` appear only in Run 5.

## 2. Plausible-Sounding But Incorrect or Unverifiable Elements

- **Deprecated or Obsolete Security Layers:**
  - Runs 1, 2, and 3 reference TCP Wrappers and `/etc/hosts.deny`, which have
    been deprecated and removed from modern OpenSSH and Linux distributions.

- **Confusion Over Network Behavior (Drop vs. Refused):**
  - Runs 1, 3, and 5 list cloud security group changes under connection
    refusal, whereas cloud firewalls typically drop packets silently (resulting
    in a timeout rather than a TCP RST "Connection refused"), a distinction
    correctly highlighted in Run 4.

- **Conflating Connection Refusal with Authentication Errors:**
  - Run 5 lists strict file permissions on user `~/.ssh` directories as a
    cause for connection refusal, whereas permission errors on SSH keys or
    directories prevent successful authentication *after* the TCP connection is
    successfully established, returning a "Permission denied" error instead.

## 3. Implications for Real Investigations

- **Incomplete Coverage from a Single Prompt:**
  - Relying on a single LLM output would miss unique failure modes such as
    duplicate IPs found in Run 4 or disk space exhaustion found in Run 5.

- **Probabilistic Hypotheses vs. Empirical Facts:**
  - LLMs generate statistically likely token sequences rather than conducting
    live diagnostics on a target machine, meaning outputs can mix valid
    troubleshooting steps with outdated or technically imprecise claims.

- **The Necessity of Validation:**
  - In a real system administration investigation, an LLM output should be used
    merely as a brainstorming checklist to guide systematic, empirical
    validation using system logs and network tools.
