; 500H     -------------
;          |user stack | 25.75K
; 6C00H    -------------
;          |kernel stack| 4k
; 7C00H    ------------
;          |   boot   | 512
; 7E00H    ------------
;          |  KERNEL  | 64K
; 17E00H   ------------
;
[section .text]

global _start

; TOP_OF_KERNEL_STACK equ 0x7C00 ;内核栈顶
TOP_OF_KERNEL_STACK equ 0x7C00 ;内核栈顶
TOP_OF_USER_STACK equ 0x6C00 ;用户栈顶
BASE_OF_KERNEL_STACK equ 6C00H ;kernel栈基地址
; SELF_CS equ 7E0H ;当前程序的段基址
SELF_CS equ 10000H ;当前程序的段基址
GDT_SEL_KERNEL_CODE equ 0x8|SA_RPL0 ;因为loader的 GDT_SEL_CODE 选择子为 8
GDT_SEL_KERNEL_DATA equ 0x10|SA_RPL0
GDT_SEL_VIDEO equ 0x18|SA_RPL3
GDT_SEL_USER_CODE equ 0x20|SA_RPL3
GDT_SEL_USER_DATA equ 0x28|SA_RPL3
GDT_SEL_TSS equ 0x30|SA_RPL0
; SELF_ES equ 17E00H ;当前程序的段基址
INT_VECTOR_SYS_CALL equ 0x80 ;系统中断号

%include "include/pm.inc"
%include "include/func.inc"

extern exception_handler
extern cstart
extern kernel_main
extern SelectorTss
extern tss
extern irq_table
extern sys_call_table
extern current_process
extern is_in_ring0

global gdt
global gdt_ptr
global idt
global idt_ptr
global page_dir_ptr

global restart
;默认中断向量
global	divide_error
global	single_step_exception
global	nmi
global	breakpoint_exception
global	overflow
global	bounds_check
global	inval_opcode
global	copr_not_available
global	double_fault
global	copr_seg_overrun
global	inval_tss
global	segment_not_present
global	stack_exception
global	general_protection
global	page_fault
global	copr_error
global	hwint00
global	hwint01
global	hwint02
global	hwint03
global	hwint04
global	hwint05
global	hwint06
global	hwint07
global	hwint08
global	hwint09
global	hwint10
global	hwint11
global	hwint12
global	hwint13
global	hwint14
global	hwint15
global sys_call

global sys_call_0_param
global sys_call_1_param
global sys_call_2_param
global sys_call_3_param

gdt times 128*64 db 0
GdtLen equ $-gdt

gdt_ptr dw GdtLen-1
gdt_ptr_base dd gdt

idt:
times 256*64 db 0
IdtLen equ $-idt

idt_ptr dw IdtLen-1
idt_ptr_base dd idt

page_dir_ptr dw 0

[BITS 32]
_start:
    mov esp, TOP_OF_KERNEL_STACK
	call cstart
    lgdt [gdt_ptr]
    lidt [idt_ptr]
    jmp GDT_SEL_KERNEL_CODE:csinit ;强制刷新

csinit:
    mov ax, GDT_SEL_TSS
    ltr ax

    ; mov eax, page_dir_ptr
    ; mov eax, 1000H
    mov eax, 180000H
    mov cr3, eax
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    jmp kernel_main
    retf
    jmp $

; 中断和异常 -- 异常
divide_error:
	push	0xFFFFFFFF	; no err code
	push	0		; vector_no	= 0
	jmp	exception
single_step_exception:
	push	0xFFFFFFFF	; no err code
	push	1		; vector_no	= 1
	jmp	exception
nmi:
	push	0xFFFFFFFF	; no err code
	push	2		; vector_no	= 2
	jmp	exception
breakpoint_exception:
	push	0xFFFFFFFF	; no err code
	push	3		; vector_no	= 3
	jmp	exception
overflow:
	push	0xFFFFFFFF	; no err code
	push	4		; vector_no	= 4
	jmp	exception
bounds_check:
	push	0xFFFFFFFF	; no err code
	push	5		; vector_no	= 5
	jmp	exception
inval_opcode:
	push	0xFFFFFFFF	; no err code
	push	6		; vector_no	= 6
	jmp	exception
copr_not_available:
	push	0xFFFFFFFF	; no err code
	push	7		; vector_no	= 7
	jmp	exception
double_fault:
	push	8		; vector_no	= 8
	jmp	exception
copr_seg_overrun:
	push	0xFFFFFFFF	; no err code
	push	9		; vector_no	= 9
	jmp	exception
inval_tss:
	push	10		; vector_no	= A
	jmp	exception
segment_not_present:
	push	11		; vector_no	= B
	jmp	exception
stack_exception:
	push	12		; vector_no	= C
	jmp	exception
general_protection:
	push	13		; vector_no	= D
	jmp	exception
page_fault:
	push	14		; vector_no	= E
	jmp	exception
copr_error:
	push	0xFFFFFFFF	; no err code
	push	16		; vector_no	= 10h
	jmp	exception

exception:
	call exception_handler
	add	esp, 4*2	; 让栈顶指向 EIP，堆栈中从顶向下依次是：EIP、CS、EFLAGS
	hlt

%macro	hwint_master 1
    call save
    ; sub esp,4
    ; pushad
    ; push ds
    ; push es
    ; push fs
    ; push gs
    ; mov dx,ss
    ; mov ds, dx
    ; mov es,dx

    ; inc dword[is_in_ring0]
    ; cmp dword[is_in_ring0], 0
    ; jne ret_to_proc

    ; mov esp, TOP_OF_KERNEL_STACK

    in al, INT_M_CTLMASK ;屏蔽当前中断
    or al, (1<<%1)
    out INT_M_CTLMASK, al
    mov al, EOI
    out INT_M_CTL, al

    sti

    push %1
	call [irq_table+4*%1]
	add	esp, 4

    cli

    in al, INT_M_CTLMASK ;恢复当前中断
    and al, ~(1<<%1)
    out INT_M_CTLMASK, al

    ; push restart
	ret
%endmacro

hwint00:		; Interrupt routine for irq 0 (the clock).
hwint_master 0
    ; sub esp,4
;     pushad
;     push ds
;     push es
;     push fs
;     push gs
;     mov dx,ss
;     mov ds, dx
;     mov es,dx

;     inc dword[is_in_ring0]
;     cmp dword[is_in_ring0], 0
;     jne .1
;     mov esp, TOP_OF_KERNEL_STACK
;     push restart
;     jmp .3

; .1:
;     push ret_to_proc
; .3:

;     in al, INT_M_CTLMASK ;屏蔽当前中断
;     or al, (1<<0)
;     out INT_M_CTLMASK, al
;     mov al, EOI
;     out INT_M_CTL, al

;     sti

;     push 0
; 	call [irq_table+4*0]
; 	add	esp, 4

;     cli

;     in al, INT_M_CTLMASK ;恢复当前中断
;     and al, ~(1<<0)
;     out INT_M_CTLMASK, al

; 	ret

; ALIGN	16
hwint01:		; Interrupt routine for irq 1 (keyboard)
	hwint_master 1

; ALIGN	16
hwint02:		; Interrupt routine for irq 2 (cascade!)
   hwint_master 2

; ALIGN	16
hwint03:		; Interrupt routine for irq 3 (second serial)
	hwint_master 3

; ALIGN	16
hwint04:		; Interrupt routine for irq 4 (first serial)
	hwint_master 4

; ALIGN	16
hwint05:		; Interrupt routine for irq 5 (XT winchester)
	hwint_master 5

; ALIGN	16
hwint06:		; Interrupt routine for irq 6 (floppy)
	hwint_master 6

; ALIGN	16
hwint07:		; Interrupt routine for irq 7 (printer)
	hwint_master 7

; ---------------------------------
%macro	hwint_slave	1
	call save

    in al, INT_S_CTLMASK ;屏蔽当前中断
    or al, (1<<(%1-8))
    out INT_S_CTLMASK, al
    mov al, EOI
    out INT_M_CTL, al
    nop
    out INT_S_CTL, al

    sti
    push %1
	call [irq_table+4*%1]
	add	esp, 4
    cli

    in al, INT_S_CTLMASK ;恢复当前中断
    and al, ~(1<<(%1-8))
    out INT_S_CTLMASK, al

	ret
%endmacro
; ---------------------------------

; ALIGN	16
hwint08:		; Interrupt routine for irq 8 (realtime clock).
	hwint_slave	8

; ALIGN	16
hwint09:		; Interrupt routine for irq 9 (irq 2 redirected)
	hwint_slave	9

; ALIGN	16
hwint10:		; Interrupt routine for irq 10
	hwint_slave	10

; ALIGN	16
hwint11:		; Interrupt routine for irq 11
	hwint_slave	11

; ALIGN	16
hwint12:		; Interrupt routine for irq 12
	hwint_slave	12

; ALIGN	16
hwint13:		; Interrupt routine for irq 13 (FPU exception)
	hwint_slave	13

; ALIGN	16
hwint14:		; Interrupt routine for irq 14 (AT winchester)
	hwint_slave	14

; ALIGN	16
hwint15:		; Interrupt routine for irq 15
	hwint_slave	15

sys_call:
    call save
    ; sub esp,4
    ; pushad
    ; push ds
    ; push es
    ; push fs
    ; push gs
    ; mov edi,ss
    ; mov ds, edi
    ; mov es,edi

    ; mov esi, esp ;进程表起始地址

    ; inc dword[is_in_ring0]
    ; cmp dword[is_in_ring0], 0
    ; jne ret_to_proc

    ; mov esp, TOP_OF_KERNEL_STACK

    sti
    push esi

    push edx
    push ecx
    push ebx
	call [sys_call_table+4*eax]
    add esp, 4*3
    pop esi
    mov [esi+EAXREG-P_STACKBASE], eax ;保存 [sys_call_table+4*eax]  函数的返回值到进程表的eax寄存器以便获取

    cli
    ; push restart
	ret

save:
    pushad
    push ds
    push es
    push fs
    push gs

    mov esi,ss
    mov ds, esi
    mov es,esi

    mov esi, esp ;进程表起始地址

    inc dword[is_in_ring0]
    cmp dword[is_in_ring0], 0
    jne .1
    mov esp, TOP_OF_KERNEL_STACK
    push restart
    jmp [esi+RETADR-P_STACKBASE]
.1:
    push ret_to_proc
    jmp [esi+RETADR-P_STACKBASE]

restart:
    mov	esp, [current_process]
	lldt [esp + P_LDT_SEL]
    mov eax, dword[esp+P_CR3]
    mov cr3, eax
	lea	eax, [esp + P_STACKTOP]
	mov	dword [tss + TSS3_S_SP0], eax
ret_to_proc:
    dec dword[is_in_ring0]
    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp,4
    iretd

; void sys_call_0_param(int index);
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


