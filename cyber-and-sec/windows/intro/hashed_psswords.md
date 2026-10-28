
## Exercise 6: Find the hashed Passwords in the system.

**Objective:** Identify the storage location of local password hashes within Windows, evaluate the architectural differences between Unix flat-file implementations and Windows binary registry hives, navigate kernel file locking mechanisms, and analyze the internal encryption layout of the Security Account Manager (SAM).

---

### Step 1: Theoretical Mapping – Linux vs. Windows Equivalents

In Linux environments, account metadata and authentication credentials reside in flat ASCII files:
* User configuration and UIDs: `/etc/passwd`
* Salted password hashes: `/etc/shadow` (governed by standard file permissions `0640`/`0600`).

In Windows, local accounts are governed by the Security Account Manager (SAM) subsystem. Unlike Unix text streams, credentials are stored inside proprietary binary registry hives:

| Component | Physical Disk Location | Registry Mounting Path | Architecture Role |
| :--- | :--- | :--- | :--- |
| **SAM Hive** | `C:\Windows\System32\config\SAM` | `HKEY_LOCAL_MACHINE\SAM` | Stores local account structures, RIDs, group memberships, and encrypted NTLM hash blocks. |
| **SYSTEM Hive** | `C:\Windows\System32\config\SYSTEM` | `HKEY_LOCAL_MACHINE\SYSTEM` | Contains system configuration, device drivers, and the Boot Key (Syskey) required to decrypt the SAM. |

---

### Step 2: The Kernel File Lock Constraint

Attempting to read or copy the on-disk SAM file directly using standard command-line tools fails immediately:

```cmd
type C:\Windows\System32\config\SAM

```

**Observed Error:**

```text
The process cannot access the file because it is being used by another process.

```

**Root Cause Analysis:**
Upon OS boot, the NT Kernel Configuration Manager maps the binary hive files directly into memory and places an exclusive kernel-level file lock (`FILE_SHARE_READ` denied) on them. As long as the operating system kernel is active, standard Win32 I/O operations are blocked from reading these files directly from disk.

---

### Step 3: Extracting Registry Hives via Native APIs

To capture a consistent snapshot of the active hives without rebooting into an offline environment (such as WinPE or Linux Live USB), administrative tools invoke the Windows Configuration Manager API (`RegSaveKeyEx`).

1. Open an **Elevated PowerShell (Run as Administrator)**.
2. Execute the `reg save` utility, which leverages the caller's `SeBackupPrivilege` to query the kernel for an on-disk export of the in-memory hive:

```powershell
reg save HKLM\SAM "$PWD\sam.hiv" /y
reg save HKLM\SYSTEM "$PWD\system.hiv" /y

```

3. Confirm that both binary hive exports exist in your designated directory:

```powershell
Get-ChildItem -Path .\*.hiv

```

---

### Step 4: Loading and Inspecting the Binary Structure

Because `.hiv` files are binary databases (composed of bin blocks, cells, and subkeys), running text parsers produces binary gibberish. To visually inspect the underlying schema without corrupting live system hives, mount the backup under a temporary logical key:

1. Mount the extracted SAM hive:

```powershell
reg load HKLM\TempSAM "$PWD\sam.hiv"

```

2. Open the **Registry Editor**:

```cmd
regedit

```

3. Navigate down the object hierarchy:

```text
HKEY_LOCAL_MACHINE\TempSAM\SAM\Domains\Account\Users

```

4. Observe the user account schema:
* **`Names` Subkey:** Maps plain-text usernames (e.g., `Administrator`, `Guest`, local lab accounts) to their corresponding hexadecimal Relative Identifier (RID).
* **RID Keys (e.g., `000001F4` for RID 500):** Contains two binary structures:
* **`F` (Fixed Length):** Account metadata, expiration timestamps, logon statistics, and status flags.
* **`V` (Variable Length):** Binary offsets and lengths describing account descriptions, user strings, and encrypted password hash blocks.




5. Unmount the temporary hive cleanly after inspection:

```powershell
reg unload HKLM\TempSAM

```

---

### Appendix: The Multi-Tiered Hash Derivation and Decryption Flow

The NTLM hash in Windows is never stored in clear-text or simple base64, nor can it be resolved from the SAM hive in isolation. The operating system utilizes a four-tier cryptographic process combining the SAM hive, the SYSTEM hive, and the user's RID:

**Phase 1: Boot Key (Syskey) Reconstruction**

* The Boot Key is a 16-byte symmetric key fragmented across four separate subkeys in the SYSTEM hive:
* `HKLM\SYSTEM\CurrentControlSet\Control\Lsa\JD`
* `HKLM\SYSTEM\CurrentControlSet\Control\Lsa\Skew1`
* `HKLM\SYSTEM\CurrentControlSet\Control\Lsa\GBG`
* `HKLM\SYSTEM\CurrentControlSet\Control\Lsa\Data`


* The system extracts the text values, concatenates them into a single byte stream, and applies a static permutation matrix to construct the 16-byte **Boot Key**.

**Phase 2: SAM Encryption Key Decryption**

* Inside `HKLM\SAM\SAM\Domains\Account`, the binary value `F` contains an encrypted data block and an Initialization Vector (IV).
* The OS uses the reconstructed **Boot Key** to decrypt this block (using AES-128-CBC on modern systems, or RC4 historically), producing the master **SAM Key**.

**Phase 3: Per-User Key Derivation**

* To ensure isolation across local accounts, the OS derives an encryption key unique to each individual user.
* It concatenates the master **SAM Key** with the user's **RID** (4 bytes, Little-Endian):

$$\text{UserKey} = \text{Hash}(\text{SAM\_Key} \,\Vert{}\, \text{RID})$$


* On contemporary Windows versions, this derivation utilizes HMAC-SHA256 (or MD5 in legacy structures).

**Phase 4: Resolving the NTLM Hash**

* The user's `V` value contains the encrypted password block and its local IV.
* The system applies the unique **UserKey** (via AES-128) to decrypt the block.
* The resulting 16-byte output is the standard raw **NTLM Hash** ($MD4(\text{UTF-16LE}(\text{Password}))$).


---

### Step 5: Automated Offline Extraction via Impacket

While manual inspection reveals registry architecture, recovering hashes
requires calculating the cryptographic chain detailed in the Appendix.
The `secretsdump` utility from the Impacket suite performs this offline
decryption workflow directly against the extracted binary hives.

1. Execution Syntax:

```bash
impacket-secretsdump -sam <sam.hiv> -system <system.hiv> LOCAL

```

2. Operational Mechanism:

* **Syskey Parsing:** Reads the `SYSTEM` hive to reconstruct the Boot Key
from the class metadata of `JD`, `Skew1`, `GBG`, and `Data`.
* **Master Key Decryption:** Uses the Syskey to decrypt the binary `F` value
in `SAM\Domains\Account`, extracting the Password Encryption Key (PEK).
* **Hash Resolution:** Iterates through user RID subkeys, computes individual
per-user keys, and decrypts the `V` values to reveal raw NTLM hashes.

3. Output Schema:
Extracted local credentials are emitted in standard pwdump notation:

```text
Username:RID:aad3b435b51404eeaad3b435b51404ee:<NTLM-Hash>:::

```
