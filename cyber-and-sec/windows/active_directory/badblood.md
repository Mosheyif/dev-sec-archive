

# Exercise 3. Enlarge AD Database using BadBlood

**Objective:** Use BadBlood to enlarge your AD database by populating it with
thousands of simulated users, groups, computers, and complex permissions.
This creates a realistic, messy enterprise environment for testing.

#### 3.1 Download and Extract BadBlood
1. Open **PowerShell as Administrator** on `DC01`.
2. Execute the following script to download and extract the BadBlood repository
   from GitHub:
   ```powershell
   # Define paths
   $Url = "https://github.com/davidprowe/BadBlood/archive/refs/heads/master.zip"
   $ZipPath = "C:\BadBlood_master.zip"
   $ExtractPath = "C:\BadBlood"

   # Download the zip file
   [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
   Invoke-WebRequest -Uri $Url -OutFile $ZipPath -UseBasicParsing

   # Extract the archive
   Expand-Archive -Path $ZipPath -DestinationPath $ExtractPath -Force

   ```

#### 3.2 Execute the BadBlood Script

Running this script will take several minutes as it generates thousands of
objects in the background. Do not interrupt the process.

1. In the same PowerShell window, bypass the execution policy and run the tool:
```powershell
# Temporarily bypass execution policy for this session
Set-ExecutionPolicy Bypass -Scope Process -Force

# Navigate to the extracted directory
Set-Location -Path "C:\BadBlood\BadBlood-master"

# Execute the main script
.\Invoke-BadBlood.ps1

```


2. The script will display a prompt asking for confirmation before proceeding.
Type **Enter** to start the population process.
3. Wait for the script to finish completely (it will output colorful logs as it
creates OUs, users, and groups).

#### 3.3 Verification

1. Open **Active Directory Users and Computers** (`dsa.msc`).
2. Refresh the domain tree (F5).
3. Verify that multiple new OUs have been randomly generated and populated with
user accounts and security groups.

