# x86-64 Architecture & Low-Level Assembly Practice Worksheet

This comprehensive engineering worksheet documents the internal mechanics of x86-64 systems, covering software compilation and interpretation pipelines, instruction execution cycles, virtual memory layout, pure assembly programming (NASM/Linux x86-64 ABI), and binary inspection/shellcode extraction.

---

## 1. Draw a UML Diagram of the Build Process

```text
[Source File (*.c)]
            |
            v
+-----------------------+
|    Preprocessor       |  <-- Inlines headers, expands macros, strips comments
+-----------------------+
            |
            | (Translation Unit / Preprocessed code: *.i)
            v
+-----------------------+
|      Compiler         |  <-- AST generation, Optimization, Instruction emission
+-----------------------+
            |
            | (Assembly code: *.s / *.asm)
            v
+-----------------------+
|      Assembler        |  <-- Translates mnemonics to machine opcodes
+-----------------------+
            |
            | (Relocatable Object File: *.o)
            v
+-----------------------+      [External Libraries (*.a, *.so)]
|       Linker          |  <-- Symbol resolution, Address binding, Relocation
+-----------------------+
            |
            | (Executable Binary: ELF / PE)
            v
   [Executable File]---
```


## 2. Abstraction Layers: Python `print('Hello World')` Pipeline

Demonstrate the transformation of a simple Python statement through computing abstraction layers, down to the raw binary hardware execution.

**1. Interpreted Language (Python)**
The high-level interpreted script representation.
```python
print('Hello World')

```

**2. High Level Language (C / CPython Implementation)**
Under the hood, the Python interpreter is typically written in C. It executes a C function that interfaces with the OS.

```c
puts("Hello World");

```

**3. Low Level Language (x86-64 Assembly)**
The C compiler translates the high-level function into architectural-specific assembly instructions to invoke a system call.

```nasm
mov eax, 1      ; syscall number for sys_write
mov edi, 1      ; file descriptor 1 (stdout)
mov rsi, msg    ; pointer to the string "Hello World"
mov edx, 12     ; string length
syscall         ; trigger kernel interrupt

```

**4. Hex Machine Code**
The assembler encodes the assembly mnemonics into exact hexadecimal machine opcodes (showing the first few batches).

```text
b8 01 00 00 00  (mov eax, 1)
bf 01 00 00 00  (mov edi, 1)

```

**5. Binary Machine Code**
The processor executes the raw bitwise binary representations of those hex opcodes (showing the first few batches).

```text
10111000 00000001 00000000 00000000 00000000
10111111 00000001 00000000 00000000 00000000

```


## 3. Instruction Cycle: `add rax, rbx` (Fetch-Decode-Execute-Store)

Illustrate the four-stage microarchitectural lifecycle (Fetch, Decode, Execute, Store) of a standard 64-bit register-to-register addition instruction.

**Instruction:** `add rax, rbx` (Adds the value in `rbx` to `rax` and stores the result in `rax`).

**1. Fetch Stage**
* The CPU's Control Unit requests the next instruction from memory.
* The memory address currently held in the Instruction Pointer (`RIP`) is placed on the Address Bus.
* The memory (or L1 Instruction Cache) returns the opcode bytes for `add rax, rbx` (e.g., `48 01 d8`) across the Data Bus.
* These bytes are latched into the CPU's Instruction Register (IR).
* The `RIP` is incremented to point to the next instruction in memory.

**2. Decode Stage**
* The Instruction Decoder analyzes the raw opcode in the Instruction Register.
* It determines the operation is an arithmetic addition (`add`).
* It identifies the specific operands: the 64-bit destination register `rax` and the 64-bit source register `rbx`.
* The Control Unit configures the internal data paths to route the current values of these registers into the Arithmetic Logic Unit (ALU).

**3. Execute Stage**
* The actual numerical values stored inside `rax` and `rbx` are fed into the inputs of the ALU.
* The ALU performs the binary arithmetic addition of the two 64-bit integers.

**4. Store (Write-back) Stage**
* The ALU outputs the final computed sum.
* This result is routed via the internal bus and written directly back into the destination register (`rax`), overwriting its previous value.
* The CPU updates the `RFLAGS` register (Condition Codes) based on the arithmetic result (e.g., setting the Zero Flag `ZF` if the sum is 0, the Carry Flag `CF` if an unsigned overflow occurred, or `OF` for signed overflow).

## 4. RAM Segments & Virtual Memory Layout

Illustrate the virtual memory layout in user space for a standard operating system process, detailing the division into specific segments (Stack, Heap, BSS, Data, Text).


**Memory Layout Diagram (High to Low Memory):**
```text
+-------------------------+  <-- High Memory Addresses (e.g., 0x7FFFFFFFFFFF)
|      Kernel Space       |      (Inaccessible to user process)
+-------------------------+
|         Stack           |  <-- Grows downwards (↓)
| (Local vars, arguments) |
+-------------------------+
|           |             |
|           v             |
|                         |
|           ^             |
|           |             |
+-------------------------+
| Memory Mapping (mmap)   |  <-- Shared libraries (libc), dynamically loaded modules
+-------------------------+
|           ^             |
|           |             |
+-------------------------+
|          Heap           |  <-- Grows upwards (↑)
|  (Dynamic Allocation)   |
+-------------------------+
|      .bss Segment       |  <-- Uninitialized global/static variables (Zeroed)
+-------------------------+
|     .data Segment       |  <-- Initialized global/static variables
+-------------------------+
|     .text Segment       |  <-- Executable code / Machine instructions (Read-Only)
+-------------------------+  <-- Low Memory Addresses (e.g., 0x00400000)

```

**Segment Mechanics:**

* **Stack:** Manages the control flow of the program. Stores local variables, return addresses, and function parameters. The stack grows downwards (towards lower addresses). Permissions: Read/Write (RW).
* **Heap:** Used for dynamic memory allocation at runtime (e.g., via `malloc` or `new`). Grows upwards (towards higher addresses). Permissions: Read/Write (RW).
* **BSS (Block Started by Symbol):** Contains uninitialized global and static variables. This memory is zeroed out by the OS upon program load. Permissions: Read/Write (RW).
* **Data:** Contains explicitly initialized global and static variables (like global strings). Permissions: Read/Write (RW).
* **Text (Code):** Contains the actual machine code instructions. This segment is protected and marked as read and execute only to prevent code overwriting. Permissions: Read/Execute (RX).

The strict separation of segments allows the operating system to enforce hardware-level security measures, such as the W^X (Write XOR Execute) policy. This ensures that memory areas that are writable (like the Stack and Heap) cannot be executed, effectively mitigating common exploits like stack-based buffer overflows and shellcode injection.


## General Assembly Compilation & Execution Pipeline

### Overview
Unlike high-level languages that rely on complex runtimes or virtual machines, pure assembly code for Linux is assembled directly into raw machine code and linked into an ELF executable. This exercise outlines the standard toolchain commands (`nasm` and `ld`) used to build and execute any x86-64 assembly program (such as `ex5_hello.asm`).

### Compilation & Linking Instructions
To transform a raw assembly source file into a runnable 64-bit binary, execute the following commands in the terminal:

```bash
# 1. Assemble the source code into a 64-bit ELF relocatable object file
nasm -f elf64 source.asm -o source.o

# 2. Link the object file to resolve symbols and generate the final executable binary
ld source.o -o program
```
---


## 8. Shellcode Extraction & Pwntools Integration

**Step 1: Install Pwntools (Virtual Environment Method)**
To avoid system package conflicts (such as the "externally-managed-environment" error on modern Linux distributions), set up a Python virtual environment before installing the Pwntools framework:

```bash
python3 -m venv venv
source venv/bin/activate
pip install pwntools

```

**Step 2: Compile the Assembly Program**
Assemble and link the `ex5_hello.asm` file to create an executable binary:

```bash
nasm -f elf64 ex5_hello.asm -o ex5_hello.o
ld ex5_hello.o -o ex5_hello

```

**Step 3: Extract Shellcode (CLI Method)**
Use `objcopy` to extract the executable `.text` section into a raw binary file, and then use `hexdump` to format it directly into a shellcode string:

```bash
# Extract the .text section as raw binary
objcopy -O binary -j .text ex5_hello shellcode.bin

# Format and print the raw binary as a shellcode string
hexdump -v -e '"\\""x" 1/1 "%02x" ""' shellcode.bin; echo

```

**Step 4: Output & Analysis**
Running the extraction commands on the compiled `ex5_hello` binary produces the following shellcode string:

```text
\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\x48\xbe\x00\x20\x40\x00\x00\x00\x00\x00\xba\x0d\x00\x00\x00\x0f\x05\xb8\x3c\x00\x00\x00\xbf\x00\x00\x00\x00\x0f\x05

```

**Understanding the Shellcode:**
This string is the exact hexadecimal representation of the machine instructions (opcodes) generated from the assembly code. For example:

* `\xb8\x01\x00\x00\x00` translates to `mov eax, 1` (Preparing the `sys_write` syscall).
* `\xbf\x01\x00\x00\x00` translates to `mov edi, 1` (Setting standard output).
* `\x0f\x05` translates to `syscall`.

**Important Note on Position Independent Code (PIC):**
Notice the sequence `\x48\xbe\x00\x20\x40\x00\x00\x00\x00\x00` inside the output. This corresponds to the `mov rsi, msg` instruction. The memory address of the string (`0x402000`) is hardcoded directly into the machine code.
While this extraction exercise is successful, this specific output cannot be used as functional shellcode in a real-world exploit. True shellcode must be Position Independent (PIC). If injected into another process, the hardcoded address will be invalid, likely causing a segmentation fault. Real shellcode avoids the `.data` section entirely and dynamically pushes strings (like "Hello World!") directly onto the stack at runtime.

