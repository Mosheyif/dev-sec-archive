

# Exercise 5: Network Share & Explicit Access Control

> **Worksheet Task:**
> * Create a new directory named "secret" and share it exclusively with a 
>   specific user, "SecretUser". Ensure it is restricted only to that user and 
>   remove all other users.
> * Log in from another PC and test access with both a denied user and 
>   "SecretUser", observing the differences in behavior.

**Objective:** Understand the interaction between SMB Share permissions and 
NTFS file system permissions by creating a strictly isolated network share.

### 5.1 Create the User, Directory, and Apply Permissions (On DC01)

We will perform all administrative setup on the Domain Controller.

1. Open PowerShell as Administrator on `DC01`.
2. Create the standard, non-admin user "SecretUser":
   ```powershell
   $Password = ConvertTo-SecureString "passW12!" -AsPlainText -Force
   New-ADUser -Name "SecretUser" -SamAccountName "SecretUser" -AccountPassword $Password -Enabled $true

   ```

3. Create the physical directory:
```powershell
New-Item -ItemType Directory -Path "C:\secret"

```


4. Configure NTFS Permissions. We will break inheritance to remove all default
users (like the 'Users' group), and explicitly grant access only to
Administrators, SYSTEM, and the new SecretUser:
```cmd
icacls "C:\secret" /inheritance:r
icacls "C:\secret" /grant "Administrators:(OI)(CI)F" "SYSTEM:(OI)(CI)F" "corp\SecretUser:(OI)(CI)M"

```


5. Create the SMB Share and restrict network access exclusively to SecretUser:
```powershell
New-SmbShare -Name "secret" -Path "C:\secret" -ChangeAccess "corp\SecretUser"

```


*(Note: Using specific access parameters in `New-SmbShare` automatically
omits the default 'Everyone' group).*

### 5.2 Test Access with a Denied User (On win-pc-1)

Now we will verify that standard users cannot access the share.

1. On the endpoint (`win-pc-1`), open a standard Command Prompt. This prompt
is running under your current logged-in user (or you can use the `syncuser`
from the previous exercise).
2. Attempt to list the contents of the share:
```cmd
dir \\DC01\secret

```


3. **Result:** You should receive an `Access is denied` or `Logon failure`
error, confirming that the restriction is actively blocking unauthorized
accounts.

### 5.3 Test Access with the Authorized User (On win-pc-1)

We will now authenticate explicitly as `SecretUser` to verify successful access.

1. In the same Command Prompt on `win-pc-1`, map the network share to a local
drive letter (e.g., `S:`) by providing the authorized credentials:
```cmd
net use S: \\DC01\secret /user:corp\SecretUser passW12!

```


2. The command should return `The command completed successfully.`
3. Access the mapped drive and verify you have read/write permissions:
```cmd
dir S:\
echo "Top Secret Data" > S:\confidential.txt
type S:\confidential.txt

```


4. **Result:** The operations will succeed, proving that only the explicitly
defined identity (`SecretUser`) can interact with the restricted directory.
5. Clean up the mapped connection when finished:
```cmd
net use S: /delete

```
