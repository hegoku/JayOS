[SECTION .data]
disp_pos	dd	0

[SECTION .text]

global DispStr
global DispColorStr
global in_byte
global out_byte
global port_read
global port_write
global enable_irq
global disable_irq
global disable_int
global enable_int

;include/string.h
; global memcpy
; global memset
; global strcpy
; global strlen

[BITS 32]

; ------------------------------------------------------------------------
; 内存拷贝，仿 memcpy
; ------------------------------------------------------------------------
; void* memcpy(void* es:pDest, void* ds:pSrc, int iSize);
; ----
memcpy:
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
    ; cli
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

; ; ------------------------------------------------------------------------
; ; void memset(void* p_dst, char ch, int size);
; ; ------------------------------------------------------------------------
; memset:
; 	push	ebp
; 	mov	ebp, esp

; 	push	esi
; 	push	edi
; 	push	ecx

; 	mov	edi, [ebp + 8]	; Destination
; 	mov	edx, [ebp + 12]	; Char to be putted
; 	mov	ecx, [ebp + 16]	; Counter
; .1:
; 	cmp	ecx, 0		; 判断计数器
; 	jz	.2		; 计数器为零时跳出

; 	mov	byte [edi], dl		; ┓
; 	inc	edi			; ┛

; 	dec	ecx		; 计数器减一
; 	jmp	.1		; 循环
; .2:

; 	pop	ecx
; 	pop	edi
; 	pop	esi
; 	mov	esp, ebp
; 	pop	ebp

; 	ret			; 函数结束，返回

; ; ------------------------------------------------------------------------
; ; char* strcpy(char* p_dst, char* p_src);
; ; ------------------------------------------------------------------------
; strcpy:
; 	push    ebp
; 	mov     ebp, esp

; 	mov     esi, [ebp + 12] ; Source
; 	mov     edi, [ebp + 8]  ; Destination

; .1:
; 	mov     al, [esi]               ; ┓
; 	inc     esi                     ; ┃
; 					; ┣ 逐字节移动
; 	mov     byte [edi], al          ; ┃
; 	inc     edi                     ; ┛

; 	cmp     al, 0           ; 是否遇到 '\0'
; 	jnz     .1              ; 没遇到就继续循环，遇到就结束

; 	mov     eax, [ebp + 8]  ; 返回值

; 	pop     ebp
; 	ret                     ; 函数结束，返回
; ; strcpy 结束-------------------------------------------------------------


; ; ------------------------------------------------------------------------
; ; int strlen(char* p_str);
; ; ------------------------------------------------------------------------
; strlen:
;         push    ebp
;         mov     ebp, esp

;         mov     eax, 0                  ; 字符串长度开始是 0
;         mov     esi, [ebp + 8]          ; esi 指向首地址

; .1:
;         cmp     byte [esi], 0           ; 看 esi 指向的字符是否是 '\0'
;         jz      .2                      ; 如果是 '\0'，程序结束
;         inc     esi                     ; 如果不是 '\0'，esi 指向下一个字符
;         inc     eax                     ;         并且，eax 自加一
;         jmp     .1                      ; 如此循环

; .2:
;         pop     ebp
;         ret                             ; 函数结束，返回

; ========================================================================
;                  void port_read(u16 port, void* buf, int n);
; ========================================================================
port_read:
	mov	edx, [esp + 4]		; port
	mov	edi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	insw
	ret

; ========================================================================
;                  void port_write(u16 port, void* buf, int n);
; ========================================================================
port_write:
	mov	edx, [esp + 4]		; port
	mov	esi, [esp + 4 + 4]	; buf
	mov	ecx, [esp + 4 + 4 + 4]	; n
	shr	ecx, 1
	cld
	rep	outsw
	ret

; ========================================================================
;                  void disable_irq(int irq);
; ========================================================================
; Disable an interrupt request line by setting an 8259 bit.
; Equivalent code:
;	if(irq < 8)
;		out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) | (1 << irq));
;	else
;		out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) | (1 << irq));
disable_irq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, 1
        rol     ah, cl                  ; ah = (1 << (irq % 8))
        cmp     cl, 8
        jae     disable_8               ; disable irq >= 8 at the slave 8259
disable_0:
        in      al, INT_M_CTLMASK
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INT_M_CTLMASK, al       ; set bit at master 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
disable_8:
        in      al, INT_S_CTLMASK
        test    al, ah
        jnz     dis_already             ; already disabled?
        or      al, ah
        out     INT_S_CTLMASK, al       ; set bit at slave 8259
        popf
        mov     eax, 1                  ; disabled by this function
        ret
dis_already:
        popf
        xor     eax, eax                ; already disabled
        ret

; ========================================================================
;                  void enable_irq(int irq);
; ========================================================================
; Enable an interrupt request line by clearing an 8259 bit.
; Equivalent code:
;       if(irq < 8)
;               out_byte(INT_M_CTLMASK, in_byte(INT_M_CTLMASK) & ~(1 << irq));
;       else
;               out_byte(INT_S_CTLMASK, in_byte(INT_S_CTLMASK) & ~(1 << irq));
;
enable_irq:
        mov     ecx, [esp + 4]          ; irq
        pushf
        cli
        mov     ah, ~1
        rol     ah, cl                  ; ah = ~(1 << (irq % 8))
        cmp     cl, 8
        jae     enable_8                ; enable irq >= 8 at the slave 8259
enable_0:
        in      al, INT_M_CTLMASK
        and     al, ah
        out     INT_M_CTLMASK, al       ; clear bit at master 8259
        popf
        ret
enable_8:
        in      al, INT_S_CTLMASK
        and     al, ah
        out     INT_S_CTLMASK, al       ; clear bit at slave 8259
        popf
        ret

disable_int:
    cli
    ret

enable_int:
    sti
    ret