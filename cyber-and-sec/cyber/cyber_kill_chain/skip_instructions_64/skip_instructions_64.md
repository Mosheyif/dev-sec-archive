
# 64-Bit Control Flow Hijacking: Self-Contained Stack Pivoting & RBP Smuggling

## Overview

Control flow execution can be manually hijacked by modifying the return address stored on the stack frame. In 64-bit architectures, simply overwriting the return address often causes a crash due to strict stack alignment rules. Furthermore, to strictly adhere to lab constraints requiring all code to be added **exclusively inside `func()**`, we must implement a self-contained solution.

This guide explains how to allocate a safe fake stack locally within `func()`, smuggle the pivot address through the Base Pointer (`RBP`) to survive register clobbering, execute an uncalled function (`finish`) with proper stack alignment, and seamlessly restore the caller's hardware state using pure ROP.

## Procedure: The Self-Contained Local Pivot Solution

### 1. Compilation

Compile the source file using standard debugging flags. Disabling Position Independent Executables (PIE) makes memory addresses predictable for the lab environment.

```bash
gcc -Wall -Wextra -g3 -no-pie skip_instructions_64.c -o si

```

### 2. Implementation Analysis

This implementation creates a local array (`fake_stack`) entirely within `func()`, keeping the solution completely self-contained. It uses explicit `volatile` casting to prevent the compiler from stripping the exploit via Dead Store Elimination (DSE), and smuggles the stack pivot address inside `RBP` to protect it from being wiped out by library functions like `printf`.

```c
void func() {
    register void **rbp asm("rbp");
    
    /* 1. Allocate a local fake stack inside func() to obey self-contained rules */
    void *fake_stack[1024];

    /* 2. Harvest the caller's complete hardware state */
    void *orig_rbp = (void *)((volatile void **)rbp)[0]; // Caller's saved RBP
    void *orig_ret = (void *)((volatile void **)rbp)[1]; // Return address back to caller
    void *orig_rsp = (void *)(rbp + 2); // Caller's pristine stack pointer

    void *pivot_gadget;
    void *restore_gadget;
    
    /* Point to the middle of the local stack to allow safe downward growth */
    void **pivot_point = &fake_stack[512]; 

    /* 3. Generate pure ROP gadgets inline */
    __asm__ volatile (
        "lea 1f(%%rip), %0\n\t"
        "jmp 2f\n\t"
        
        /* GADGET A: Smuggled Stack Pivot */
        "1: mov %%rbp, %%rsp\n\t" // RBP contains our smuggled pivot_point
        "ret\n\t"               
        
        "2:\n\t"
        "lea 3f(%%rip), %1\n\t"
        "jmp 4f\n\t"
        
        /* GADGET B: Complete Hardware Restore */
        "3: pop %%rax\n\t"      
        "pop %%rcx\n\t"         
        "pop %%rbp\n\t"         
        "mov %%rax, %%rsp\n\t"  // Snap real RSP back into place
        "push %%rcx\n\t"        // Plant orig_ret back into execution path
        "ret\n\t"               // Clean return
        "4:"
        : "=r" (pivot_gadget), "=r" (restore_gadget)
    );

    /* 4. Build the ROP chain dynamically on the local fake stack */
    pivot_point[0] = (void *)finish;
    pivot_point[1] = restore_gadget;
    pivot_point[2] = orig_rsp;
    pivot_point[3] = orig_ret;
    pivot_point[4] = orig_rbp;

    /* 5. FORCE the compiler to smuggle the pivot pointer into saved RBP */
    ((volatile void **)rbp)[0] = pivot_point;

    /* 6. FORCE the compiler to hijack the return address */
    ((volatile void **)rbp)[1] = pivot_gadget;
}

```

### 3. Step-by-Step Execution Breakdown

1. **Local Allocation:** We declare `void *fake_stack[1024]` locally inside `func()`, ensuring all exploit infrastructure is self-contained within the required function boundaries.


2. **State Preservation:** We extract `orig_rbp`, `orig_ret`, and `orig_rsp` to ensure we can rebuild the exact CPU state later.


3. **The Smuggle:** We assign our local stack address to `*(rbp + 0)` using volatile casting. When `func` naturally executes its epilogue (`pop %rbp`), the CPU natively loads our pivot address into `%rbp`.


4. **The Pivot:** `func` executes `ret`, jumping to Gadget A. Gadget A moves the smuggled `%rbp` address into `%rsp`. The stack is now pivoted.


5. **Execution & Alignment:** The next `ret` natively pops `finish` into the instruction pointer. The stack is flawlessly aligned to a 16-byte boundary for vector instructions.


6. **The Restoration:** `finish` concludes and returns into Gadget B, which pulls our saved hardware state off the local array and reconstructs the active caller's exact stack frame.

---

## Position Independent Executable (PIE) Compatibility

If the binary is compiled *without* the `-no-pie` flag, it becomes a Position Independent Executable (PIE). This means the memory addresses of functions and code segments are no longer fixed absolute values; they are loaded at randomized offsets every time the program runs.

Despite this randomization, **this exploit continues to work flawlessly.** The solution was architected to be position-independent through two key mechanisms:

1. **RIP-Relative Addressing:** The inline assembly uses `lea 1f(%%rip), %0` to calculate gadget addresses relative to the current Instruction Pointer (`RIP`). Because the relative distance between instructions inside the function never changes (even if the entire function is shifted in memory), the gadget addresses are calculated perfectly.


2. **Dynamic Pointer Resolution:** By assigning `pivot_point[0] = (void *)finish;`, we rely on the C compiler to resolve the address of `finish` at runtime. The compiler handles the PIE offset tracking automatically.



Had we utilized fragile methods like hardcoded absolute memory addresses or offset jumps that do not leverage `RIP`, removing the `-no-pie` flag would have resulted in an immediate segmentation fault.

---

## Alternative Solutions & Their Weaknesses

During the development of this exploit, several easier alternative solutions were explored. While functional in simple labs, they introduce critical vulnerabilities or violate exercise constraints.

### Alternative 1: Hardcoded Offset (`+6` Byte Jump)

Instead of generating an inline gadget, the return address is overwritten by adding exactly 6 bytes to the original return address to jump to an existing `ret` instruction in the caller.

* **Mechanism:** `*(rbp + 1) = (char *)orig_ret + 6;`

* **Weakness (Fragility):** Assumes a very specific compiler instruction layout. Changes in compiler optimization flags (`-O1`, `-O2`) or architecture completely break the hardcoded calculation, causing the exploit to jump into the middle of an instruction and crash.



### Alternative 2: Dynamic Opcode Scan (`0xC3`)

To avoid hardcoding an offset, the code treats memory as a byte array and dynamically scans forward from the return address searching for `0xC3` (the `ret` opcode).

* **Mechanism:** `if (*ptr == 0xc3) { ret_gadget = ptr; break; }`

* **Weakness (False Positives):** The x86 instruction set is dense and unaligned. The hex byte `0xC3` frequently appears as part of memory addresses, immediate values, or offsets within other instructions. Hitting a false positive causes an immediate segmentation fault.



### Alternative 3: Prologue Skip (`+5` Byte Jump)

Hijacking execution directly to `finish + 5`. This skips the target function's `push %rbp` instruction, natively offsetting the 8-byte misalignment caused by the hijack.

* **Mechanism:** `*(rbp + 1) = (char *)finish + 5;`

* **Weakness (Compiler Specificity):** Relies heavily on the exact prologue size generated by the compiler (e.g., assuming a 4-byte `endbr64` instruction followed by a 1-byte `push %rbp`). If Control-flow Enforcement Technology (CET) is disabled, the offset breaks.


### Alternative 4: Basic Stack Shifting (No Pivot)

Building the ROP chain directly on the active stack by sliding the caller's original return address and saved registers upwards by 16 bytes.

* **Mechanism:** `*(rbp + 4) = caller_saved_rbp; *(rbp + 5) = libc_ret;`

* **Weakness (Stack Corruption):** Blindly overwriting memory slots deeper in the stack destroys local variables of the parent function. This only works if the caller (like `main`) has an entirely empty stack frame directly behind the base pointer.
