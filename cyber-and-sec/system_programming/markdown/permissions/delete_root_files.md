Reviewer: Alon

# Exercise Guide: Deleting Root-Owned Files and Directories

This guide explores what happens when a standard user attempts to delete root-owned files, empty directories, and populated directories located within their own home folder.

---

## Step 1: Create the Test Environment Properly
First, create a test directory and enter it. To create root-owned files without changing your current directory, either use `sudo` before each command or use `sudo su` (without the `-` flag).

```bash
mkdir test_delete
cd test_delete

# Create files and directories as root
sudo mkdir root_folder
sudo mkdir root_rec_folder
sudo touch root_file
sudo touch root_rec_folder/root_rec_file

```

Step 2: Attempting Deletion as a Regular User

Now, as your regular user (who owns the parent test_delete directory), what happens when you try to delete these items?

# Case 1: Deleting a Root-Owned File

```bash

rm root_file

```

The system will prompt you: rm: remove write-protected regular empty file 'root_file'?. If you type y (yes) or use rm -f, the file will be deleted successfully.

Why: The ability to delete a file is governed by the permissions of the parent directory, not the file itself. Since you own test_delete, you can remove pointers from it.

# Case 2: Deleting an Empty Root-Owned Directory

```bash

rm -r root_folder

```

The system will prompt you for confirmation. If you accept, the directory will be deleted successfully.

Why: Just like the file, an empty directory is just an entry in the parent directory. Your write permissions on test_delete allow you to remove it.

# Case 3: Deleting a Populated Root-Owned Directory

```bash

rm -r root_rec_folder

```

The system will fail and return: rm: cannot remove 'root_rec_folder/root_rec_file': Permission denied.

Why: To delete a directory using rm -r, Linux must first go inside it and delete all its contents. However, root_rec_folder is owned by root and has default permissions of 755 (drwxr-xr-x). Because you do not have write permissions to inside root_rec_folder, you cannot delete root_rec_file. Since the file cannot be deleted, the folder cannot be emptied, and thus the deletion operation fails entirely.

Conclusion & Key Takeaway

A non-root user can delete root-owned files and empty directories if they are located inside a directory the user owns. However, the user cannot delete a root-owned directory if it contains files, because the user lacks the required write permissions to empty it first.
