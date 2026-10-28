; *********************************************************
; *
; * Description: Print "Hello World!" program in assembly.
; * Author: Moshe Yifrah
; *
; *********************************************************

global _start

section .data
	msg db 'Hello World!', 0x0A  ; 'Hello World!' string followed by newline character (\n)
	len equ $ - msg              ; Calculate the length of the string dynamically

section .text
_start:
	; --- sys_write(stdout, msg, len) ---
	mov rax, 1                   ; System call number 1 (sys_write)
	mov rdi, 1                   ; 1st arg: File descriptor 1 (Standard Output)
	mov rsi, msg                 ; 2nd arg: Pointer to the message string
	mov rdx, len                 ; 3rd arg: Length of the message
	syscall                      ; Trigger the kernel interrupt to execute sys_write

	; --- sys_exit(0) ---
	mov rax, 60                  ; System call number 60 (sys_exit)
	mov rdi, 0                   ; 1st arg: Exit status code (0 = success)
	syscall                      ; Trigger the kernel interrupt to execute sys_exit
