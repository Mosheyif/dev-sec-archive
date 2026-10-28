Reviewer: Alon

## Create a Secure Root File

**Task:** Login as root and create a file with no permissions for other and group, then switch back to your own user.

```bash
# 1. Switch to the root user account
sudo su

# 2. Create the file.
touch <file_name>

# 3. Remove permissions for 'group' and 'others' (set to 600)
chmod 600 <file_name>

# 4. Switch back to your regular user
exit

```

# Can be done shortly using the command:

```bash
sudo touch <file_name> && sudo chmod 600 <file_name>

```
