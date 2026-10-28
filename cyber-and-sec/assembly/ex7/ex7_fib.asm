; *********************************************************
; *
; * Description: Fibonacci sequence up to user input.
; * Author: Moshe Yifrah
; *
; *********************************************************

global _start

section .data
    ; Split string across two lines to maintain the 80-char limit strictly
    welcome_msg db 'Welcome! May your day be filled with immense joy '
                db 'and happiness! Please enter a max number: ', 0
    welcome_len equ $ - welcome_msg
    newline     db 0x0A

section .bss
    input_buf resb 32

section .text
_start:
    ; --- sys_write(stdout, welcome_msg, welcome_len) ---
    mov rax, 1                  ; sys_write syscall number
    mov rdi, 1                  ; stdout file descriptor
    mov rsi, welcome_msg        ; ptr to string
    mov rdx, welcome_len        ; string length
    syscall

    ; --- sys_read(stdin, input_buf, 32) ---
    mov rax, 0                  ; sys_read syscall number
    mov rdi, 0                  ; stdin file descriptor
    mov rsi, input_buf          ; ptr to input buffer
    mov rdx, 32                 ; max bytes to read
    syscall

    ; Convert ASCII input string to integer (stored in rbx as max limit)
    xor rbx, rbx                ; rbx = 0 (will hold parsed integer)
    mov rsi, input_buf          ; rsi points to start of input

parse_loop:
    movzx rax, byte [rsi]       ; Load 1 byte from buffer, zero-extend
    inc rsi                     ; Move to next byte
    cmp rax, 0Ah                ; Check for newline (Enter)
    je parse_done
    cmp rax, 0                  ; Check for null terminator
    je parse_done
    sub rax, '0'                ; ASCII to integer ('5' -> 5)
    imul rbx, rbx, 10           ; rbx = rbx * 10 (shift left in base 10)
    add rbx, rax                ; Add current digit to total
    jmp parse_loop

parse_done:
    ; Initialize Fibonacci sequence: F(0) = 0, F(1) = 1
    xor r8, r8                  ; r8 = F(n-2) = 0
    mov r9, 1                   ; r9 = F(n-1) = 1

    cmp rbx, 0                  ; Check if max limit <= 0
    jle exit_program

print_fib_0:
    mov rax, r8                 ; Set rax to 0 for print_number
    call print_number           ; Print F(0)
    cmp rbx, 0
    jle exit_program

    ; Print F(1) before entering the loop to fix missing '1'
    mov rax, r9                 ; Set rax to F(1)
    cmp rax, rbx                ; Ensure max limit isn't exceeded
    jg exit_program
    call print_number           ; Print F(1)

print_fib_loop:
    ; Calculate next Fibonacci number: F(n) = F(n-1) + F(n-2)
    mov rax, r8
    add rax, r9                 ; rax = F(n-2) + F(n-1)
    
    cmp rax, rbx                ; Check if new number exceeds max limit
    jg exit_program

    ; Shift values: r8 = r9, r9 = rax
    mov r8, r9                  ; Old F(n-1) becomes new F(n-2)
    mov r9, rax                 ; New F(n) becomes new F(n-1)

    call print_number
    jmp print_fib_loop

; Helper function to print the number currently in rax
print_number:
    ; Save all modified registers to preserve state
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi

    mov rdi, input_buf + 31     ; Start at end of buffer (working backwards)
    mov byte [rdi], 0x0A        ; Append newline char
    dec rdi                     ; Move ptr left for next char
    mov rcx, 1                  ; Init char count (1 for newline)

    mov rax, [rsp + 40]         ; Retrieve original rax from stack
    test rax, rax               ; Check if number is 0
    jnz convert_loop
    
    ; Special case for zero
    mov byte [rdi], '0'         ; Write '0' to buffer
    dec rdi
    inc rcx                     ; Increment char count
    jmp do_print

convert_loop:
    test rax, rax               ; Stop when quotient is 0
    jz do_print
    xor rdx, rdx                ; Clear rdx for division
    mov rbx, 10
    div rbx                     ; rax = rax / 10, rdx = remainder
    add dl, '0'                 ; Convert remainder to ASCII char
    mov [rdi], dl               ; Store char in buffer
    dec rdi                     ; Move ptr left
    inc rcx                     ; Increment char count
    jmp convert_loop

do_print:
    inc rdi                     ; rdi points to start of generated string
    mov rsi, rdi                ; Set string ptr BEFORE modifying rdi
    mov rax, 1                  ; sys_write syscall number
    mov rdi, 1                  ; stdout file descriptor
    mov rdx, rcx                ; string length
    syscall                     ; Execute write

    ; Restore all saved registers
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    ret

exit_program:
    mov rax, 60                 ; sys_exit syscall number
    xor rdi, rdi                ; standard practice: zero rdi for exit code
    syscall
