
# Comprehensive Guide: 5 Vectors to Crash a Linux Binary

This guide details five distinct methods to crash the `make_this_crash` binary, starting with the classic buffer overflow and progressing through static patching, dynamic linking interception, and environmental starvation. Every assumption relies on empirical verification using standard Linux reversing utilities.

---

## 1. Buffer Overflow (Stack Smashing)

**Concept:** The binary securely fails via `SIGABRT` when a stack buffer overrun overwrites the Stack Canary prior to the function's return sequence.

**Step 1: Information Gathering**
Before attempting a buffer overflow, verify the binary's memory protections using `checksec` (part of `pwntools`).

```bash
$ checksec --file=./make_this_crash
    Arch:     amd64-64-little
    RELRO:    Partial RELRO
    Stack:    Canary found
    NX:       NX enabled
    PIE:      No PIE

```

The output explicitly confirms `Canary found`, meaning a traditional control-flow hijack will fail if the canary is corrupted. Dynamic tracing identifies the exact environment variable being pulled into memory.

```bash
$ ltrace ./make_this_crash
getenv("XDG_CURRENT_DESKTOP") = "ubuntu:GNOME"
sprintf("The received string was: ubuntu:"..., "The received string was: %s", "ubuntu:GNOME") = 37

```

Static disassembly reveals the exact layout of the stack frame.

```bash
$ objdump -d ./make_this_crash | grep -A 15 "<main>:"
40119e:    48 83 ec 60             sub    $0x60,%rsp             # Total stack allocation
4011a2:    64 48 8b 04 25 28 00    mov    %fs:0x28,%rax          # Canary setup
4011ab:    48 89 45 f8             mov    %rax,-0x8(%rbp)        # Canary stored at RBP-0x8
4011c5:    48 8d 45 b0             lea    -0x50(%rbp),%rax       # Buffer starts at RBP-0x50

```

**Step 2: Execution Mechanics**
The buffer begins at `RBP-0x50` and the canary is located at `RBP-0x8`. This creates a 72-byte gap (`0x50 - 0x8`). The hardcoded `sprintf` string `"The received string was: "` consumes 25 bytes. Subtracting 25 from 72 leaves exactly 47 bytes for user input. Injecting 48 characters pushes the total size to 73 bytes, which, along with the null terminator, spills directly into the canary's memory space.

```bash
$ env XDG_CURRENT_DESKTOP=$(python3 -c 'print("A"*48)') ./make_this_crash
The received string was: AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
*** stack smashing detected ***: terminated
Aborted (core dumped)

```

* **Advantages:** Exploits the native logic of the application without external tools or file modifications.


* **Disadvantages:** Prevented from becoming a control-flow hijack (like a ROP chain) due to the canary validation in the epilogue (`__stack_chk_fail`), which safely aborts the program.



---

## 2. Hardware Exception (Instruction Patching)

**Concept:** Modifying the executable on disk to insert an invalid CPU opcode (`ud2`), causing the processor to fault immediately when attempting to read the instruction.

**Step 1: Information Gathering**
To patch a specific instruction, you must map the Virtual Memory Address (VMA) back to its physical offset on the disk. First, find the VMA of `main`.

```bash
$ objdump -M intel -d ./make_this_crash | grep "<main>:"
0000000000401196 <main>:

```

Do not guess the load base address. You must verify if Position Independent Executable (PIE) is disabled. Run `file ./make_this_crash`; if it outputs `executable` instead of `shared object`, it is No-PIE. You can definitively extract the load base address by reading the ELF Program Headers and locating the `LOAD` segment with `R E` (Read/Execute) permissions:

```bash
$ readelf -l ./make_this_crash | grep -A 1 "LOAD"
  Type           Offset             VirtAddr           PhysAddr
  LOAD           0x0000000000000000 0x0000000000400000 0x0000000000400000

```

This confirms the executable is mapped to a static base of `0x400000`. Subtracting this base from the VMA yields the exact physical file offset: `0x401196 - 0x400000 = 0x1196`.

**Step 2: Script Mechanics (`patch_sigill.py`)**
The Python script opens the binary in raw binary mode (`r+b`) to allow precise byte manipulation without corrupting the file structure. It uses `f.seek(0x1196)` to jump directly to the first instruction of `main`. By writing `\x0f\x0b`, it replaces the original opcodes with the x86-64 `ud2` (Undefined Instruction 2) opcode.

**Step 3: Execution**

```bash
$ python3 patch_sigill.py
Enter 1 to destroy and 0 to restore
1
[+] Binary permanently patched with SIGILL.

$ ./make_this_crash
Illegal instruction (core dumped)

# [OPTIONAL] Restore the original instruction to explore other exploit vectors. 
# If the binary is not repaired, subsequent attacks will fail due to the immediate hardware fault.
$ python3 patch_sigill.py
Enter 1 to destroy and 0 to restore
0
[+] Code segment repaired. Binary execution restored.

$ ./make_this_crash
The received string was: ubuntu:GNOME
```

* **Advantages:** Permanent DoS. It does not rely on the execution environment.


* **Disadvantages:** Modifies the file on disk, altering its hash and making the sabotage easily detectable by integrity checkers.



---

## 3. Format Corruption (ELF Header Patching)

**Concept:** The Linux kernel (`execve` syscall) expects executables to start with the magic bytes `\x7fELF`. Overwriting these bytes causes the OS to reject the file before loading it into memory.

**Step 1: Information Gathering**
Do not assume the file format. You can verify the ELF magic number located at offset `0x0` using a hex dumper or by reading the ELF header.

```bash
$ xxd -l 4 ./make_this_crash
00000000: 7f45 4c46                                .ELF

$ readelf -h ./make_this_crash | grep "Magic"
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00

```

**Step 2: Execution Mechanics**
Instead of writing a custom script, the `dd` (Data Duplicator) utility handles raw byte writes natively.

* `bs=1`: Sets the block size to 1 byte.
* `seek=0`: Starts writing at the very beginning of the file.
* `count=4`: Ensures exactly 4 bytes are written.
* `conv=notrunc`: Crucially prevents `dd` from truncating (deleting) the remainder of the binary.

```bash
$ printf "AAAA" | dd of=./make_this_crash bs=1 seek=0 count=4 conv=notrunc
4+0 records in
4+0 records out
4 bytes copied, 0.000123 s, 32.5 kB/s

$ ./make_this_crash
bash: ./make_this_crash: cannot execute binary file: Exec format error

# [OPTIONAL] Restore the ELF magic bytes to explore other exploit vectors.
# If the binary is not repaired, subsequent attacks will fail because the OS will refuse to load it.
$ printf "\x7fELF" | dd of=./make_this_crash bs=1 seek=0 count=4 conv=notrunc

```

* **Advantages:** The strongest form of static DoS. The binary cannot even reach the CPU or memory.


* **Disadvantages:** Like instruction patching, it alters the file permanently on disk.



---

## 4. Dynamic Linker Hijacking (PLT/GOT Hooking)

**Concept:** Linux dynamically loads shared libraries (like `libc`) at runtime. Using the `LD_PRELOAD` environment variable, we can load our own malicious library *first*, intercepting calls to standard functions.

**Step 1: Information Gathering**
Confirm the binary is dynamically linked and utilizes the target function.

```bash
$ ldd ./make_this_crash
    libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6

```

To verify that dynamic hijacking is permitted, use `checksec` to ensure the binary is compiled with `Partial RELRO` (which leaves the Global Offset Table writable) rather than `Full RELRO` (which makes the GOT read-only before execution starts).

```bash
$ checksec --file=./make_this_crash | grep "RELRO"
    RELRO:    Partial RELRO

```

**Step 2: Script Mechanics (`hijack.c`)**
The provided C file redefines the exact signature of `char *getenv(const char *name)`. Because our object is preloaded, the dynamic linker resolves our function instead of the one in `libc`. The payload forces a hardware memory violation by initializing an integer pointer to `NULL` (`0x0`) and immediately attempting to write the value `1337` to that unmapped address.

**Step 3: Execution**
Compile the C file into a Position Independent (`-fPIC`) Shared Object (`-shared`). Then, execute the binary with the library injected into the loader path.

```bash
$ gcc -shared -fPIC hijack.c -o hijack.so
$ LD_PRELOAD=./hijack.so ./make_this_crash
Segmentation fault (core dumped)

```

* **Advantages:** Extremely stealthy. Leaves the original binary completely unmodified on disk.


* **Disadvantages:** The Linux kernel strictly ignores `LD_PRELOAD` for SUID (privileged) binaries to prevent privilege escalation.



---

## 5. Virtual Memory Starvation (Loader Abort)

**Concept:** Before a program runs, the dynamic linker (`ld-linux`) uses the `mmap` system call to allocate memory for libraries. If we restrict the allowed memory, the loader fails.

**Step 1: Information Gathering**
You can empirically verify that dynamically linked C binaries require massive memory allocations before `main` is reached by tracing the `mmap` syscalls.

```bash
$ strace -e mmap ./make_this_crash
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7fae4b1cc000
mmap(NULL, 2217216, PROT_READ, MAP_PRIVATE|MAP_DENYWRITE, 3, 0) = 0x7fae4aee4000
...

```

This output proves the loader maps over 2 Megabytes of memory during initialization. Constraining the Virtual Address Space below this threshold guarantees a crash.

**Step 2: Script Mechanics (`starve.py`)**
The Python script utilizes `os.fork()` to isolate the execution. In the child process, it calls `resource.setrlimit()` to impose a strict 1,048,576 byte (1MB) ceiling on the `RLIMIT_AS` (Address Space). The script then uses `os.execve()` to replace the child process image with the target binary. The parent process safely waits, ensuring the terminal itself does not crash.

**Step 3: Execution**

```bash
$ python3 starve.py
./make_this_crash: error while loading shared libraries: libc.so.6: failed to map segment from shared object
[*] Child process crashed safely. Shell remains intact.

```

* **Advantages:** Bypasses all application-level logic and crashes during the OS preparation phase. Safe for the parent shell.


* **Disadvantages:** Requires an external wrapper (Python/C) to configure the environment constraints before execution.
