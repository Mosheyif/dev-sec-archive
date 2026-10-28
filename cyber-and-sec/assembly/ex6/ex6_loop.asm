; *********************************************************
; *
; * Description: Loop iteration printing numbers 0 to 9.
; * Author: Moshe Yifrah
; *
; *********************************************************

global _start

section .data
	newline db 0x0A

section .bss
	digit resb 1

section .text
_start:
	xor rbx, rbx				; Initialize loop counter / iteration number to 0 (using rbx to avoid syscall clobbering rcx)

loop_start:
	cmp rbx, 10					; Compare counter with 10
	jge loop_end				; If counter >= 10, exit loop

	; Convert number to ASCII and store in buffer
	mov rax, rbx
	add al, '0'					; Convert digit to its ASCII character representation
	mov [digit], al

	; --- sys_write(stdout, digit, 1) ---
	mov rax, 1					; System call number 1 (sys_write)
	mov rdi, 1					; 1st arg: File descriptor 1 (Standard Output)
	mov rsi, digit				; 2nd arg: Pointer to the character buffer
	mov rdx, 1					; 3rd arg: Length 1 byte
	syscall						; Trigger the kernel interrupt to execute sys_write

	; --- sys_write(stdout, newline, 1) ---
	mov rax, 1
	mov rdi, 1
	mov rsi, newline
	mov rdx, 1
	syscall

	inc rbx						; Increment iteration counter
	jmp loop_start				; Repeat loop

loop_end:
	; --- sys_exit(0) ---
	mov rax, 60					; System call number 60 (sys_exit)
	mov rdi, 0					; 1st arg: Exit status code (0 = success)
	syscall						; Trigger the kernel interrupt to execute sys_exit
