org 0x0 ;和boot.asm的 OFFSET_OF_LOADER一样

TOP_OF_STACK equ 0x7C00 ;loader栈顶
BIOS_ADDR equ 7E00H ;bios信息存放起始地址
BIOS_HD equ 0080H ;bios存放硬盘信息的偏移量
TOP_OF_KERNEL_STACK equ 27E00H ;保护模式下栈顶
; TOP_OF_KERNEL_STACK equ 7C00H ;kernel栈顶
BASE_OF_KERNEL_STACK equ 6C00H ;kernel栈基地址
SELF_CS equ 0x17E0 ;当前loader程序的段基址
BASE_OF_KERNEL_FILE equ 0x27E0 ;13h中断取出的数据存放的段地址 es, Kernel文件基址
OFFSET_OF_KERNEL_FILE equ 0x0 ;13h中断取出的数据存放的偏移地址 bx

IDT_BASE equ 0x17E0 ;中断向量基址
IDT_LIMIT equ 0x27E0 ;中断向量界限

jmp start
nop
%include "include/fat12hdr.inc"
%include "include/elf.inc"
%include "include/pm.inc"
%include "include/func.inc"

[BITS 16]
start:
    mov ax,cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, TOP_OF_STACK
    call ClearScreen
    ; call ReadBIOS
    call ReadMemorySize
    call FindKernel
    jmp $

ReadBIOS1:
    push ds
    push es
    ;// 取第一个硬盘的信息（复制硬盘参数表）。
    ;// 第1 个硬盘参数表的首地址竟然是中断向量41 的向量值！而第2 个硬盘
    ;// 参数表紧接第1 个表的后面，中断向量46 的向量值也指向这第2 个硬盘
    ;// 的参数表首址。表的长度是16 个字节(10)。
    ;// 下面两段程序分别复制BIOS 有关两个硬盘的参数表，7E080 处存放第1 个
    ;// 硬盘的表，7E090 处存放第2 个硬盘的表。
    mov	ax,0000h
    mov	ds,ax
    lds	si,[ds:4*41h]		;// 取中断向量41 的值，也即hd0 参数表的地址 ds:si
    mov	ax,BIOS_ADDR
    mov	es,ax
    mov	di,BIOS_HD		;// 传输的目的地址: 7E00:0080 -> es:di
    mov	cx,10h			;// 共传输10 字节。
    rep movsb
    ;// Get hd1 data
	mov	ax,0000h
	mov	ds,ax
	lds	si,[ds:4*46h]		;// 取中断向量46 的值，也即hd1 参数表的地址 -> ds:si
	mov	ax,BIOS_ADDR
	mov	es,ax
	mov	di,BIOS_HD+10H		;// 传输的目的地址: 7E00:0090 -> es:di
	mov	cx,10h
	rep movsb
    ;// 检查系统是否存在第2 个硬盘，如果不存在则第2 个表清零。
    ;// 利用BIOS 中断调用13 的取盘类型功能。
    ;// 功能号ah = 15；
    ;// 输入：dl = 驱动器号（8X 是硬盘：80 指第1 个硬盘，81 第2 个硬盘）
    ;// 输出：ah = 类型码；00 --没有这个盘，CF 置位； 01 --是软驱，没有change-line 支持；
    ;//  02--是软驱(或其它可移动设备)，有change-line 支持； 03 --是硬盘。
	mov	ax,1500h
	mov	dl,80h
	int	13h
	jc	no_disk1
	cmp	ah,3			;// 是硬盘吗？(类型= 3 ？)。
	je	has_disk1
no_disk1:
	mov	ax,BIOS_ADDR		;// 第2个硬盘不存在，则对第2个硬盘表清零。
	mov	es,ax
	mov	di,BIOS_HD+10H
	mov	cx,10h
	mov	ax,00h
	rep stosb
has_disk1:
    pop es
    pop ds
    ret

ReadBIOS:
    push ds
    xor ax, ax
    mov ds, ax
    mov ax,0
    mov [ds:0x475], ax

    mov cl, 80h
.hd_loop:
    push cx
    mov	ax,1500
	mov	dl, cl
	int	13h
	jc	.hd_add_cl
	cmp	ah,3			;// 是硬盘吗？(类型= 3 ？)。
	je	.has_disk

    jz .hd_count_finish
.has_disk:
    mov ax, [ds:0x475]
    inc ax
    mov [ds:0x475], ax
    jmp .hd_add_cl

.hd_add_cl:
    pop cx
    inc cl
    cmp cl, 90H
    je .hd_count_finish
    jmp .hd_loop

.hd_count_finish:
    pop ds
    ret

ReadMemorySize:
    pushad
    push ds
    push es
    xor eax, eax
    mov ds, ax
    mov es, ax
    mov ebx, 0
    ; mov ax, _MemChkBuf
    ; mov [ds:0x500], ax ;内核会从这个地址取_MemChkbuf的地址
    ; mov ax, _dwMCRNumber
    ; mov [ds:0x502], ax ;内核会从这个地址取_dwMCRNumber的地址
    ; mov di, _MemChkBuf
    mov word[ds:BIOS_ADDR], bx
    mov di, BIOS_ADDR+2
.ReadMemorySizeLoop:
    mov eax, 0E820h
    mov ecx, 20
    mov edx, 0534D4150h
    int 15h
    jc .LABEL_MEM_CHK_FAIL
    add di, 20
    ; inc dword[_dwMCRNumber]
    inc dword[ds:BIOS_ADDR]
    cmp ebx, 0
    jne .ReadMemorySizeLoop
    jmp .LABEL_MEM_CHK_OK
.LABEL_MEM_CHK_FAIL:
    ; mov dword [_dwMCRNumber], 0
    mov dword [ds:BIOS_ADDR], 0
.LABEL_MEM_CHK_OK:
    pop es
    pop ds
    popad
    ret

FindKernel:
    xor ah, ah
    xor dl, dl
    int 13h

    .ReadRootSectorLoop: ;循环读取所有根目录扇区
        mov byte[RootDirEntryCount], 0
        movzx ax, [RootDirSectorCount]
        cmp ax, ROOT_DIR_SECTORS
        je DispKernelNotFound;如果到达了根目录最大条数, 说明没有找到 Kernel, 则跳到未找到到提示
        mov ax, BASE_OF_KERNEL_FILE
        mov es, ax
        mov bx, OFFSET_OF_KERNEL_FILE
        mov ax, SECTOR_NO_OF_ROOT_DIRECTORY ;要读的扇区号
        mov cl, 1 ;一次读几个扇区
        call ReadSector ;读了一个扇区
        .ReadRootEntryLoop: ;循环读取该扇区中的根目录条目
            call FindKernelFileName        
            add bx, ROOT_DIRECTORY_ENTRY_SIZE ;移动到下一条目

            inc word[RootDirEntryCount]
            movzx ax, [RootDirEntryCount]
            cmp ax, ROOT_DIR_ENTRY
            jne .ReadRootEntryLoop;如果没有到达了条目最大条数, 继续搜索下一个条目

        inc word[RootDirSectorCount]
        jmp .ReadRootSectorLoop

ReadSector: ;ax传入要读取的扇区号
    push bp
    mov bp, sp
    sub esp, 2

    mov byte[bp-2], cl
    push bx
    mov bl, [BPB_SecPerTrk]
    div bl
    mov ch, al ;柱面
    shr ch, 1
    mov dh, al ;磁头号
    and dh, 1
    mov dl, [BS_DrvNum] ;启动器号
    mov cl, ah ;起始扇区号
    inc cl
    pop bx
    .GoOnReading:
        mov ah, 2
        mov al, byte[bp-2]
        int 13h
        jc .GoOnReading
        add esp, 2
        pop bp
        ret

FindKernelFileName:
    push ds
    push si
    push di
    mov cx, 11
    mov ax, SELF_CS
    mov ds, ax
    mov si, KernelFileName
    mov di, bx
    cld
.CompareChar:
    cmp cx, 0
    jz BootKernel ;如果11个字符都相等, 说明找到了
    dec cx
    lodsb
    cmp al, byte[es:di]
    jz .CompareNextChar
    jmp .FindKernelFileNameRet ;如果有一个字符不一样说明不是要找的
.CompareNextChar:
    inc di
    jmp .CompareChar
.FindKernelFileNameRet:
    pop di
    pop si
    pop ds
    ret

BootKernel:
    sub di, 11 ;因为找Kernel的时候执行了11次 inc di，所以为了复位kernel所在的根目录条目，减去11
    add di, ROOT_DIRECTORY_ENTRY_FSTCLUS_OFFSET ;找到开始簇号, 占用2个字节
    mov cx, word[es:di] ;将开始簇号存入cx
    push cx
    mov ax, ROOT_DIR_SECTORS+SECTOR_NO_OF_ROOT_DIRECTORY-2
    add cx, ax
    mov ax, BASE_OF_KERNEL_FILE
    mov es, ax
    mov bx, OFFSET_OF_KERNEL_FILE
    mov ax, cx
    .Loading:
        mov cl, 1
        call ReadSector
        pop ax
        call GetFATEntry
        cmp ax, 0FFFh
        jz InitProtectedModel
        push ax
        mov dx, ROOT_DIR_SECTORS+SECTOR_NO_OF_ROOT_DIRECTORY-2
        add ax, dx
        add bx, [BPB_BytsPerSec]
        jc .1 ;如果bx重新变成0，说明内核大于64KB
        jmp .Loading
.1: ;es+=0x1000, 指向下一个段
    push ax
    mov ax, es
    add ax, 1000h
    mov es, ax
    pop ax
    jmp .Loading

GetFATEntry:
    push es
    push bx
    push ax
    mov ax, FAT_ES
    mov es, ax
    pop ax
    mov byte[bOdd], 0
    mov bx, 3
    mul bx
    mov bx, 2
    div bx
    cmp dx, 0
    jz .Label_even
    mov byte[bOdd], 1
    .Label_even:
        xor dx, dx
        mov bx, [BPB_BytsPerSec]
        div bx
        push dx
        mov bx, 0
        add ax, [BPB_RsvdSecCnt]
        mov cl, 2
        call ReadSector

        pop dx
        add bx, dx
        mov ax, [es:bx]
        cmp byte[bOdd], 1
        jnz .Label_even_2
        shr ax, 4
    .Label_even_2:
        and ax, 0FFFh

    pop bx
    pop es
    ret

DispKernelNotFound:
    mov ax, KernelNotFoundMsg
    call DispStrRM
    call KillFloppyMotor
    jmp $

DispStrRM: ;显示字符串, ax作为字符串地址
    push es
    push bx
    mov bp, ax
    mov ax, SELF_CS
    mov es, ax
    mov cx, 16
    mov ax, 01301h
    mov bx, 0007h
    mov dl, 0
    int 10h
    pop bx
    pop es
    ret

ClearScreen:
    mov ax, 0600h
    mov bx, 0700h
    mov cx, 0
    mov dx, 0184fh
    int 10h
    mov ax, 0200h
    mov bh, 0
    mov dx, 0
    int 10h
    ret

KillFloppyMotor:
    push dx
    mov dx, 03F2h
    mov al, 0
    out dx, al
    pop dx
    ret

InitProtectedModel:
    call KillFloppyMotor

    lgdt [GdtPtr]

    cli

    in al, 92h
    or al, 00000010b
    out 92h, al

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp dword GDT_SEL_CODE:(SELF_CS*10h+InitKernel)

[section .s32]
ALIGN 32
[BITS 32]

InitKernel:
    mov ax, GDT_SEL_VIDEO
    mov gs, ax

    mov ax, GDT_SEL_DATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    ; mov ax, BASE_OF_KERNEL_STACK
    mov ss, ax
    mov esp, TOP_OF_KERNEL_STACK

    mov eax,  dword[es:BASE_OF_KERNEL_FILE*10h+(OFFSET_OF_KERNEL_FILE+ELF32_HDR_ENTRY)]
    mov dword [SELF_CS*10h+KernelEntry], eax ;kernel入口地址
    mov eax, dword[es:BASE_OF_KERNEL_FILE*10h+(OFFSET_OF_KERNEL_FILE+ELF32_HDR_PHOFF)]
    mov dword[SELF_CS*10h+ELFPhoff], eax ;Program header table 偏移量
    mov ax, word[es:BASE_OF_KERNEL_FILE*10h+(OFFSET_OF_KERNEL_FILE+ELF32_HDR_PHNUM)]
    mov word[SELF_CS*10h+ELFPhnum], ax ;Program header条数
    mov ax, word[es:BASE_OF_KERNEL_FILE*10h+(OFFSET_OF_KERNEL_FILE+ELF32_HDR_PHENTSOZE)]
    mov word[SELF_CS*10h+ELF_Phentsize], ax ;Program header每条大小
    mov cx, 0
    mov ebx, BASE_OF_KERNEL_FILE*10h+OFFSET_OF_KERNEL_FILE
    add ebx, dword[SELF_CS*10h+ELFPhoff] ;program header 条目地址
    .lop:
        cmp cx, word[SELF_CS*10h+ELFPhnum]
        je JmpToKernel ;所有 Program header条目都处理完
        push ebx ;保存program header 条目地址
        add ebx, ELF32_PHDR_OFFSET
        mov eax, BASE_OF_KERNEL_FILE*10h
        add eax, dword[es:ebx]
        mov dword[SELF_CS*10h+MemCpySrc], eax
        pop ebx

        push ebx
        add ebx, ELF32_PHDR_FILESZ
        mov eax, dword[es:ebx]
        mov dword[SELF_CS*10h+MemCpySize], eax
        pop ebx

        push ebx
        add ebx, ELF32_PHDR_PADDR
        mov eax, dword[es:ebx]
        mov dword[SELF_CS*10h+ELFPaddr], eax
        mov dword[SELF_CS*10h+MemCpyDst], eax
        
        push dword[SELF_CS*10h+MemCpySize]
        push dword[SELF_CS*10h+MemCpySrc]
        push dword[SELF_CS*10h+MemCpyDst]

        call memcpy
        add esp, 12

        pop ebx
        movzx eax, word[SELF_CS*10h+ELF_Phentsize]
        add ebx, eax
        inc cx
        jmp .lop

JmpToKernel:
    jmp [SELF_CS*10h+KernelEntry]

; memcpy:
; 	push	ebp
; 	mov	ebp, esp

; 	push	esi
; 	push	edi
; 	push	ecx

; 	mov	edi, [ebp + 8]	; Destination
; 	mov	esi, [ebp + 12]	; Source
; 	mov	ecx, [ebp + 16]	; Counter
; .1:
; 	cmp	ecx, 0		; 判断计数器
; 	jz	.2		; 计数器为零时跳出

; 	mov	al, [ds:esi]		; ┓
; 	inc	esi			; ┃
; 					; ┣ 逐字节移动
; 	mov	byte [es:edi], al	; ┃
; 	inc	edi			; ┛

; 	dec	ecx		; 计数器减一
; 	jmp	.1		; 循环
; .2:
; 	mov	eax, [ebp + 8]	; 返回值

; 	pop	ecx
; 	pop	edi
; 	pop	esi
; 	mov	esp, ebp
; 	pop	ebp

; 	ret			; 函数结束，返回

RootDirSectorCount db 0 ;根目录扇区循环次数, 每循环一次加一, 最大值为 ROOT_DIR_SECTORS
RootDirEntryCount db 0 ;根目录扇区的条目循环次数, 没读取一条根目录加一次, 最大值为 ROOT_DIR_ENTRY
KernelFileName db 'KERNEL     ' ;Kernel文件名
KernelNotFoundMsg db 'Kernel not found'
KernelFoundMsg db 'Kernel found    '
bOdd db 0 ;是奇数数还是偶数

KernelEntry dd 0 ;kernel入口地址
ELFPhoff dd 0 ;Program header table偏移量
ELF_Phentsize dw 0 ;program header table条目大小
ELFPhnum dw 0 ;prgram header table中条目数
ELFPaddr dd 0 ;program header内存地址
MemCpySize dd 0
MemCpyDst dd 0
MemCpySrc dd 0

gdt_0: Descriptor 0, 0, 0
gdt_code: Descriptor 0, 0xfffff, (DA_CR|DA_32|DA_LIMIT_4K|DA_DPL0)
gdt_data: Descriptor 0, 0xfffff, (DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL0)
gdt_video: Descriptor 0B8000h, 0xfffff, (DA_DRW|DA_DPL3)
; gdt_user_code: Descriptor 0, 0xfffff, (DA_CR|DA_32|DA_LIMIT_4K|DA_DPL3)
; gdt_user_data: Descriptor 0, 0xfffff, (DA_DRW|DA_32|DA_LIMIT_4K|DA_DPL3)
GdtLen equ $-gdt_0
GdtPtr dw GdtLen-1
       dd SELF_CS*10h+gdt_0

GDT_SEL_CODE equ gdt_code-gdt_0
GDT_SEL_DATA equ gdt_data-gdt_0
GDT_SEL_VIDEO equ gdt_video-gdt_0
; GDT_SEL_USER_CODE equ gdt_user_code-gdt_0
; GDT_SEL_USER_DATA equ gdt_user_data-gdt_0

_dwMCRNumber dd 0
_MemChkBuf:times 256 db 0
