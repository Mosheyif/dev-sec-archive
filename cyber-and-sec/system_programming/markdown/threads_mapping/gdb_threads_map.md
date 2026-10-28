# A Practical Guide to Multi-Threaded Memory Mapping in Linux

This guide demonstrates how to inspect and map the memory address space of a
multi-threaded C program using GDB. We analyze where different memory segments
(Stacks, Heap, Global data, and Thread Local Storage) reside within the virtual
memory of a running process.

## Prerequisites

- A compiled C program using `pthreads` with debugging symbols
  (`gcc -g -pthread`).
- The program should spawn multiple threads and have examples of global
  variables, heap allocations (`malloc`), and a TLS variable (`__thread`).

Program location: `system_programming/src/threads_map.c`

---

## Step-by-Step GDB Analysis

### Step 1: Setting Breakpoints to Freeze the State

To accurately map the memory, halt the program when all threads are actively
running.

1. Launch GDB: `gdb ./threads_map.out`
2. Set breakpoints inside your thread routines and `main`:

   ```gdb
   (gdb) break main
   (gdb) break PrintHello
   (gdb) break PrintBye
   ```

3. Run the program: `(gdb) run`
4. Continue execution until you hit a breakpoint inside a spawned thread:
   `(gdb) c`

*At this point, all threads are spawned and the process state is completely
frozen.*

### Step 2: Extracting the Memory Addresses

Use GDB's `thread apply all` command to gather data from all threads
simultaneously.

**1. Hardware Stack Pointers (RSP)**

```gdb
(gdb) thread apply all print /x $rsp
Thread 3: $1 = 0x7ffff73fdea0
Thread 2: $2 = 0x7ffff7bfeea0
Thread 1: $3 = 0x7fffffffdad8
```

**2. Thread Local Storage (TLS)**

Print the address of a `__thread` variable — each thread gets a unique address:

```gdb
(gdb) thread apply all print &tls_var
Thread 3: $4 = 0x7ffff73fe6bc
Thread 2: $5 = 0x7ffff7bff6bc
Thread 1: $6 = 0x7ffff7fa073c
```

**3. Shared Resources (Global & Heap)**

These are shared, so addresses are identical across all threads:

```gdb
(gdb) thread apply all print &global_var
$7, $8, $9 = 0x555555559014

(gdb) thread apply 2-3 print args  # Assuming 'args' contains the heap pointer
$10, $11 = 0x55555555a2a0
```

### Step 3: Correlating with the OS Memory Map

Run `info proc mappings` to view the Linux kernel's memory map and match
addresses to their respective blocks.

```text
Mapped address spaces:
  Start Addr           End Addr       Size     Offset  Perms  objfile
  0x555555559000     0x55555555a000     0x1000     0x4000  rw-p   threads_map.out
  0x55555555a000     0x55555557b000    0x21000        0x0  rw-p   [heap]
  0x7ffff6bff000     0x7ffff73ff000   0x800000        0x0  rw-p
  0x7ffff7400000     0x7ffff7c00000   0x800000        0x0  rw-p
  0x7ffffffde000     0x7ffffffff000    0x21000        0x0  rw-p   [stack]
```

**The Breakdown:**

- **Global Variable** (`0x555555559014`): Falls into the `.data` segment
  (`0x555555559000` – `0x55555555a000`).
- **Heap** (`0x55555555a2a0`): Falls inside the dynamically grown `[heap]`
  segment (`0x55555555a000` – `0x55555557b000`).
- **Main Thread Stack** (`0x7fffffffdad8`): Fits inside the OS-designated
  `[stack]` region at the top of user-space memory.
- **Spawned Thread Stacks**: Thread 3 (`0x7ffff73fdea0`) and Thread 2
  (`0x7ffff7bfeea0`) fall into the two anonymous 8MB blocks in the middle.

---

## Core Concepts Explained

### 1. Where Is the Stack of Each Thread Located?

- **Main Thread:** Its stack is allocated automatically by the OS loader when
  the process starts. Resides in high memory, explicitly labeled `[stack]`.
- **Spawned Threads (pthreads):** Their stacks are allocated dynamically by the
  `pthreads` library using `mmap()`, residing in the memory mapping segment
  (between the heap and the main stack). Each spawned thread gets a default
  8MB block (`0x800000` bytes).

### 2. Can a Thread Access Another Thread's Stack?

**Yes.** Memory isolation exists only at the *process* level, not the *thread*
level. Because all threads share the same virtual address space, Thread A can
freely read from or write to Thread B's stack memory if given a valid pointer.
This shared nature is what makes threading powerful — and the root cause of race
conditions.

### 3. Where Does TLS (Thread Local Storage) Reside?

Variables marked with `__thread` are global in scope but private in instance —
every thread gets its own copy. For spawned threads, TLS addresses
(`0x7ffff73fe6bc` and `0x7ffff7bff6bc`) are located at the very top of their
respective 8MB stack mappings. The `pthreads` library places the Thread Control
Block (TCB) and TLS segment directly adjacent to the top of the thread's mapped
stack, allowing quick access to private global variables without conflicts.
