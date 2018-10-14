[SECTION .data]
disp_pos	dd	0

[SECTION .txt]

global DispStr
global MemCpy
global DispColorStr
global in_byte
global out_byte

[BITS 32]

; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* MemCpy(void* es:pDest, void* ds:pSrc, int iSize);
; ----
MemCpy:
	push	ebp
	mov	ebp, esp

	push	esi
	push	edi
	push	ecx

	mov	edi, [ebp + 8]	; Destination
	mov	esi, [ebp + 12]	; Source
	mov	ecx, [ebp + 16]	; Counter
.1:
	cmp	ecx, 0		; 判断计数器
	jz	.2		; 计数器为零时跳出

	mov	al, [ds:esi]		; ┓
	inc	esi			; ┃
					; ┣ 逐字节移动
	mov	byte [es:edi], al	; ┃
	inc	edi			; ┛

	dec	ecx		; 计数器减一
	jmp	.1		; 循环
.2:
	mov	eax, [ebp + 8]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回

DispStr:
    push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, 0Fh
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi
    ;移动光标
    cli
    xor eax, eax
    xor edx, edx
    mov edx, 0x3d4
    mov al, 0xe
    out dx, al
    nop
    nop
    mov eax, [disp_pos]
    shr eax, 1
    shr eax, 8
    mov edx, 0x3d5
    out dx, al
    nop
    nop
    mov al, 0xf
    mov edx, 0x3d4
    out dx, al
    nop
    nop
    mov eax, [disp_pos]
    shr eax, 1
    mov edx, 0x3d5
    out dx, al
    nop
    nop
    sti

	pop	ebp
	ret

; ========================================================================
;                  void disp_color_str(char * info, int color);
; ========================================================================
DispColorStr:
	push	ebp
	mov	ebp, esp

	mov	esi, [ebp + 8]	; pszInfo
	mov	edi, [disp_pos]
	mov	ah, [ebp + 12]	; color
.1:
	lodsb
	test	al, al
	jz	.2
	cmp	al, 0Ah	; 是回车吗?
	jnz	.3
	push	eax
	mov	eax, edi
	mov	bl, 160
	div	bl
	and	eax, 0FFh
	inc	eax
	mov	bl, 160
	mul	bl
	mov	edi, eax
	pop	eax
	jmp	.1
.3:
	mov	[gs:edi], ax
	add	edi, 2
	jmp	.1

.2:
	mov	[disp_pos], edi
    ;移动光标
    cli
    xor eax, eax
    xor edx, edx
    mov edx, 0x3d4
    mov al, 0xe
    out dx, al
    nop
    nop
    mov eax, [disp_pos]
    shr eax, 1
    shr eax, 8
    mov edx, 0x3d5
    out dx, al
    nop
    nop
    mov al, 0xf
    mov edx, 0x3d4
    out dx, al
    nop
    nop
    mov eax, [disp_pos]
    shr eax, 1
    mov edx, 0x3d5
    out dx, al
    nop
    nop
    sti
    
	pop	ebp
	ret


in_byte:
    mov edx, [esp+4]
    xor eax, eax
    in al, dx
    nop
    nop
    ret

out_byte:
    mov edx, [esp+4]
    mov al, [esp+4+4]
    out dx, al
    nop
    nop
    ret