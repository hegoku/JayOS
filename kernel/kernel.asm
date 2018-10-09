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

; extern moveGdt
; extern gdt_ptr
; extern gdt
extern exception_handler
extern cstart
extern SelectorTss

global gdt
global gdt_ptr
global idt
global idt_ptr

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


global calltest
global tss
extern testcallS
extern ss3

gdt times 128*64 db 0
GdtLen equ $-gdt

gdt_ptr dw 0
gdt_ptr_base dd 0

idt:
; %rep 255
; Gate GDT_SEL_KERNEL_CODE, inval_opcode_limit, 0, DA_386IGate
; %endrep
times 256*64 db 0
IdtLen equ $-idt

idt_ptr dw IdtLen-1
idt_ptr_base dd idt

[BITS 32]
_start:
    mov esp, TOP_OF_KERNEL_STACK
    ; mov eax, SELF_CS

    ; sgdt [gdt_ptr]
	call cstart
    call moveGdt
    lgdt [gdt_ptr]

    lidt [idt_ptr]

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

extern exception_handler
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
inval_opcode_limit equ inval_opcode-$$
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

test:
    ; call cstart
    ; int 6
    ; mov eax, TSS
    ; jmp $
    ; mov ax, word[ss3]
    ; jmp $
    ; mov eax,tss
    ; jmp $
    mov ax, GDT_SEL_TSS
    ltr ax
    push GDT_SEL_USER_DATA
    push TOP_OF_USER_STACK
    push GDT_SEL_USER_CODE
    push ring3
    retf
    jmp $

ring3:
	; int 3
    call 0x0038:0
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



