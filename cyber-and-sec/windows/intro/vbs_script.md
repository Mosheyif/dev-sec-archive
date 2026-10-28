
## Exercise 7: Dynamic Script Analysis with Process Monitor

**Objective:** Use Procmon to monitor the script's activity and understand its
main functionality. This involves capturing the runtime events of an obfuscated
VBScript to identify its payload, specifically focusing on registry
manipulations and file drops.

---

### Step 1: Environment Preparation and ProcMon Filtering

To isolate the script's behavior from background operating system noise, a
strict process filter must be applied before execution.

1. Launch **Process Monitor (ProcMon)** with Administrator privileges.
2. Pause the automatic event capture (`Ctrl + E`) and clear any existing
   background events (`Ctrl + X`).
3. Open the **Process Monitor Filter** (`Ctrl + L`) and configure the following
   inclusion rule to target the Windows Script Host:
   * `Process Name` — `is` — `wscript.exe` (or `cscript.exe`) -> **Include**.
4. Click **Add** and then **APPLY** to apply the configuration.

### Step 2: Execution and Data Capture

1. Resume the ProcMon event capture (`Ctrl + E`).
2. Execute the target obfuscated script (`run_me.vbs`).
3. Wait for the script execution to complete, then immediately pause the
   capture (`Ctrl + E`) to prevent log flooding.

### Step 3: Registry Activity Analysis

By toggling the ProcMon toolbar to show only **Registry Activity**, the
script's configuration changes become visible despite its obfuscation.

1. Locate the `RegCreateKey` and subsequent `RegSetValue` operations.
2. **Target Path:** `HKCU\Software\MyVBScriptExample\ExampleValue`
3. **Payload Details:** Examining the Event Properties reveals that the script
   writes a `REG_SZ` (String) value containing the data `"ILRD"`. The length
   is recorded as 10 bytes (accounting for Windows UTF-16LE encoding and the
   null terminator).
4. **Analysis:** This behavior mimics a malware "infection marker" or a basic
   configuration modification stored within the Current User registry hive.

### Step 4: File System Activity Analysis

By toggling the ProcMon toolbar to show only **File System Activity**, the
script's file dropping capabilities are revealed.

1. Locate the `CreateFile` and `WriteFile` operations.
2. **Target Path:**
   `C:\Users\<Username>\AppData\Local\Temp\ExampleFile.txt`
3. **Payload Details:** The script queries the OS for the `%TEMP%` directory
   variable, creates a new text file (using the `OverwriteIf` disposition
   flag), and performs sequential `WriteFile` operations to inject the payload
   `"ILRD\r\n"` (the text followed by a carriage return and line feed).
4. **Analysis:** This demonstrates a classic "dropper" technique. The
   obfuscated loader decodes its instructions in memory and writes a secondary
   payload to a designated, unprivileged disk location.

---

### Conclusion: Main Functionality

Despite the internal mathematical obfuscation designed to evade static
signature-based detection, dynamic monitoring via ProcMon clearly exposes the
script's true functionality. It operates as a rudimentary dropper that performs
two discrete actions:
1. Injects a predefined string (`ILRD`) into a newly created registry key.
2. Drops a corresponding text file containing the same string into the user's
   temporary directory.
