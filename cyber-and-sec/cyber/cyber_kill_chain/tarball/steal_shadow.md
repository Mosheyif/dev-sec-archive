
# SUID Binary & Script Deobfuscation Guide: Reading /etc/shadow

## Overview
This guide covers the step-by-step resolution of the tarball challenge, which involves an SUID-enabled binary (`run_me`) and an obfuscated Python script. By analyzing execution flow, deobfuscating the script's logic, and resolving file-overwrite conflicts, we can leverage root privileges to read protected system files like `/etc/shadow` without using `sudo`.

---

## Step 1: Environment Setup & Permissions
1. Extract the tarball:
   ```bash
   tar -xzvf tarball.tar.gz
   ```

2. Rename the extracted script to include the `.py` extension so that the Python interpreter can execute it correctly:
   ```bash
   mv script script.py
   ```

3. Ensure the binary has the correct permissions, ownership, and SUID bit set:
   ```bash
   sudo chown root:root run_me
   chmod 755 run_me
   chmod u+s run_me
   ```

---

## Step 2: Comprehensive Script Analysis (`script.py`)
The provided script is heavily obfuscated to hide its true mechanics. Below is a breakdown of how it works under the hood:

- **Custom Bitwise & XOR Math Functions (`w`, `ww`, `www`, `wwww`):** The script defines several custom operators combining characters using arithmetic (`+`, `*`, `//`) and bitwise XOR (`^`) wrapped in a cycle over ASCII sets (`string.ascii_lowercase`, `string.ascii_uppercase`, etc.).
- **Dynamic Decoding Strings (`f2()`, `ll`, `erwqyuioa`):** 
  - The `ll` list contains arrays of characters representing binary, octal, or hexadecimal fragments (e.g., `['c', 'h', 'r', '(', '0', 'b', '0', 'b', ...]`). 
  - The function `f2()` dynamically evaluates these fragments using `eval()` to build hidden text strings piece by piece at runtime.
- **Compressed & Encoded Payloads (`ee`, `s97`, `s96`):** 
  - The variable `ee` holds compressed zlib bytes (`b'x\x9c...'`) which are decompressed and mapped via `po()` and `pl()` utilities.
  - The arrays `s97` and `s96` act as index mappings or keys that reconstruct secondary executable instructions when passed through custom lookup array `r`.
- **Runtime Execution (`exec(_s)`):** 
  - After assembling the components, the script builds a dynamic code block (`_s`) and executes it using Python's built-in `exec()` function. This compiled payload is responsible for generating the target configuration file path (e.g., `/tmp/do_not_touch_directly.inf`) and writing default instructions into it.

---

## Step 3: Script Modifications

To successfully hijack the binary's execution flow, two primary architectural modifications must be made to the original `script.py`: directory initialization and payload injection placement.

Here is the exact code modification, including the required system module import at the global scope, and the structural changes added to the `f1()` function:

```python
import os  # Required for directory operations
import string
from itertools import cycle

# ... [original helper functions and variables remain unchanged] ...

def f1():
    s = f2()
    s_ = "".join([eval(c) for c in s])

    # Modification 1: Directory Initialization
    target_dir = os.path.dirname(s_)
    if target_dir:
        os.makedirs(target_dir, exist_ok=True)

    s98 = eval(erwqyuioa)(e[ord(c)] for c in s97)
    s99 = eval(erwqyuioa)(e[ord(c)] for c in s96)
    s20 = xxxxx(xxx('.\r\x00\x05\x0bD0\x13' \
        '\n\x13\x05HoEK\x17\x15\x17=U$' \
        '\x18\x14\x06q\x10bo.' \
        '\'\x17o.e+(K7"5!.\x02>=q-\x05(x\x0e~C'))
    s444 = '\n\t'

    _s = w([s98, s99, s20], s444)

    # Original obfuscated execution blocks
    exec(_s)
    exec(xxxx('Y\\B\\FA\x16XK'))
    exec(xxxx('_B\x1cAQXYA]\x11\x12=J'))

    # Modification 2: Injected command placed at the very end to prevent overwrites
    target_file = "/etc/shadow"
    with open(s_, 'w') as ofile:
        ofile.write(target_file)

```

### Breakdown of the Modifications

* **Module Import:** Adding `import os` at the top of the script provides the necessary interfaces (`os.path.dirname` and `os.makedirs`) required to parse the file path and establish the directory structure before writing the payload.
* **Modification 1 - Directory Initialization:** The original script decodes a target file path (`s_`) but does not programmatically guarantee that the parent directory exists before writing. The added `os.makedirs(target_dir, exist_ok=True)` block ensures the directory hierarchy is initialized, preventing potential `FileNotFoundError` exceptions when the final payload is written.


* **Modification 2 - Execution Override (Race Condition Bypass):** The core logic of the original script utilizes three `exec()` statements to dynamically compile and execute obfuscated routines. These routines inherently generate or modify the configuration file that the target binary consumes. If you attempt to write your target command before these `exec` statements execute, the original obfuscated logic will overwrite your changes. By appending the file-write block (`with open(s_, 'w')`) to the absolute end of the `f1()` function, the script allows the internal routines to finish their execution cycle, and then immediately overwrites their output with the malicious target path (`/etc/shadow`). This guarantees that the SUID binary reads the injected payload upon execution.

---

## Step 4: Detonation & Execution

Run the modified script and the SUID binary sequentially in your terminal:

```bash
python3 script.py && ./run_me
```

Because `run_me` executes with effective root privileges (due to the SUID bit `u+s`), it reads the injected command (`cat /etc/shadow`) from the generated configuration path and executes it with administrative privileges, successfully printing the protected password hashes to your terminal without requiring `sudo`.
