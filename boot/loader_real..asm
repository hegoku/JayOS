org 0x0 ;和boot.asm的 OFFSET_OF_LOADER一样

BASE_OF_STACK equ 0x7C00 ;堆栈基地址
SELF_ES equ 0x17E0 ;当前loader程序的段基址
BASE_OF_KERNEL_FILE equ 0x27E0 ;13h中断取出的数据存放的段地址 es, Kernel文件基址
OFFSET_OF_KERNEL_FILE equ 0x0 ;13h中断取出的数据存放的偏移地址 bx

jmp start
nop
%include "include/fat12hdr.inc"
%include "include/elf.inc"

start:
    mov ax,cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BASE_OF_STACK
    call ClearScreen
    call FindKernel
    jmp $

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
    mov ax, SELF_ES
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
        jz InitKernel
        push ax
        mov dx, ROOT_DIR_SECTORS+SECTOR_NO_OF_ROOT_DIRECTORY-2
        add ax, dx
        add bx, [BPB_BytsPerSec]
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

InitKernel:
    push es

    mov ax, cs
    mov ds, ax

    mov ax, BASE_OF_KERNEL_FILE
    mov es, ax

    mov eax,  dword[es:OFFSET_OF_KERNEL_FILE+ELF32_HDR_ENTRY]
    mov dword [KernelEntry], eax ;kernel入口地址
    mov eax, dword[es:OFFSET_OF_KERNEL_FILE+ELF32_HDR_PHOFF]
    mov dword[ELFPhoff], eax ;Program header table 偏移量
    mov ax, word[es:OFFSET_OF_KERNEL_FILE+ELF32_HDR_PHNUM]
    mov word[ELFPhnum], ax ;Program header条数
    mov ax, word[es:OFFSET_OF_KERNEL_FILE+ELF32_HDR_PHENTSOZE]
    mov word[ELF_Phentsize], ax ;Program header每条大小
    mov cx, 0
    mov bx, OFFSET_OF_KERNEL_FILE
    add bx, word[ELFPhoff] ;program header 条目地址
    .lop:
        cmp cx, word[ELFPhnum]
        je JmpToKernel ;所有 Program header条目都处理完
        push bx ;保存program header 条目地址
        add bx, ELF32_PHDR_OFFSET
        mov eax, dword[es:bx]
        mov dword[MemCpySrc], eax
        pop bx
        push bx
        add bx, ELF32_PHDR_FILESZ
        mov eax, dword[es:bx]
        mov dword[MemCpySize], eax
        pop bx
        add bx, ELF32_PHDR_PADDR
        mov eax, dword[es:bx]
        mov dword[ELFPaddr], eax
        push bx
        mov dword[MemCpyDst], 0
        
        push es
        push ds

        mov eax, 0x1f9ea
        xor dx, dx
        
        mov bx, 16
        div ebx
        shr eax,4
        mov ds, ax

        mov eax, dword[MemCpySize]
        push eax
        mov ebp, esp
        mov ebx, [ebp+1]

        push dword[MemCpySize]
        push dword[MemCpySrc]
        push dword[MemCpyDst]
        mov eax, dword[ELFPaddr]
        shr eax, 4
        mov es, ax
        mov ax, BASE_OF_KERNEL_FILE
        mov ds, ax
        call MemCpy
        add esp, 12
        pop es
        pop ds

        pop bx
        add bx, word[ELF_Phentsize]
        inc cx
        jmp .lop
    pop es

JmpToKernel:
    call KillFloppyMotor
    jmp BASE_OF_KERNEL_FILE:(OFFSET_OF_KERNEL_FILE)

DispKernelNotFound:
    mov ax, KernelNotFoundMsg
    call DispStr
    call KillFloppyMotor
    jmp $

DispStr: ;显示字符串, ax作为字符串地址
    push es
    push bx
    mov bp, ax
    mov ax, SELF_ES
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
	mov	eax, [ebp + 9]	; 返回值

	pop	ecx
	pop	edi
	pop	esi
	mov	esp, ebp
	pop	ebp

	ret			; 函数结束，返回

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


