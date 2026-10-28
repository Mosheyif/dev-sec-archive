Reviewer: Alon

# Exercise Guide: File Permissions & Umask Management

## Step 1: Create a New File and Check Default Permissions

First, create an empty file and view its details:

```bash
touch <filename>
ls -l <filename>

```

**Why are these the default permissions?**
When you create a standard file, Linux starts with a base permission of `0666` (Read and Write for everyone). However, the actual permissions are filtered by your shell's `umask` setting. If your default `umask` is `0002` (common for standard users), it masks the write bit for "others" (`0666` minus `0002`), resulting in default permissions of `0664` (`-rw-rw-r--`).

## Step 2: Change Permissions (Two Different Ways)

The goal is to grant **Read and Write** to the owner, and **Execute only** to everyone else (group and others). The correct permission string for this is `-rw---x--x`.

**Method 1: Using Octal (Numeric) Mode**
Calculate the numbers: Owner (`r`+`w` = 4+2 = 6), Group (`x` = 1), Others (`x` = 1).

```bash
chmod 611 <filename>

```

**Method 2: Using Symbolic Mode**
Explicitly define the exact permissions for user (`u`), group (`g`), and others (`o`).

```bash
chmod u=rw,go=x <filename>

```

## Step 3: Change the Default System Permissions

To change the default permissions for any *newly created* files or directories in your current session, you modify the `umask`.

For example, to enforce strict privacy where new files are created with `-rw-------` (accessible only to the owner), use:

```bash
umask 0077

```

*Caution:* Changing the `umask` restricts permissions for all subsequent files and folders created in that specific terminal session.

