

# Instruction Skip via Return Address Manipulation

## Overview
Control flow execution can be manually hijacked to bypass standard sequential processing by dynamically modifying the return address stored on the stack. By manipulating the stack frame during a function call, the instruction pointer can be redirected to skip specific variable assignments and jump directly to subroutine preparations.

## Core Mechanism
Under the 32-bit x86 architecture utilizing the `cdecl` calling convention, the caller sets up arguments and manages stack cleanup. When a `call` instruction is executed, the CPU pushes the address of the next sequential instruction (the Return Address) onto the stack. Upon entering the callee function, the base pointer (`EBP`) is pushed, placing the original Return Address at memory location `EBP + 4`. 

To safely bypass instructions, the exact machine code byte offset between the original Return Address and the target destination address must be calculated and added to this memory location. Note that this offset is highly volatile and varies across different operating systems, compiler versions, and architectures.

## Procedure: Offset Calculation

**1. Compilation**
Ensure consistent, unoptimized 32-bit machine code generation for precise offset calculation by compiling the source file with strict debugging and architecture flags:
```bash
gcc -Wall -Wextra -g3 -m32 -no-pie skip_instructions.c -o si

```

**2. Disassembly Analysis**
Extract the absolute memory addresses of the executed instructions using `objdump`:

```bash
objdump -d si | grep -A 15 "<main>:"

```

*Disassembly Output:*

```text
 80491d3:    e8 8e ff ff ff          call   8049166 <func>
 80491d8:    83 c4 10                add    $0x10,%esp              # Return Address (Caller stack cleanup)
 80491db:    c7 45 f4 04 00 00 00    movl   $0x4,-0xc(%ebp)         # Variable assignment (num = 4)
 80491e2:    83 ec 08                sub    $0x8,%esp               # Target Destination (printf stack allocation)
 80491e5:    ff 75 f4                push   -0xc(%ebp)              # printf argument 2 (num)
 80491e8:    8d 83 19 e0 ff ff       lea    -0x1fe7(%ebx),%eax      # printf argument 1 calculation (format string)
 80491ee:    50                      push   %eax                    # printf argument 1 push
 80491ef:    e8 4c fe ff ff          call   8049040 <printf@plt>    # Subroutine execution

```

* **Return Address:** The instruction immediately following the `call` to `func` is located at `0x80491d8`.
* **Target Destination:** The sequence initializing the `printf` subroutine begins at `0x80491e2`.
* **Offset Calculation:** Subtracting the Return Address from the Target Destination (`0x80491e2 - 0x80491d8`) yields `0xA` (10 bytes).

## Solutions

### Preferred Method: C Pointer Arithmetic

This method achieves the offset jump entirely within standard C syntax by leveraging argument memory structure. In a 32-bit stack frame, the first argument (`a`) is stored at `EBP + 8`. Subtracting 1 from the pointer of `a` (where `long` size is 4 bytes) shifts the address strictly back by 4 bytes to `EBP + 4`, targeting the Return Address directly.

```c
void func(long a, long b) {
    (void)b; 
    
    /* 
     * &a resolves to EBP + 8
     * &a - 1 resolves to EBP + 4 (Return Address memory space)
     * Dereferencing and adding 10 modifies the EIP execution flow
     */
    *(&a - 1) += 10;
    return; /* To return immediately without executing the rest of the fnction. */
}

```

### Alternative Method: Inline Assembly

This method uses direct hardware instructions to manipulate the stack pointer memory. It requires explicitly dereferencing the `EBP` register and adding the calculated byte offset.

```c
void func(long a, long b) {
    (void)a;
    (void)b;

    /* Add exactly 10 bytes to the memory address stored at EBP + 4 */
    __asm__ __volatile__ (
        "addl $10, 4(%%ebp);"
        :
        :
    );
    return; /* To return immediately without executing the rest of the fnction. */
}

```
