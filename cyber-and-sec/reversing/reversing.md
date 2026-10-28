

## Summary

Comprehensive analysis of x86-64 assembly binaries, covering high-level 
functionality, instruction syntax, M/o/Vfuscator obfuscation, singly linked 
list implementations, static/dynamic password extraction techniques, and 
handling packed binaries.

## Exercise 1:

## Program Functionality Identification

Both provided assembly snippets evaluate to the same logical output but 
employ different standard library functions and register allocation strategies.

* **Program 1:** Validates command-line arguments and utilizes `atoi` to 
  parse the first argument (`argv[1]`) into an integer. It calculates the sum 
  of all integers from `0` to `N-1` by utilizing stack-allocated memory space 
  (`[rbp-4]` and `[rbp-8]`) for the loop accumulator and iterator variables.
* **Program 2:** Calculates the identical arithmetic progression (from `0` to 
  `N-1`) but employs `strtol` for string-to-integer parsing. It demonstrates 
  an optimized loop structure by strictly utilizing CPU registers (`esi` for 
  the accumulator, `eax` for the iterator) instead of local stack memory, 
  significantly reducing memory read/write overhead during the loop execution.

## Assembly Syntax and Memory Operations

* **Assembly Syntax:** Intel syntax. Identifiable by the `instruction 
  destination, source` operand order, explicit memory size definitions, and 
  the omission of register or immediate prefixes (e.g., `%`, `$`).
* **`DWORD PTR`:** A size directive specifying a 32-bit (4-byte) memory operand 
  width. Alternative standard directives include `BYTE PTR` (8-bit), `WORD PTR` 
  (16-bit), and `QWORD PTR` (64-bit).
* **`[rsi + 8]`:** Computes and accesses an effective memory address. In a 
  standard C entry point, `rsi` serves as the base pointer holding the `argv` 
  array base address. The `+ 8` applies an 8-byte offset (advancing one 64-bit 
  pointer address), and `[]` dereferences the location to fetch `argv[1]`.
* **`xor eax, eax`:** A bitwise exclusive OR operation that clears the `eax` 
  register to `0`. It is utilized in place of `mov eax, 0` because it compiles 
  to fewer machine code bytes and explicitly breaks dependency chains within 
  the processor pipeline, preventing execution stalls.


## Exercise 2: M/o/Vfuscator Binary Analysis (`summer_mov`)

**Overview:** This exercise walks through the reverse engineering of an 
obfuscated ELF binary. The objective is to identify its logical functionality, 
analyze its abnormal instruction set, and understand the architectural 
mechanics of achieving Turing Completeness utilizing solely memory data 
transfers.

**Results** When executing the binary with varying inputs, it clearly 
calculates the sum of all integers from `0` to `N-1` (e.g., an input of `100` 
yields `4950`). However, the program exhibits abnormal latency and hanging 
behavior when no arguments are provided, indicating that the underlying 
machine code is executing simulated operations rather than native hardware 
instructions. Disassembling the binary explains this performance hit: the 
`.text` section contains thousands of `mov` instructions but completely lacks 
standard Arithmetic Logic Unit (ALU) operations like `add` or `sub` and 
standard control flow logic. Since the CPU isn't actively computing the sums, 
inspecting the symbol table reveals the true mechanism—the binary relies on 
a massively inflated `.data` segment containing over 130,000 embedded symbols 
(primarily massive static arrays named `alu_add16`), proving that dynamic 
mathematical computation has been entirely replaced by static memory lookups.

### Architectural Breakdown

* **Can you figure out what it does?**
  The program parses a single command-line integer argument `N` and calculates 
  the arithmetic sum of the sequence from `0` to `N-1`.
* **What's weird about it?**
  The compiled execution segment contains absolutely no standard arithmetic 
  instructions (e.g., `add`, `sub`, `xor`, `cmp`) or conditional branching 
  instructions. The entire program operates sequentially using almost 
  exclusively the `mov` instruction, resulting in a massively inflated 
  `.data` segment containing over 132,000 static symbols.
* **What does that teach you?**
  It practically demonstrates that the x86 `mov` instruction is Turing 
  Complete. Any computational logic, algorithmic state machine, or programmatic 
  flow can be implemented purely through memory data transfers without 
  requiring the processor's ALU.
* **How is this possible?**
* **Arithmetic Execution (ALU Bypass):** Mathematical operations are mapped 
  to massive pre-computed memory lookup tables embedded in the binary (the 
  `alu_add16` structures). To "add" two numbers, the CPU calculates a memory 
  offset based on the operands and executes a `mov` to fetch the 
  pre-calculated result directly from RAM.
* **Control Flow (Pointer Aliasing):** Conditional branching (if/else loops) 
  is simulated without hardware jumps. The execution path remains strictly 
  linear. When a conditional block evaluates to false, the target destination 
  pointer is dynamically switched to a dummy memory address (a "bit bucket"). 
  The instructions still execute, but their outputs are written to junk memory, 
  silently discarding the invalid state and allowing sequential execution to 
  continue uninterrupted.


## Exercise 3: SLL Assembly Analysis

**Overview:** This exercise involves analyzing the assembly of a Singly 
Linked List (SLL) implementation by comparing raw compiler-generated assembly 
with the fully linked, disassembled binary.

### Generating Assembly and Compilation
*   **Raw Assembly Generation:** Running `gcc -S slist.c -o sll.s` instructs 
    the compiler to produce raw assembly code without linking, saving it to 
    a `.s` file.
*   **Compilation and Disassembly:** Running `gcc slist.c -o sll` compiles 
    and links the code into an executable binary. Running 
    `objdump -D sll > sll_dsm` dumps the disassembled machine code.

### Comparing the `main` Function
*   **In the Raw Assembly (`sll.s`):** The `main` function is simply declared 
    under the label `main:`. It strictly contains the logic from the C code, 
    utilizing relative addressing and symbolic names for standard calls 
    (e.g., `call ListCreate`).
*   **In the Disassembled Binary (`sll_dsm`):** The `main` function is mapped 
    to an absolute virtual memory address (`000000000000184b <main>:`) within 
    the `.text` segment. 
*   **Why they differ:** The raw assembly is just a translation of your 
    specific source code. The disassembled binary has been processed by the 
    linker, which merges your code with system libraries and startup routines. 
    The linker replaces symbolic labels with actual memory addresses and adds 
    necessary overhead sections like `_start`, `_init`, and `.plt` / `.got` 
    tables to handle dynamic linking for external calls like `malloc` and `free`.

### Analysis of the SLL `ListInsert` Method
The `ListInsert` function executes a textbook x86-64 stack frame and dynamic 
allocation flow:
*   **Prologue:** Located at address `00000000000012f0 <ListInsert>:` in the 
    binary, the function initializes its stack frame (`push %rbp`, 
    `mov %rsp,%rbp`). It then allocates 32 bytes (`sub $0x20,%rsp`) for local 
    variables.
*   **Calling Convention:** Arguments are received via registers `%rdi` and 
    `%rsi` (the list iterator and the data to insert) and immediately pushed 
    onto the stack.
*   **Memory Allocation:** The function prepares for dynamic allocation by 
    passing the required byte size (`$0x10`, or 16 bytes) into `%edi` and 
    invoking `call 10b0 <malloc@plt>` (or `call malloc@PLT`).
*   **Pointer Manipulation:** After checking that the allocation did not 
    return NULL, the function utilizes `%rax` and `%rdx` registers to modify 
    the list pointers. It links the new node into the list by directly 
    manipulating the `next` struct members, which are accessed dynamically in 
    memory using an 8-byte offset (e.g., `8(%rax)`). 
*   **Epilogue:** It finalizes operations, cleans up the local stack space using 
    the `leave` instruction, and returns control to the caller with `ret`.


## Exercise 4: Password Extraction Analysis

**Overview:** The objective of this exercise is to find a hidden password 
inside the provided binary and explore multiple reverse engineering techniques 
to achieve this. Because the binary does not employ encryption or obfuscation 
for the target string, the password can be extracted using both static and 
dynamic approaches.

### Method 1: Basic Static Analysis (`strings`)
The most straightforward approach for unencrypted binaries is extracting 
readable ASCII characters. By piping the output of `strings` to `grep`, the 
cleartext password is immediately revealed.

**Terminal Output:**
```bash
$ strings find_pass_opt | grep "password"
Please insert a password.
super_long_and_complicated_password

$ ./find_pass_opt super_long_and_complicated_password
this is my first secret
this is the second secret

```

### Method 2: Section Extraction (`objdump`)

Hardcoded strings are typically stored in the Read-Only Data (`.rodata`)
segment. Dumping this specific section provides the exact memory offsets and
hex values alongside the ASCII representation, bypassing the heuristic nature
of `strings`.

**Terminal Output:**

```bash
$ objdump -s -j .rodata find_pass_opt | grep "super" -A 4
 b5030 6c65642e 00000000 73757065 725f6c6f  led.....super_lo
 b5040 6e675f61 6e645f63 6f6d706c 69636174  ng_and_complicat
 b5050 65645f70 61737377 6f726400 00000000  ed_password.....
 b5060 74686973 20697320 6d792066 69727374  this is my first
 b5070 74686973 20697320 74686520 7365636f  this is the seco

```

### Method 3: Dynamic Analysis via Debugging (`gdb` / `pwndbg`)

Analyzing the binary's memory during execution requires bypassing a
"short-circuit" mechanism. The C++ `std::string` comparison first checks the
length of the input against the expected password length (35 characters). If
the length does not match, the program exits before executing the actual
string comparison (`memcmp`).

To defeat this, we supply a dummy payload of exactly 35 characters (e.g.,
35 'A's). This forces the program to proceed to the `memcmp` instruction. By
placing a breakpoint at `memcmp`, we can inspect the registers used for the
function's arguments (`RDI` and `RSI`) and extract the password directly
from allocated memory.

**Terminal Output:**

```gdb
pwndbg> break memcmp
pwndbg> run AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA

# The program halts at memcmp. We inspect the string pointers:
pwndbg> x/s $rdi
0x555555609038: "super_long_and_complicated_password"
pwndbg> x/s $rsi
0x7fffffffe1f5: "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"

```

### Alternative Methods

* **Library Tracing (`ltrace`):** By running
`ltrace ./find_pass_opt AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA`, the tracer hooks
into dynamic library calls. Since the length constraint is satisfied,
`ltrace` intercepts the `memcmp` call and prints the memory addresses of
both the dummy input and the expected password.
* **Decompilation (Ghidra / IDA Pro):** Loading the binary into a decompiler
translates the assembly back into Pseudo-C. This exposes the logic flow and
the `memcmp` comparison explicitly in the interface, revealing the hardcoded
password pointer without requiring execution.
* **Binary Patching:** Instead of finding the password, the binary's control
flow can be manipulated. By locating the conditional jump instruction
(e.g., `jne` or `je`) immediately following the password check, we can
overwrite the opcodes with `NOP` (`0x90`) instructions using a Hex Editor.
This neutralizes the validation logic, forcing the binary to dump its secrets
regardless of the provided input.

## Exercise 5: Packed Binary Password Extraction (UPX)

**Overview:** This exercise focuses on analyzing a packed binary where the
original executable code and strings are compressed or encapsulated inside
a runtime wrapper, preventing direct static analysis until it is unpacked.

### Question / Objective

Extract the hidden password from a packed binary (`find_pass_packed`) that
hides standard analysis tools like `strings` or `objdump` until its wrapper
is removed.

### Understanding Binary Packing

* **What is Packing?** Packing compresses and encrypts or obfuscates an
executable file to reduce its size and hinder reverse engineering. A small
stub loader is prepended to the binary; when executed, this stub unpacks
the real payload into memory and transfers control to it.
* **The UPX Format:** Ultimate Packer for Executables (UPX) is a popular
free, portable packer. Packed binaries typically exhibit missing or altered
section headers (e.g., standard `.rodata` or `.text` sections are replaced
by `.upx` compression sections).

### Unpacking the Binary

To restore the binary to its original, analyzable state, we must remove the
UPX wrapper using the decompression flag:

```bash
upx -d find_pass_packed -o find_pass_unpacked

```

This command decompresses the payload, restores the original section headers
and symbol tables, and writes out a clean, standard ELF binary.

### Password Discovery

Once unpacked into `find_pass_unpacked`, the binary is structurally identical
to a standard compiled executable. Consequently, password discovery is performed
in the exact same way as Exercise 4: using static analysis tools like `strings`,
section inspection with `objdump`, or dynamic debugging with `gdb` / `pwndbg`
once the length check is satisfied.


## Exercise 6: Additional Assembly Instructions Encountered

**Overview:** During the analysis of our previous exercises and disassembled 
binaries, we encountered several real-world x86-64 assembly instructions that 
were not explicitly listed in the base worksheet.

### Identified Instructions and Their Purpose

* **`cltq` (Convert Long to Quadword):**
  Sign-extends the 32-bit register `eax` into the 64-bit register `rax`. 
  It is commonly generated by GCC when converting an integer loop counter 
  or index into a 64-bit pointer or memory offset.
* **`endbr64` (End Branch 64-bit):**
  An Intel CET (Control-flow Enforcement Technology) security instruction 
  placed at function entry points (such as `main` or `ListInsert`). It acts 
  as a valid target for indirect branch/function calls to mitigate ROP 
  (Return-Oriented Programming) exploitation.
* **`repz ret` (Repeated Return):**
  A compatibility-optimized return instruction where a `rep` prefix is 
  prepended to `ret`. Compilers like GCC utilize this to prevent branch 
  prediction penalties on specific older processor microarchitectures.

