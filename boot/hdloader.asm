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

ROOT_BASE equ 0x2801 ;分区起始扇区
ROOT_DIR_START_SECTOR equ 0x3000 ;存放根目录起始扇区的地址 dword
ROOT_DIR_SIZE equ ROOT_DIR_START_SECTOR + 4 ;根目录总字节数

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
    
    ; 读取fat16的super block
    ; mov dword[disk_address_packer+8], ROOT_BASE ;要读的扇区号
    ; mov word[disk_address_packer+4], 0x5000
    ; mov word[disk_address_packer+6], 0x17e0
    ; call ReadSector
    ; jmp $
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
    call CalMemSize
    pop es
    pop ds
    popad
    ret

CalMemSize:
    xor esi, ecx
    xor ecx, ecx
    mov esi, BIOS_ADDR+2
    mov ecx, dword[ds:BIOS_ADDR]
.cloop:
    mov eax, dword[esi+4*4]
    cmp eax, 1
    jne .2
    mov eax, dword[esi+0]
    add eax, dword[esi+2*4]
    cmp eax, dword[MemSize]
    jb .2
    mov dword[MemSize], eax
.2:
    add esi, 5*4
    loop .cloop
    ret

SetupPaging:
    pushad
    push ds
    push es
    xor eax, eax
    mov ds, ax
    mov es, ax

    mov dword[Page_Dir_Base], Page_Tbl_base | PG_P | PG_USS | PG_RWW
    mov dword[Page_Dir_Base+4], Page_Tbl_base+1000H | PG_P | PG_USS | PG_RWW ;第二个4MB
    mov dword[Page_Dir_Base+3072], Page_Tbl_base | PG_P | PG_USS | PG_RWW
    mov dword[Page_Dir_Base+3072+4], Page_Tbl_base+1000H | PG_P | PG_USS | PG_RWW
    mov edi, Page_Tbl_base+4092+4096
    ; mov eax, 03ff007H  ;4Mb - 4096 + 7 (r/w user,p)
    mov eax, 07ff007H  ;8Mb - 4096 + 7 (r/w user,p)
    mov ecx, 1024*2
    std
 .1:
    stosd
    sub eax, 0x1000
    loop .1
    cld
    mov eax, Page_Dir_Base
    mov cr3, eax
    ; mov eax, cr0
    ; or eax, 0x80000000
    ; mov cr0, eax
    pop es
    pop ds
    popad
    ret

FindKernel:
    xor eax, eax ;为了获取boot里面存放的 从硬盘读取的FAT等数据，需要将ds,es设置为0
    mov es, ax
    mov ds, ax

    movzx ax, byte[0x4000+16] ;FAT表数量
    mul word[0x4000+22] ;一个FAT扇区数
    add ax, word[0x4000+14] ;BPB_RsvdSecCnt boot占用扇区数
    add eax, ROOT_BASE
    mov dword[ROOT_DIR_START_SECTOR], eax ;存放根目录起始扇区

    mov word[disk_address_packer+4], 0x5000
    mov dword[disk_address_packer+8],eax ;要读的扇区号

    mov ax, ROOT_DIRECTORY_ENTRY_SIZE
    mul word[0x4000+17] ;根目录最大文件数
    mov dword[ROOT_DIR_SIZE], eax ;根目录总大小

    add eax, 512-1
    shr eax, 9 ;除以512
    mov dword[RootDirSectorCount], eax ;根目录占用扇区数
    mov dword[RootDirSectorCount1], eax ;根目录占用扇区数

    .ReadRootSectorLoop: ;循环读取所有根目录扇区
        mov dword[RootDirEntryCount], 0
        cmp dword[RootDirSectorCount], 0
        je DispKernelNotFound;如果到达了根目录最大条数, 说明没有找到 Loader, 则跳到未找到到提示
        call ReadSector ;读了一个扇区
        inc dword[disk_address_packer+8]

        mov bx, 0x5000
        .ReadRootEntryLoop: ;循环读取该扇区中的根目录条目
            call FindKernelFileName
            add bx, ROOT_DIRECTORY_ENTRY_SIZE ;移动到下一条目

            inc dword[RootDirEntryCount]
            movzx eax, word[0x4000+17]
            cmp dword[RootDirEntryCount], eax
            jne .ReadRootEntryLoop;如果没有到达了条目最大条数, 继续搜索下一个条目

        dec dword[RootDirSectorCount]
        jmp .ReadRootSectorLoop

ReadSector: ;ax传入要读取的扇区号
    mov ah, 0x42
    mov dl, 0x80
    mov si, disk_address_packer
    int 0x13
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
    xor ecx, ecx
    xor eax, eax

    mov bp, BASE_OF_KERNEL_FILE

    mov ds, ax
    mov cx, word[es:di] ;将开始簇号存入cx

    mov eax, dword[RootDirSectorCount1]
    add eax, dword[ROOT_DIR_START_SECTOR]
    mov dword[DataStartSector], eax

    mov bx, 0
    mov eax, ecx
    
    .Loading:
        push cx
        sub eax, 2
        mul byte[0x4000+13]
        add eax, dword[DataStartSector] ;某簇对应的数据在哪个扇区
        movzx cx, byte[0x4000+13]
.1:     
        mov dword[disk_address_packer+8], eax ;要读的扇区号
        mov word[disk_address_packer+4], bx
        mov byte[disk_address_packer+2], 1
        mov word[disk_address_packer+6], bp
        push eax
        call ReadSector
        pop eax
        inc eax
        dec cx
        add bx, word[0x4000+11]
        jc .2 ;如果bx重新变成0，说明内核大于64KB，需要段+1
.3:     cmp cx, 0
        jne .1

        xor eax, eax
        pop ax
        call GetFATEntry
        cmp ax, 0FFFh
        mov cx, ax
        jz InitProtectedModel
        jmp .Loading
.2:
    add bp, 0x1000
    jmp .3
.4:
    jmp $

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
        mov bx, word[0x4000+11]
        div bx
        push dx
        mov bx, 0
        ; add ax, [BPB_RsvdSecCnt]
        add ax, word[0x4000+14]
        add ax, ROOT_BASE
        mov dword[disk_address_packer+8], eax ;要读的扇区号
        mov word[disk_address_packer+4], bx
        mov word[disk_address_packer+6], FAT_ES
        mov byte[disk_address_packer+2], 2
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
    mov ax, SELF_CS
    mov ds, ax
    mov es, ax
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

    call SetupPaging
    mov ax, SELF_CS ;恢复段寄存器
    mov ds, ax
    mov es, ax
    
    lgdt [GdtPtr]

    cli

    in al, 92h
    or al, 00000010b
    out 92h, al

    mov eax, Page_Dir_Base
    mov cr3, eax
    mov eax, cr0
    or eax, 1
    or eax, 0x80000000
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

RootDirSectorCount equ ROOT_DIR_SIZE+4; db 0 ;根目录扇区循环次数, 每循环一次加一, 最大值为 ROOT_DIR_SECTORS
RootDirEntryCount equ RootDirSectorCount+4 ;db 0 ;根目录扇区的条目循环次数, 没读取一条根目录加一次, 最大值为 ROOT_DIR_ENTRY
RootDirSectorCount1 equ RootDirEntryCount+4 ;根目录扇区数
DataStartSector equ RootDirSectorCount1+4 ;数据区起始扇区
KernelFileName db 'KERNEL     ' ;Kernel文件名
KernelNotFoundMsg db 'Kernel not found'
KernelFoundMsg db 'Kernel found    '
bOdd equ DataStartSector + 4
disk_address_packer equ bOdd + 1

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

Page_Dir_Base equ 1000H
Page_Tbl_base equ 2000H

GDT_SEL_CODE equ gdt_code-gdt_0
GDT_SEL_DATA equ gdt_data-gdt_0
GDT_SEL_VIDEO equ gdt_video-gdt_0
; GDT_SEL_USER_CODE equ gdt_user_code-gdt_0
; GDT_SEL_USER_DATA equ gdt_user_data-gdt_0

MemSize dd 0

