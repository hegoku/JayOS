[section .text]

global _start

; TOP_OF_KERNEL_STACK equ 0x7C00 ;内核栈顶
TOP_OF_KERNEL_STACK equ 0x7C00 ;内核栈顶
TOP_OF_USER_STACK equ 0x6C00 ;用户栈顶
BASE_OF_KERNEL_STACK equ 6C00H ;kernel栈基地址
SELF_CS equ 7E0H ;当前程序的段基址
GDT_SEL_KERNEL_CODE equ 0x8|SA_RPL0 ;因为loader的 GDT_SEL_CODE 选择子为 8
GDT_SEL_KERNEL_DATA equ 0x10|SA_RPL0
GDT_SEL_VIDEO equ 0x18|SA_RPL3
GDT_SEL_USER_CODE equ 0x20|SA_RPL3
GDT_SEL_USER_DATA equ 0x28|SA_RPL3
GDT_SEL_TSS equ 0x30|SA_RPL0
; SELF_ES equ 17E00H ;当前程序的段基址

%include "include/pm.inc"
%include "include/func.inc"

gdt: Descriptor 0, 0, 0
gdt_code: Descriptor 0, 0xfffff, DA_CR|DA_32|DA_LIMIT_4K|DA_DPL0
gdt_data: Descriptor 0, 0xfffff, DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL0
gdt_video: Descriptor 0B8000h, 0xfffff, DA_DRW|DA_DPL3
gdt_user_code: Descriptor 0, 0xfffff, DA_CR|DA_32|DA_LIMIT_4K|DA_DPL3
gdt_user_data: Descriptor 0, 0xfffff, DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL3
gdt_tss: Descriptor 0x00007ef4, TssLen-1, DA_386TSS
gdt_tst: Gate 0x40, 0, 0, (DA_386CGate |DA_DPL3)
gdt_tdt: Descriptor 0x00007f5d, SegCodeDestLen-1, DA_C+DA_32
GdtLen equ $-gdt
gdt_ptr dw GdtLen-1
gdt_ptr_base       dd 0

gdt_tss_s equ gdt_tst-gdt

[BITS 32]
tss:
    dd 0
    dd TOP_OF_KERNEL_STACK
    dd GDT_SEL_KERNEL_DATA
    DD	0			; 1 级堆栈
		DD	0			; 
		DD	0			; 2 级堆栈
		DD	0			; 
		DD	0			; CR3
		DD	0			; EIP
		DD	0			; EFLAGS
		DD	0			; EAX
		DD	0			; ECX
		DD	0			; EDX
		DD	0			; EBX
		DD	0			; ESP
		DD	0			; EBP
		DD	0			; ESI
		DD	0			; EDI
		DD	0			; ES
		DD	0			; CS
		DD	0			; SS
		DD	0			; DS
		DD	0			; FS
		DD	0			; GS
		DD	0			; LDT
		DW	0			; 调试陷阱标志
    dw $-tss+2
    db 0xff
TssLen equ $-tss

calltest:
    mov eax, 0x4444
    jmp $
    SegCodeDestLen	equ	$ - calltest

_start:
; mov eax ,0x5555
; jmp $
; mov eax, DA_386CGate | DA_DPL3
; jmp $
    mov esp, TOP_OF_KERNEL_STACK
    ; mov eax, SELF_CS
    ; xor	eax, eax
	; mov	eax, calltest
    ; jmp $
	; mov	word [gdt_tst + 2], ax
	; shr	eax, 16
	; mov	byte [gdt_tst + 4], al
	; mov	byte [gdt_tst + 7], ah

    ; xor	eax, eax
	; mov	eax, tss
    ; jmp $
	; mov	word [gdt_tss + 2], ax
	; shr	eax, 16
	; mov	byte [gdt_tss + 4], al
	; mov	byte [gdt_tss + 7], ah
    ; jmp $

    ; sgdt [gdt_ptr]
    call moveGdt
    lgdt [gdt_ptr]

    ; lidt [idt_ptr]
    jmp GDT_SEL_KERNEL_CODE:test ;强制刷新

moveGdt:
    ; movzx eax, word[gdt_ptr]
    ; push eax ;size

    ; mov eax, dword[gdt_ptr_base]
    ; push eax ;src
    
    ; mov eax, gdt
    ; push eax ;dst
    
    ; call MemCpy
    ; add esp, 12

    mov ax, GdtLen
    mov word[gdt_ptr], ax

    mov eax, gdt
    mov dword[gdt_ptr_base], eax
    ret

test:
    ; call cstart
    ; int 6
    ; mov eax, TSS
    ; jmp $
    ; mov ax, word[ss3]
    ; jmp $
    ; call 0x0038:0
    mov ax, GDT_SEL_TSS
    ltr ax
    push GDT_SEL_USER_DATA
    push TOP_OF_USER_STACK
    push GDT_SEL_USER_CODE
    push ring3
; 	xor    eax, eax
;  mov    ax, 7E00H
;  shl    eax, 4
;  add    eax, calltest
; jmp $
	; mov eax, calltest
	; jmp $
	; call 0x0038:0
    retf
    jmp $



ring3:
	; int 3
	; mov eax, tss
	; jmp $
    call 0x0038:0
	; call GDT_SEL_USER_DATA:calltest
    mov ax, GDT_SEL_VIDEO
    mov gs, ax
    mov edi, (80*14+20)*2
    mov ah, 0ch
    mov al, '3'
    mov [gs:edi], ax
    ; mov eax, GDT_SEL_USER_DATA
    ; mov ds, eax
    ; mov word[0x27a00], 0x3355
    jmp $