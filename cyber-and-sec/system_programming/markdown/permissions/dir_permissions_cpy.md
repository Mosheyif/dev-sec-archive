Reviewer: Alon

# Exercise Guide: Copying Directories and Preserving Permissions

This guide explains how to copy directories, why their permissions might change during the process, and how to preserve the original permissions.

---

## Step 1: Create a Directory with Full Permissions

You can create a directory and assign it specific permissions immediately using the `-m` (mode) flag.

```bash
mkdir -m 777 <directory_name>

```

To verify the permissions, run:

```bash
ls -l

```

*(You should see `drwxrwxrwx`, meaning read, write, and execute permissions are granted to the owner, group, and others).*

## Step 2: Copy the Directory

Copy the directory to a new location (e.g., the home directory `~/`).

```bash
cp -r <directory_name> ~/

```

**What happened to the permissions?**
If you run `ls -l ~/`, you will notice that the copied directory's permissions might have changed.

**Why did it change?**
When you use the standard `cp` command, it doesn't just "move" the original folder; it creates a brand *new* directory at the destination. By default, any new directory is subjected to your shell's **umask** (which is typically `0002` for standard users). The system automatically subtracts the `umask` from the requested permissions, stripping away the write permission for "others".

## Step 3: How to Prevent Permission Changes

To copy a directory while strictly maintaining its exact original permissions, ownership, and timestamps, you must use the `-p` (preserve) flag.

```bash
cp -rp <directory_name> <destination>

```

Alternatively, you can use the `-a` (archive) flag, which is a popular shortcut that automatically implies both `-r` (recursive) and `-p` (preserve):

```bash
cp -a <directory_name> <destination>

```
