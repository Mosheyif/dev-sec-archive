
# Windows Internals & Deployment: PuTTY Exercises

## Overview
This technical guide outlines the procedures required to fulfill the
three separate deployment and persistence objectives.

---

## Exercise 1: Install PuTTY

**Objective:** Acquire and install the standalone PuTTY executable.

1. Open a **Command Prompt**.
2. Navigate to your designated working directory and download the binary:
   ```cmd
   cd C:\Workspace\PuTTY_Project
   curl -o putty.exe [https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe](https://the.earth.li/~sgtatham/putty/latest/w64/putty.exe)

    ```

---

## Exercise 2: Make PuTTY start on boot using the registry

**Objective:** Establish persistence via the Windows Registry.

1. Open a **Command Prompt**.
2. Inject the target path into the Current User's Run key:


```cmd
reg add "HKCU\Software\Microsoft\Windows\CurrentVersion\Run" ^
  /v "PuTTY_AutoStart" /t REG_SZ ^
  /d "\"C:\Workspace\PuTTY_Project\putty.exe\"" /f

```



---

## Exercise 3: Create a .msi package that installs putty and makes it

## start on boot

**Objective:** Architect an automated `.msi` package that performs both
actions atomically using the WiX Toolset.

### Prerequisite: Environment Preparation

Before packaging, the build environment must be configured with all 
required dependencies and compilation tools. 

Open an **Elevated Command Prompt (Run as Administrator)** to 
perform the setup:

1. **Verify and Enable .NET Framework 3.5 (.NET 3.5):**
   The WiX Toolset compilation engine requires the legacy .NET 3.5 
   feature to be active in the Windows Component Store.
   ```cmd
   dism /Online /Enable-Feature /FeatureName:NetFx3 /All /NoRestart

    ```

2. **Verify Execution Policy and Package Manager (Winget):**
Ensure `winget` is operational to fetch the toolchain.
```cmd
winget --version

```


3. **Install WiX Toolset:**
Download and install the WiX Toolset framework (version 3.14 or
compatible), which will automatically resolve any remaining package dependencies.
```cmd
winget install WiXToolset.WiXToolset

```

### Step 1: Generate Unique Identifiers

Open **PowerShell** and generate three unique GUIDs required for the MSI
database structure:

```powershell
[guid]::NewGuid().ToString().ToUpper()

```

(Run this three times to obtain unique values for the UpgradeCode,
ExecutableComponent, and RegistryComponent).

### Step 2: Construct the WiX Source File (.wxs)

Place the provided `installer.wxs` file in your working directory alongside
`putty.exe`. Ensure you open it and replace the `INSERT-GUID-X`
placeholders with the unique GUIDs generated in Step 1.

### Step 3: Compilation and Linking

Return to the standard **Command Prompt** in your working directory.
Compile the `.wxs` source into an object file, then link it to generate
the final `.msi` package.

```cmd
"C:\Program Files (x86)\WiX Toolset v3.14\bin\candle.exe" installer.wxs

"C:\Program Files (x86)\WiX Toolset v3.14\bin\light.exe" installer.wixobj ^
  -o PuTTY_Final.msi

```

Execution of `PuTTY_Final.msi` will now seamlessly install the binary
and establish boot persistence.