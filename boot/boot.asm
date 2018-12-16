bits 16
org 07c00h

;
; 500H     ------------
;          |   free   | 29.75K
; 7C00H    ------------
;          |   boot   | 512
; 7E00H    ------------
;          |  loader  | 59K
; 16A00H   ------------
;          |   FAT    | 5K
; 17E00H   ------------
;          |  kernel  | 64K
; 27E00H   ------------
;          |  KERNEL  |
;
;
; 500H     ------------
;          |   free   | 2.75K
; 1000H    ------------
;          | page_dir | 4K
; 2000H    ------------
;          |page_entr0| 4K
; 3000H    ------------
;          |   free   | 19K
; 7C00H    ------------
;          |   boot   | 512
; 7E00H    ------------
;          |  KERNEL  | 64K
; 17E00H   ------------
;          |  loader  | 59K
; 26A00H   ------------
;          |   FAT    | 5K
; 27E00H   ------------
;          |  kernel  | 64K
; 37E00    ------------

SELF_ES equ 0x0 ;自身的段基址
BASE_OF_STACK equ 0x7C00 ;堆栈基地址
BASE_OF_LOADER equ 0x17E0 ;13h中断取出的数据存放的段地址 es, Loader基址
OFFSET_OF_LOADER equ 0x0 ;13h中断取出的数据存放的偏移地址 bx

jmp start
nop

%include "include/fat12hdr.inc"

start:
    mov ax,cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BASE_OF_STACK
    call ClearScreen
    call FindLoader
    jmp $

FindLoader:
    xor ah, ah
    xor dl, dl
    int 13h

    .ReadRootSectorLoop: ;循环读取所有根目录扇区
        mov byte[RootDirEntryCount], 0
        movzx ax, [RootDirSectorCount]
        cmp ax, ROOT_DIR_SECTORS
        je DispLoaderNotFound;如果到达了根目录最大条数, 说明没有找到 Loader, 则跳到未找到到提示
        mov ax, BASE_OF_LOADER
        mov es, ax
        mov bx, OFFSET_OF_LOADER
        mov ax, SECTOR_NO_OF_ROOT_DIRECTORY ;要读的扇区号
        mov cl, 1 ;一次读几个扇区
        call ReadSector ;读了一个扇区
        .ReadRootEntryLoop: ;循环读取该扇区中的根目录条目
            call FindLoaderFileName        
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

FindLoaderFileName:
    push ds
    push si
    push di
    mov cx, 11
    mov ax, SELF_ES
    mov ds, ax
    mov si, LoaderFileName
    mov di, bx
    cld
.CompareChar:
    cmp cx, 0
    jz BootLoader ;如果11个字符都相等, 说明找到了
    dec cx
    lodsb
    cmp al, byte[es:di]
    jz .CompareNextChar
    jmp .FindLoaderFileNameRet ;如果有一个字符不一样说明不是要找的
.CompareNextChar:
    inc di
    jmp .CompareChar
.FindLoaderFileNameRet:
    pop di
    pop si
    pop ds
    ret

BootLoader:
    sub di, 11 ;因为找Loader的时候执行了11次 inc di，所以为了复位loader所在的根目录条目，减去11
    add di, ROOT_DIRECTORY_ENTRY_FSTCLUS_OFFSET ;找到开始簇号, 占用2个字节
    mov cx, word[es:di] ;将开始簇号存入cx
    push cx
    mov ax, ROOT_DIR_SECTORS+SECTOR_NO_OF_ROOT_DIRECTORY-2
    add cx, ax
    mov ax, BASE_OF_LOADER
    mov es, ax
    mov bx, OFFSET_OF_LOADER
    mov ax, cx
    .Loading:
        mov cl, 1
        call ReadSector
        pop ax
        call GetFATEntry
        cmp ax, 0FFFh
        jz JmpToLoader
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

JmpToLoader:
    jmp BASE_OF_LOADER:OFFSET_OF_LOADER

DispLoaderNotFound:
    mov ax, LoaderNotFoundMsg
    call DispStr
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

RootDirSectorCount db 0 ;根目录扇区循环次数, 每循环一次加一, 最大值为 ROOT_DIR_SECTORS
RootDirEntryCount db 0 ;根目录扇区的条目循环次数, 没读取一条根目录加一次, 最大值为 ROOT_DIR_ENTRY
LoaderFileName db 'LOADER     ' ;Loader文件名
LoaderNotFoundMsg db 'Loader not found'
LoaderFoundMsg db 'Loader found    '
bOdd db 0 ;是奇数数还是偶数
times 510-($-$$) db 0
dw 0xaa55