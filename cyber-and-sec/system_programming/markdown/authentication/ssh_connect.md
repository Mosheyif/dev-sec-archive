Guide: Connecting to a Remote Computer via SSH

What is SSH?

SSH (Secure Shell) is a cryptographic network protocol used to securely connect to a remote computer over an unsecured network. It allows you to access a remote machine's command line, manage files, and execute commands exactly as if you were sitting physically in front of that machine.

The Basic Command

To establish a standard SSH connection, you only need to open your terminal and run one short command. The syntax is simply ssh followed by the remote username and the remote machine's address:

``` bash
ssh username@remote_ip_address
```

(Note: If the server is listening on a non-standard port instead of the default port 22, you can specify it using the -p flag: ssh -p 2222 admin@192.168.1.50)

Authentication Methods

Once you press Enter, the remote server needs to verify your identity before granting access. This typically happens in one of two ways:

1. Password Authentication

This is the most common default method.

How it works: The terminal will prompt you to type the password for the username you specified.

Important: As a security feature in Linux/Unix terminals, when you type your password, nothing will appear on the screen (no stars or dots). Just type it blindly and press Enter.

2. Public-Key Authentication

This is the industry standard for security and is highly recommended over passwords.

How it works: Instead of typing a password, authentication relies on a cryptographic key pair (a Private Key on your local machine and a Public Key on the remote server).

The Process: When you attempt to connect, the server issues a mathematical challenge that can only be solved if your computer possesses the matching Private Key. If the math checks out, you are logged in instantly without typing a password.
