

# Active Directory Lab Setup & Administration Runbook

## 1. Architecture & Environment Overview
This lab simulates a standard enterprise Active Directory environment.

* **Domain Controller (DC):** 1x Windows Server 2016 VM (`win_server`).
* **Endpoints:** 2x Windows 10 Pro VMs (`win-pc-1`, `win-pc-2`).
* **OS Version Requirement:** The endpoint VMs must run Windows Pro (or
  Enterprise). The Home edition lacks the necessary OS components to join 
  an Active Directory domain.
* **Networking:** All machines reside on a shared, isolated local subnet 
  (e.g., `192.168.5.0/24`).

---

## 2. Exercise 1: Active Directory Lab Setup & Provisioning

**Objective:** Complete the end-to-end deployment of the Active Directory
environment, including environment preparation, Domain Controller promotion, 
and user provisioning.

### Step 1: Environment Preparation & VM Cloning (ESXi)

#### 1.1 Machine SID Conflict Prevention (Sysprep)
Every Windows installation generates a unique Security Identifier (Machine
SID). Cloning a Windows machine directly without generalizing it duplicates
this SID. In an Active Directory environment, duplicate machine SIDs cause
serious authentication, computer account trust, and domain join collisions.

**Before cloning any baseline VM, execute Sysprep inside the source:**
1. Open PowerShell / Command Prompt as Administrator in the source VM.
2. Execute the Generalization process:
   ```cmd
   C:\Windows\System32\Sysprep\sysprep.exe /generalize /oobe /shutdown

   ```

* `/generalize`: Strips machine-specific data, including the unique SID.
* `/oobe`: Boots into the Out-of-Box Experience on the next startup to
generate new security identities.
* `/shutdown`: Automatically powers down the machine once the image is
generalized, keeping it clean for duplication.

#### 1.2 VM Provisioning & Manual Cloning (Standalone ESXi)

In a standalone ESXi host (without vCenter), VMs can be deployed from
scratch or duplicated using the Datastore Browser:

* **From Scratch:** Navigate to **Virtual Machines** -> **Create / Register
VM** -> Select **Create a new virtual machine** and follow the wizard.
* **Cloning via Datastore (Post-Sysprep):**
1. Confirm the source VM is fully shut down.
2. Navigate to **Storage** -> Click your datastore -> Open Datastore browser.
3. Click **Create directory** and name it after your destination VM.
4. Enter source VM directory, select configuration files and virtual
disks (`.vmx`, `.vmdk`), click **Copy**, and paste into the target.
5. Return to **Virtual Machines** -> **Create / Register VM** -> Select
**Register an existing virtual machine** -> Browse to the new folder,
select the `.vmx` file, and finish registration.
6. Power on the cloned VM. When ESXi prompts a question regarding VM
movement, select **"I copied it"** (generates a new MAC and UUID).
7. Complete brief initial setup, assign unique computer name, and set
a distinct static IP address.



#### 1.3 Infrastructure Access & Console Rules

* **Console Access:** Initial configurations must be executed via the
ESXi Web Console due to isolated network boundaries.
* **Windows Credential Prompt (Ctrl+Alt+Delete):** Physical keyboard
combinations are trapped by the host OS. To unlock Windows in ESXi:
* Click **Actions** -> **Guest OS** -> **Send keys** -> **Ctrl-Alt-Delete**.


* **Network Isolation:** All 3 virtual network adapters must be mapped
to the exact same **Port Group** on the ESXi Virtual Switch to ensure
Layer 2 connectivity.

---

### Step 2: Deploying the Domain Controller

**Objective:** Create a virtual machine running Windows Server and make
it run as Active Directory.

#### 2.1 Server Baseline Configuration

A Domain Controller requires a predictable hostname before promoting it.

1. Access the `win_server` VM via the ESXi Web Console.
2. Open **PowerShell as Administrator**.
3. Rename the server to `DC01` and reboot:
```powershell
Rename-Computer -NewName "DC01" -Force
Restart-Computer -Force

```



#### 2.2 Install Active Directory (AD DS)

Once the server reboots, log back in as Administrator. Install the AD DS
binaries and management tools.

1. Open **PowerShell as Administrator**.
2. Execute the installation command:
```powershell
Install-WindowsFeature -Name AD-Domain-Services -IncludeManagementTools

```



#### 2.3 Promote to a Domain Controller

Promote the server to create a new Forest and Domain (e.g., `corp.local`).
This step will automatically install DNS Server and prompt a reboot.

1. Execute the following PowerShell commands (replace `NoPassword!` with
your lab's standard password):
```powershell
$Password = ConvertTo-SecureString "NoPassword!" -AsPlainText -Force

Install-ADDSForest `
    -DomainName "corp.local" `
    -DomainNetbiosName "CORP" `
    -SafeModeAdministratorPassword $Password `
    -InstallDns:$true `
    -Force:$true

```


2. Wait for the server to configure the directory and reboot automatically.
3. *Note: After rebooting, log in using domain credentials: `CORP\Administrator`.*

#### 2.4 Network & DNS Hardening (Post-Reboot)

To ensure stable domain operations, the DC must have a static IP address,
and its DNS client must point to itself.

1. Identify the network adapter name:
```powershell
Get-NetAdapter

```


2. Set the static IP and DNS server (assuming adapter is `Ethernet0` and
IP is `192.168.5.40`):
```powershell
Set-NetIPAddress -InterfaceAlias "Ethernet0" -IPAddress "192.168.5.40" `
    -PrefixLength 24
Set-DnsClientServerAddress -InterfaceAlias "Ethernet0" `
    -ServerAddresses ("127.0.0.1", "192.168.5.40")

```



#### 2.5 Validation

Verify that the domain is active and DNS SRV records are resolvable.

```powershell
# Verify Domain configuration
Get-ADDomain

# Verify LDAP SRV record resolution
Resolve-DnsName -Name _ldap._tcp.dc._msdcs.corp.local

```

*Expected Result: The DNS resolution should return the IP address of DC01.*

---

### Step 3: User Management

**Objective:** Create 2 users in your Active Directory.

#### 3.1 Create Domain User Accounts

Create dedicated user accounts mapped for endpoint machines
(Oliver Queen for `win-pc-1` and Felicity Smoak for `win-pc-2`).

1. Open **PowerShell as Administrator** on `DC01`.
2. Execute the user creation script:
```powershell
$Password = ConvertTo-SecureString "ArrowPass123!" -AsPlainText -Force

# Create user for win-pc-1 (Oliver Queen)
New-ADUser -Name "Oliver Queen" `
    -SamAccountName "oqueen" `
    -UserPrincipalName "oqueen@corp.local" `
    -GivenName "Oliver" `
    -Surname "Queen" `
    -AccountPassword $Password `
    -Enabled $true `
    -PasswordNeverExpires $true

# Create user for win-pc-2 (Felicity Smoak)
New-ADUser -Name "Felicity Smoak" `
    -SamAccountName "fsmoak" `
    -UserPrincipalName "fsmoak@corp.local" `
    -GivenName "Felicity" `
    -Surname "Smoak" `
    -AccountPassword $Password `
    -Enabled $true `
    -PasswordNeverExpires $true

```



#### 3.2 Validation

Verify that the users have been successfully provisioned:

```powershell
Get-ADUser -Filter * -Properties GivenName, Surname

```

---

### Step 4: Group Management

**Objective:** Create 2 groups and add a different user to each group.

#### 4.1 Create Security Groups

Create two global security groups to organize users logically across
departments or operational teams.

1. Open **PowerShell as Administrator** on `DC01`.
2. Provision the security groups:
```powershell
New-ADGroup -Name "Team-Arrow" -GroupScope Global `
    -GroupCategory Security
New-ADGroup -Name "Team-Tech" -GroupScope Global `
    -GroupCategory Security

```



#### 4.2 Assign Members to Groups

Add each provisioned domain user to their designated security group.

1. Map `oqueen` to `Team-Arrow`:
```powershell
Add-ADGroupMember -Identity "Team-Arrow" -Members "oqueen"

```


2. Map `fsmoak` to `Team-Tech`:
```powershell
Add-ADGroupMember -Identity "Team-Tech" -Members "fsmoak"

```



#### 4.3 Validation

Confirm group membership assignments:

```powershell
Get-ADGroupMember -Identity "Team-Arrow"
Get-ADGroupMember -Identity "Team-Tech"

```

*Expected Result: The query returns Oliver Queen for Team-Arrow and Felicity*
*Smoak for Team-Tech.*

---

### Step 5: Endpoint Provisioning and Domain Join

**Objective:** Create 2 Windows VMs and connect them to your Active
Directory.

#### 5.1 Endpoint Network Configuration

To successfully join the domain, the endpoints must use the Domain Controller
as their primary DNS server. Perform these steps on both `win-pc-1` and
`win-pc-2`.

1. Open **PowerShell as Administrator** on the endpoint.
2. Verify the network adapter name:
```powershell
Get-NetAdapter

```


3. Set a static IP and point the DNS to `DC01` (`192.168.5.40`):
```powershell
# On win-pc-1 (Oliver Queen):
Set-NetIPAddress -InterfaceAlias "Ethernet0" `
    -IPAddress "192.168.5.38" -PrefixLength 24
Set-DnsClientServerAddress -InterfaceAlias "Ethernet0" `
    -ServerAddresses ("192.168.5.40")

# On win-pc-2 (Felicity Smoak):
Set-NetIPAddress -InterfaceAlias "Ethernet0" `
    -IPAddress "192.168.5.39" -PrefixLength 24
Set-DnsClientServerAddress -InterfaceAlias "Ethernet0" `
    -ServerAddresses ("192.168.5.40")

```


4. Verify communication with the domain controller:
```powershell
Test-Connection -ComputerName "corp.local" -Count 2

```



#### 5.2 Join the Active Directory Domain

Once DNS is configured and verified, join the machines to the domain.

1. Execute the domain join command on each endpoint:
```powershell
Add-Computer -DomainName "corp.local" `
    -Credential "CORP\Administrator" -Restart

```


2. When prompted, enter the Domain Administrator password. The machine will
automatically restart upon a successful join.

#### 5.3 Validation & User Login

After the reboot, the machines are officially part of the domain.

1. At the Windows login screen, select **Other user**.
2. Log in using the newly created domain accounts:
* On `win-pc-1`: Log in as `CORP\oqueen` (or `oqueen@corp.local`).
* On `win-pc-2`: Log in as `CORP\fsmoak` (or `fsmoak@corp.local`).



---

### Step 6: Group Policy (GPO) Configuration - Desktop Wallpaper

**Objective:** In the first group, define a policy that changes the
desktop background.

#### 6.1 Create and Link the GPO

Group Policies link to OUs or Domains, not directly to Security Groups.
To target `Team-Arrow`, link the GPO to the root domain and configure
Security Filtering.

1. Open **PowerShell as Administrator** on `DC01`.
2. Create a new GPO and link it to the root domain:
```powershell
New-GPO -Name "Team-Arrow-Wallpaper" | New-GPLink -Target "dc=corp,dc=local"

```



#### 6.2 Configure Security Filtering & Server Exclusion

Strictly restrict policy processing to `Team-Arrow`, ensure client machine
access, and explicitly exclude `DC01$` to protect server settings.

1. Revoke default Apply permissions from Authenticated Users:
```powershell
Set-GPPermissions -Name "Team-Arrow-Wallpaper" -TargetType Group `
    -TargetName "Authenticated Users" -PermissionLevel None

```


2. Grant read and apply permissions exclusively to `Team-Arrow`:
```powershell
Set-GPPermissions -Name "Team-Arrow-Wallpaper" -TargetType Group `
    -TargetName "Team-Arrow" -PermissionLevel GpoApply

```


3. Grant Domain Computers read access (required for group processing):
```powershell
Set-GPPermissions -Name "Team-Arrow-Wallpaper" -TargetType Group `
    -TargetName "Domain Computers" -PermissionLevel GpoRead

```


4. Exclude the Domain Controller computer account from applying this GPO:
```powershell
Set-GPPermissions -Name "Team-Arrow-Wallpaper" -TargetType Computer `
    -TargetName "DC01$" -PermissionLevel None

```



#### 6.3 Stage the Wallpaper File in SYSVOL

Stage the wallpaper file in the domain's replicated SYSVOL folder so all
endpoints can read it over the network.

1. Open **PowerShell as Administrator** on `DC01`.
2. Create the directory and download the wallpaper:
```powershell
$TargetDir = "C:\Windows\SYSVOL\sysvol\corp.local\scripts"
if (-not (Test-Path -Path $TargetDir)) {
    New-Item -ItemType Directory -Path $TargetDir -Force | Out-Null
}

$Url = '[https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTds3uuZ7ci38YPLnkX9iDqLNbmwNkIte0KsuJ6XtX-_w&s=10](https://encrypted-tbn0.gstatic.com/images?q=tbn:ANd9GcTds3uuZ7ci38YPLnkX9iDqLNbmwNkIte0KsuJ6XtX-_w&s=10)'
$OutFile = "$TargetDir\arrow_bg.jpg"

[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
Invoke-WebRequest -Uri $Url -OutFile$OutFile -UseBasicParsing

```



#### 6.4 Apply Wallpaper via Registry Policy

Configure the Active Desktop wallpaper settings within the GPO:

1. Set the Wallpaper network path:
```powershell
$Key = "HKCU\Software\Microsoft\Windows\CurrentVersion\Policies\System"

Set-GPRegistryValue -Name "Team-Arrow-Wallpaper" -Key $Key `
    -ValueName "Wallpaper" -Type String `
    -Value "\\DC01\SYSVOL\corp.local\scripts\arrow_bg.jpg"

```


2. Set the Wallpaper display style (2 = Stretch):
```powershell
Set-GPRegistryValue -Name "Team-Arrow-Wallpaper" -Key $Key `
    -ValueName "WallpaperStyle" -Type String -Value "2"

```



#### 6.5 Client Verification

The policy applies automatically during regular refresh cycles. To enforce
and verify it immediately:

1. Log in to `win-pc-1` as `CORP\oqueen`.
2. Force an immediate Group Policy update and sign out:
```powershell
gpupdate /force
logoff

```


3. Sign back in as `CORP\oqueen` to view the new wallpaper.
4. Log in to `win-pc-2` as `CORP\fsmoak` and confirm that the default
background remains unchanged.

---

### Step 7: Logon Script Policy (GPO) - Automated Ping

**Objective:** In the second group, define a policy that sends a ping to
the other computer upon login.

#### 7.1 Enable ICMP Echo Requests on the Target Endpoint

By default, Windows Defender Firewall blocks inbound ICMP traffic. Allow
incoming ping requests on the destination workstation.

1. Open **PowerShell as Administrator** on `win-pc-1`.
2. Create an inbound firewall rule for ICMPv4:
```powershell
New-NetFirewallRule -DisplayName "Allow ICMPv4-In" `
    -Protocol ICMPv4 -IcmpType 8 -Action Allow

```



#### 7.2 Create and Link the GPO

Create a dedicated GPO to manage the logon ping behavior.

1. Open **PowerShell as Administrator** on `DC01`.
2. Create and link the GPO to the domain:
```powershell
New-GPO -Name "Team-Tech-Ping" | New-GPLink -Target "dc=corp,dc=local"

```



#### 7.3 Configure Security Filtering & Server Exclusion

Ensure execution is strictly restricted to Felicity's group (`Team-Tech`),
while preventing server-side processing on `DC01`.

1. Revoke default Apply permissions from Authenticated Users:
```powershell
Set-GPPermissions -Name "Team-Tech-Ping" -TargetType Group `
    -TargetName "Authenticated Users" -PermissionLevel None

```


2. Grant read and apply permissions exclusively to `Team-Tech`:
```powershell
Set-GPPermissions -Name "Team-Tech-Ping" -TargetType Group `
    -TargetName "Team-Tech" -PermissionLevel GpoApply

```


3. Grant Domain Computers read access:
```powershell
Set-GPPermissions -Name "Team-Tech-Ping" -TargetType Group `
    -TargetName "Domain Computers" -PermissionLevel GpoRead

```


4. Exclude the Domain Controller computer account:
```powershell
Set-GPPermissions -Name "Team-Tech-Ping" -TargetType Computer `
    -TargetName "DC01$" -PermissionLevel None

```



#### 7.4 Define the Logon Ping Execution via Registry

Configure the policy to execute a ping command directed at `win-pc-1`
(`192.168.5.38`) upon user logon, redirecting output to a verification file.

1. Set the user Run registry value:
```powershell
$Key = "HKCU\Software\Microsoft\Windows\CurrentVersion\Run"
$Cmd = "cmd.exe /c ping 192.168.5.38 -n 4 > C:\Users\Public\ping.txt"

Set-GPRegistryValue -Name "Team-Tech-Ping" -Key $Key `
    -ValueName "LogonPing" -Type String -Value $Cmd

```



#### 7.5 Validation

Verify the logon action from Felicity's workstation.

1. Log in to `win-pc-2` as `CORP\fsmoak`.
2. Run standard PowerShell to synchronize policy and sign out:
```powershell
gpupdate /force
logoff

```


3. Sign back in as `CORP\fsmoak`.
4. Check the generated ping log:
```powershell
Get-Content -Path "C:\Users\Public\ping.txt"

```



*Expected Result: The file contains 4 successful ICMP reply packets with*
*0% packet loss.*


---

### Step 8: Software Deployment Policy (GPO)

**Objective:** Create a policy that installs Chrome across the domain.

#### 8.1 Download, Unblock, and Share the Chrome MSI
Software deployment requires an MSI package hosted on a network share with
explicit read permissions for all domain computers.

1. Open **PowerShell as Administrator** on `DC01`.
2. Execute the following script to stage the software package:
   ```powershell
   # Create directory and download the Chrome Enterprise MSI
   New-Item -ItemType Directory -Path "C:\Software" -Force | Out-Null
   $Url = "[https://dl.google.com/dl/chrome/install/googlechromestandaloneenterprise64.msi](https://dl.google.com/dl/chrome/install/googlechromestandaloneenterprise64.msi)"
   $OutFile = "C:\Software\GoogleChrome.msi"
   Invoke-WebRequest -Uri $Url -OutFile$OutFile -UseBasicParsing

   # Unblock the downloaded file to prevent Windows Security GUI prompts in GPMC
   Unblock-File -Path $OutFile

   # Share the directory and grant NTFS permissions to Domain Computers
   net share Software$=C:\Software /grant:everyone,READ
   icacls "C:\Software" /grant "Domain Computers:(OI)(CI)RX" /T

   ```

#### 8.2 Create and Link the GPO (Lab vs. Production Approaches)

Create the deployment policy and bypass Windows Fast Startup to ensure
the installation triggers immediately upon reboot. Choose the appropriate
security approach below:

**Option A: Lab Environment (Strict Compliance)**
Applies the policy to every machine in the domain, including the Domain
Controller, to strictly satisfy the "all the domain" wording of the exercise.

```powershell
New-GPO -Name "Domain-Chrome-Deployment" | New-GPLink -Target "dc=corp,dc=local"

# Force Windows to wait for the network at startup (bypass Fast Logon)
$Key = "HKLM\Software\Policies\Microsoft\Windows NT\CurrentVersion\Winlogon"
Set-GPRegistryValue -Name "Domain-Chrome-Deployment" -Key $Key `
    -ValueName "SyncForegroundPolicy" -Type DWord -Value 1

```

**Option B: Production Environment (Security Best Practice)**
In real-world scenarios, installing end-user applications on a Domain
Controller expands the attack surface. This option explicitly denies the DC
from applying the policy.

```powershell
New-GPO -Name "Domain-Chrome-Deployment" | New-GPLink -Target "dc=corp,dc=local"

# Explicitly deny the Domain Controller from applying this GPO
Set-GPPermissions -Name "Domain-Chrome-Deployment" -TargetType Computer `
    -TargetName "DC01$" -PermissionLevel None -Replace

# Force Windows to wait for the network at startup (bypass Fast Logon)
$Key = "HKLM\Software\Policies\Microsoft\Windows NT\CurrentVersion\Winlogon"
Set-GPRegistryValue -Name "Domain-Chrome-Deployment" -Key $Key `
    -ValueName "SyncForegroundPolicy" -Type DWord -Value 1

```

#### 8.3 Configure the Software Installation Package (GUI)

The MSI package must be assigned via the GPMC using its network UNC path.

1. On `DC01`, press `Win + R`, type `gpmc.msc`, and hit Enter.
2. Expand `Forest: corp.local` -> `Domains` -> `corp.local`.
3. Right-click the **Domain-Chrome-Deployment** GPO and select **Edit**.
4. Navigate to:
**Computer Configuration** -> **Policies** -> **Software Settings** ->
**Software installation**.
5. Right-click the empty right pane, select **New** -> **Package...**
6. **CRITICAL:** Do not paste the local path. Navigate via the network browser:
Click **Network** -> **DC01** -> **Software$**, select `GoogleChrome.msi`,
and click **Open**.
7. In the Deploy Software dialog, select **Assigned** and click **OK**.
8. Wait a few seconds until Google Chrome appears in the list, then close
the editor.

#### 8.4 Validation & Client Execution

1. Log in to a targeted domain machine (e.g., `win-pc-1`).
2. Open **PowerShell as Administrator** and force a policy update:
```powershell
gpupdate /force

```


3. When prompted `Restart now? (yes/no)`, type **y** and press Enter.
4. During boot, Windows displays "Installing managed software Google Chrome...".
After login, verify Google Chrome is available on the desktop.


---

### Step 9: Browser Standardization (Migrate to Firefox)

**Objective:** Enforce company policy by completely removing Google Chrome
and deploying Mozilla Firefox domain-wide.

#### 9.1 Remove the Existing Chrome Deployment
To uninstall Chrome, the existing assigned package must be configured for
explicit removal. Do not simply delete the GPO.

1. On `DC01`, open Group Policy Management (`gpmc.msc`).
2. Edit the **Domain-Chrome-Deployment** GPO.
3. Navigate to **Computer Configuration** -> **Policies** -> 
   **Software Settings** -> **Software installation**.
4. Right-click the **Google Chrome** package in the right pane.
5. Select **All Tasks** -> **Remove...**
6. Select **Immediately uninstall the software from users and computers**
   and click **OK**.

#### 9.2 Download and Stage the Firefox MSI
Download the Firefox MSI using TLS 1.2 to avoid secure channel errors.

1. Open **PowerShell as Administrator** on `DC01`.
2. Execute the following script:
   ```powershell
   # Force TLS 1.2 for the download session
   [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
   
   $Url = "[https://download.mozilla.org/?product=firefox-msi-latest-ssl&os=win64&lang=en-US](https://download.mozilla.org/?product=firefox-msi-latest-ssl&os=win64&lang=en-US)"
   $OutFile = "C:\Software\Firefox.msi"
   
   # Use WebClient to handle redirects smoothly
   $WebClient = New-Object System.Net.WebClient
   $WebClient.DownloadFile($Url, $OutFile)
   
   # Unblock the file to prevent GUI security prompts during GPO assignment
   Unblock-File -Path $OutFile

   ```

#### 9.3 Create and Link the Firefox GPO

Create a new deployment policy and bypass Fast Logon to ensure the
installation triggers immediately.

1. Execute the following PowerShell script on `DC01`:
```powershell
# Create and link the new GPO to the domain root
New-GPO -Name "Domain-Firefox-Deployment" | New-GPLink -Target "dc=corp,dc=local"

# Force Windows to wait for the network at startup
$Key = "HKLM\Software\Policies\Microsoft\Windows NT\CurrentVersion\Winlogon"
Set-GPRegistryValue -Name "Domain-Firefox-Deployment" -Key $Key `
    -ValueName "SyncForegroundPolicy" -Type DWord -Value 1

```



#### 9.4 Assign the Firefox Package (GUI)

1. In GPMC, right-click **Domain-Firefox-Deployment** and select **Edit**.
2. Navigate to **Computer Configuration** -> **Policies** ->
**Software Settings** -> **Software installation**.
3. Right-click the empty right pane -> **New** -> **Package...**
4. Navigate via the network browser: **Network** -> **DC01** -> **Software$**.
5. Select `Firefox.msi` and click **Open**.
6. In the Deploy Software dialog, select **Assigned** and click **OK**.
7. Wait until Mozilla Firefox appears in the list, then close the editor.

#### 9.5 Client Execution & Validation

Uninstall/Install GPOs may not trigger a restart prompt automatically via
standard gpupdate. A manual reboot is strictly required.

1. Log in to the domain endpoints (and the DC).
2. Open **PowerShell as Administrator** and execute:
```powershell
gpupdate /force
Restart-Computer -Force

```


3. During boot, Windows will display messages indicating the removal of
Google Chrome followed by the installation of Mozilla Firefox.
4. Upon login, verify that Chrome is gone and Firefox is available.


## Exercise 2: Environment Backup

### 1. Take Snapshots of Your VMs

**Objective:** Create a restorable state for all virtual machines before
proceeding to advanced or potentially destructive lab exercises.

#### 1.1 Snapshot Creation
Perform this action on your hypervisor (VMware Workstation, Hyper-V, or VirtualBox).
It is recommended to gracefully shut down the VMs before taking the snapshot to
ensure data consistency, though live snapshots are also acceptable.

1. Open your hypervisor management console.
2. For each virtual machine (`DC01`, `win-pc-1`, and `win-pc-2`):
   * Right-click the VM and select **Snapshot** -> **Take Snapshot** (or equivalent).
   * **Name:** `Post-Exercise-1_Base-Domain`
   * **Description:** `Active Directory installed, basic GPOs applied, and Firefox deployed.`
3. Verify that the snapshots were created successfully in the Snapshot Manager.

