Reviewer: Alon

# System Activity & Networking Scripts Guide

This guide details the creation of a global script designed to list all active listening sockets and the specific programs holding them, highlighting the proper use of permissions and sudoers configuration.

---

## The Exercise: `activesockets` Script
The goal is to create a bash script named `activesockets` that anyone can execute to view all listening sockets and their associated processes.

### Step 1: Create the Script

```bash
touch activesockets.sh

```

(The .sh suffix is not required)

Open it in a text editor 

Inside the script, add:

```bash
#!/bin/bash
sudo /usr/bin/ss -lnp

```

Absolute path is recomended for security, in order to ensure that only the original linux command will be executed.

### Step 2: Grant Execution Permissions

Make the script executable for all users.

```bash
sudo chmod +x activesockets.sh

```

### Step 3: Configure sudoers

To ensure standard users can run the script without being prompted for a password (because ss -lnp requires root to display process names), modify the sudoers file securely.

```bash
sudo visudo

```

Add the following line to the end of the file:

ALL ALL=(root) NOPASSWD: /usr/bin/ss -lnp

Meaning: All users from all computers will have root privileges with no password in order to run the exact following command. 

### Step 4: Execution

You can now run the script, within the directory which the script is in, type:

```bash
./activesockets.sh

```

### Step 5: Make the Script Globally Accessible

You may move the script to the global binary directory with the command:

```bash
sudo mv activesockets.sh /usr/local/bin/
```

Or if you prefer to keep it in your local directory for easily changing it later, you can create a symbolic link 
for thr script in that directory:

```bash
sudo ln -s activesockets.sh /usr/local/bin/activesockets.sh

```

Now you can simply type the name of script everywhere and it will run.
