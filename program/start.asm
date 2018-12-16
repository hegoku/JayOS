extern main

bits 32

[section .text]

INT_VECTOR_SYS_CALL equ 0x80

global _start
global sys_call_0_param
global sys_call_1_param
global sys_call_2_param
global sys_call_3_param

_start:
    push eax
    push ecx
    call main
    
    push eax

sys_call_0_param:
    push esi
	mov	esi, esp
    push ebx
    push ecx
    push edx
	mov	eax, [esi + 8]	; system call table index, the index of sys_call_table[]
	mov	ebx, [esi + 12]	; not used
	mov	ecx, [esi + 16]	; not used
	mov	edx, 0	; not used
    int INT_VECTOR_SYS_CALL
    pop edx
    pop ecx
    pop ebx
    mov esp, esi
    pop esi
    ret

sys_call_1_param:
    push esi
	mov	esi, esp
    push ebx
    push ecx
    push edx
	mov	eax, [esi + 8]	; system call table index, the index of sys_call_table[]
	mov	ebx, [esi + 12]
	mov	ecx, 0	; not used
	mov	edx, 0	; not used
    int INT_VECTOR_SYS_CALL
    pop edx
    pop ecx
    pop ebx
    mov esp, esi
    pop esi
    ret

sys_call_2_param:
    push esi
	mov	esi, esp
    push ebx
    push ecx
    push edx
	mov	eax, [esi + 8]	; system call table index, the index of sys_call_table[]
	mov	ebx, [esi + 12]
	mov	ecx, [esi + 16]
	mov	edx, 0	; not used
    int INT_VECTOR_SYS_CALL
    pop edx
    pop ecx
    pop ebx
    mov esp, esi
    pop esi
    ret

sys_call_3_param:
    push esi
	mov	esi, esp
    push ebx
    push ecx
    push edx
	mov	eax, [esi + 8]	; system call table index, the index of sys_call_table[]
	mov	ebx, [esi + 12]
	mov	ecx, [esi + 16]
	mov	edx, [esi + 20]
    int INT_VECTOR_SYS_CALL
    pop edx
    pop ecx
    pop ebx
    mov esp, esi
    pop esi
    ret
