Reviewer: Alon

## Changing File Ownership created by root Without `sudo`

**Task:** Switch user to root and create a file. Then switch back to your user. Find a way to change the file owner and group without using the `sudo` command.

```bash
# 1. Switch to root and create a standard file
sudo su
touch <file_name>
exit

# 2. Back as your regular user, verify the file is readable
# It is owned by root, but default permissions (644) allow others to read it.
ls -l root_file

# 3. Copy the file to a temporary name (the new copy is owned by you)
cp root_file temp_file

# 4. Delete the original root-owned file
# This is permitted because you have write permissions to the parent directory
rm -f root_file

# 5. Rename your copy to the original filename
mv temp_file root_file

Explanation: Linux strictly prohibits regular users from using the chown command to take ownership of files. However, because you have read permissions for the file itself and write/execute permissions for the parent directory, you can securely bypass this restriction. By duplicating the file, deleting the original (which relies on directory permissions, not file permissions), and renaming the copy, you successfully take over the file's owner and group without needing sudo.
