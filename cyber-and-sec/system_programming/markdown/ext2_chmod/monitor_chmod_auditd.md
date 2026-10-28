# System vs. Custom Filesystem chmod Operations: An `auditd` Analysis

This guide details the process of using the Linux Audit Daemon (`auditd`) to
monitor and compare how the host kernel handles a native OS `chmod` command
versus a custom user-space `Chmod` tool operating on a virtual `ext2` disk
image.

By analyzing the underlying system calls, we can observe the fundamental
differences between kernel-level filesystem management and raw byte manipulation
in user space.

---

## Step 1: Install and Prepare `auditd`

Install `auditd`, create a dummy file for the OS test, and clear the existing
audit log to ensure clean, readable results.

```bash
# Install auditd (Ubuntu/Debian)
sudo apt install auditd

# Create a dummy file for the native chmod test
touch dummy_file.txt

# Truncate the audit log to remove background noise
sudo truncate -s 0 /var/log/audit/audit.log
```

## Step 2: Configure Audit Rules

Instruct the kernel to watch two specific files: the dummy text file (for
native `chmod`) and the virtual filesystem image (for the custom `Chmod`).

*Note: Replace `/absolute_path/` with your actual absolute path.*

```bash
# Watch the dummy file for attribute changes (permissions)
sudo auditctl -w /<absolute_path>/dummy_file.txt -p a -k os_chmod

# Watch the virtual disk image for write operations
sudo auditctl -w /<absolute_path>/<disk_name> -p w -k my_chmod
```

- `-p a` — triggers on attribute changes
- `-p w` — triggers on write/open-for-write operations
- `-k` — assigns a custom key (label) to the log entry for easy searching

## Step 3: Execute the Operations

Run the native Linux `chmod` on the dummy file:

```bash
chmod 777 dummy_file.txt
```

Then run the custom `Chmod` tool to change permissions of a file *inside* the
virtual disk image.

## Step 4: Log Analysis & Comparison

Use the `ausearch` tool with the `-i` (interpret) flag to translate raw
hexadecimal logs into human-readable text.

### 4.1 Analyzing the Native OS `chmod`

```bash
sudo ausearch -k os_chmod -i
```

**Output:**

```text
type=PROCTITLE msg=audit(07/15/2026 08:02:28.969:638) : proctitle=chmod 777 dummy_file.txt
type=PATH msg=audit(07/15/2026 08:02:28.969:638) : item=0 name=dummy_file.txt inode=1853139 dev=103:08 mode=file,664 ouid=user ogid=user ...
type=SYSCALL msg=audit(07/15/2026 08:02:28.969:638) : arch=x86_64 syscall=fchmodat success=yes exit=0 a0=AT_FDCWD a2=0777 ... comm=chmod exe=/usr/bin/chmod key=os_chmod
```

**Explanation:**

- **`syscall=fchmodat`:** The Linux kernel natively recognizes this as a
  permission change. The `chmod` binary explicitly asks the kernel to update
  the file's inode metadata on the host filesystem.
- **`a2=0777`:** The kernel directly registers the exact octal permission passed
  to the system call.

### 4.2 Analyzing the Custom `Chmod` Implementation

```bash
sudo ausearch -k my_chmod -i
```

**Output:**

```text
type=PROCTITLE msg=audit(07/15/2026 08:03:23.125:644) : proctitle=././system_programming/bin/debug/ext2.out ext2_sandbox.img /file
type=PATH msg=audit(07/15/2026 08:03:23.125:644) : item=0 name=ext2_sandbox.img inode=1843561 dev=103:08 mode=file,666 ouid=root ogid=root ...
type=SYSCALL msg=audit(07/15/2026 08:03:23.125:644) : arch=x86_64 syscall=openat success=yes exit=3 a0=AT_FDCWD a2=O_RDWR ... comm=ext2.out exe=.../ext2.out key=my_chmod
```

**Explanation:**

- **`syscall=openat` with `a2=O_RDWR`:** This is the core difference. The host
  kernel is entirely blind to the fact that a "chmod" operation is taking place.
  To Linux, your program is simply opening a large file (`ext2_sandbox.img`)
  with Read/Write permissions to alter raw bytes.
- The actual permission logic happens strictly in user-space C code, which
  writes the updated `i_mode` bits directly to the virtual block — bypassing
  the host kernel's filesystem drivers entirely.

## Step 5: Cleanup

Remove the tracking rules to prevent unnecessary system logging overhead:

```bash
sudo auditctl -D
```

*(Expected output: `No rules`)*

---

## Bonus: Tracking All `chmod` Operations Globally

Instead of watching a specific file, monitor permission changes across the
entire OS by tracking the system calls associated with `chmod`:

```bash
sudo auditctl -a always,exit -F arch=b64 -S chmod -S fchmod -S fchmodat -k global_chmod
```

**Rule breakdown:**

- **`-a always,exit`:** Appends the rule and triggers the audit log when the
  system call exits (completes).
- **`-F arch=b64`:** Filters for 64-bit architecture (syscall numbers differ
  between 32-bit and 64-bit).
- **`-S chmod -S fchmod -S fchmodat`:** Monitors all three variants (different
  tools, libraries, or OS versions may use different underlying syscalls).
- **`-k global_chmod`:** Assigns a custom label for easy querying.

**Searching the global logs:**

```bash
sudo ausearch -k global_chmod -i
```
