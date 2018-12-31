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
ROOT_BASE equ 0x2801 ;分区起始扇区
ROOT_DIR_START_SECTOR equ 0x3000 ;存放根目录起始扇区的地址 dword
ROOT_DIR_SIZE equ ROOT_DIR_START_SECTOR + 4 ;根目录总字节数


jmp start
nop

%include "include/fat12hdr.inc"

start:
    mov ax,cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, BASE_OF_STACK
    mov byte[disk_address_packer], 0x10
    mov byte[disk_address_packer+2], 0x1 
    ; call ClearScreen
    ;读取fat16的super block
    mov dword[disk_address_packer+8], ROOT_BASE ;要读的扇区号
    mov word[disk_address_packer+4], 0x4000
    mov word[disk_address_packer+6], SELF_ES
    call ReadSector
    ; call ReadBoot
    ; call FindLoader

; ReadBoot:
;     mov dword[disk_address_packer+8], ROOT_BASE ;要读的扇区号
;     mov word[disk_address_packer+4], 0x4000
;     mov word[disk_address_packer+6], SELF_ES
;     call ReadSector
;     ret

FindLoader:
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
        ; movzx ax, [RootDirSectorCount]
        cmp dword[RootDirSectorCount], 0
        je DispLoaderNotFound;如果到达了根目录最大条数, 说明没有找到 Loader, 则跳到未找到到提示
        ; mov dword[disk_address_packer+8], 0x2801 ;要读的扇区号
        ; mov ax, SECTOR_NO_OF_ROOT_DIRECTORY ;要读的扇区号
        ; mov cl, 1 ;一次读几个扇区
        call ReadSector ;读了一个扇区
        ;下一个扇区
        ; mov eax, dword[disk_address_packer+8]
        inc dword[disk_address_packer+8]
        ; mov dword[disk_address_packer+8], eax

        mov bx, 0x5000
        .ReadRootEntryLoop: ;循环读取该扇区中的根目录条目
            call FindLoaderFileName
            add bx, ROOT_DIRECTORY_ENTRY_SIZE ;移动到下一条目

            inc dword[RootDirEntryCount]
            movzx eax, word[0x4000+17]
            ; cmp [RootDirEntryCount], ROOT_DIR_ENTRY
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

    ; mov bl, [BPB_SecPerTrk]
    ; div bl
    ; mov ch, al ;柱面
    ; shr ch, 1
    ; mov dh, al ;磁头号
    ; and dh, 1
    ; ; mov dl, [BS_DrvNum] ;启动器号
    ; mov dl, 7 ;启动器号
    ; mov cl, ah ;起始扇区号
    ; inc cl
    ; pop bx
    ; .GoOnReading:
    ;     mov ah, 2
    ;     mov al, byte[bp-2]
    ;     int 13h
    ;     jc .GoOnReading
    ;     add esp, 2
    ;     pop bp
    ;     ret

FindLoaderFileName:
    ; push ds
    push si
    push di
    ; push es
    push ax

    ; mov ax, BASE_OF_LOADER
    ; mov ax, SELF_ES
    ; mov es, ax

    mov cx, 11
    ; mov ax, SELF_ES
    ; mov ds, ax
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
    pop ax
    ; pop es
    pop di
    pop si
    ; pop ds
    ret

BootLoader:
    sub di, 11 ;因为找Loader的时候执行了11次 inc di，所以为了复位loader所在的根目录条目，减去11
    add di, ROOT_DIRECTORY_ENTRY_FSTCLUS_OFFSET ;找到开始簇号, 占用2个字节
    xor ecx, ecx
    xor eax, eax
    mov cx, word[es:di] ;将开始簇号存入cx
    push cx

    mov eax, dword[RootDirSectorCount1]
    add eax, dword[ROOT_DIR_START_SECTOR]
    mov dword[DataStartSector], eax

    ; mov eax, dword[RootDirSectorCount1]
    ; add eax, dword[ROOT_DIR_START_SECTOR]
    ; sub 2
    ; add eax

    ; mov eax, ecx
    ; sub eax, 2
    ; mul byte[0x4000+13]
    ; add eax, dword[DataStartSector] ;某簇对应的数据在哪个扇区


    ; mov ecx, dword[RootDirSectorCount1]
    ; add ecx, dword[ROOT_DIR_START_SECTOR]
    ; sub ecx, 2
    ; add ecx, eax
    ; mov ax, BASE_OF_LOADER
    ; mov es, ax
    ; mov ebx, OFFSET_OF_LOADER
    ; mov eax, ecx

    mov bx, 0
    mov eax, ecx
    ; mov dword[disk_address_packer+4], OFFSET_OF_LOADER
    ; mov dword[disk_address_packer+6], BASE_OF_LOADER
    .Loading:
        ; mov cl, 1
        ; mov eax, ecx
        ; sub eax, 2
        ; mul byte[0x4000+13]
        ; add eax, dword[DataStartSector] ;某簇对应的数据在哪个扇区
        ; mov dword[disk_address_packer+8], eax ;要读的扇区号
        ; ; mov word[disk_address_packer+4], 0x5000
        ; mov dword[disk_address_packer+6], BASE_OF_LOADER
        ; mov eax, ecx
        ; mul dword[]
        ; mov byte[disk_address_packer+2], 1
        ; call ReadSector
        ; pop ax
        ; call GetFATEntry
        ; cmp ax, 0FFFh
        ; jz JmpToLoader
        ; push ax
        ; mov edx, dword[RootDirSectorCount1]
        ; add edx, dword[ROOT_DIR_START_SECTOR]
        ; sub edx, 2
        ; ; mov dx, ROOT_DIR_SECTORS+SECTOR_NO_OF_ROOT_DIRECTORY-2
        ; add eax, edx
        ; add ebx, [BPB_BytsPerSec]
        ; jmp .Loading
        sub eax, 2
        mul byte[0x4000+13]
        add eax, dword[DataStartSector] ;某簇对应的数据在哪个扇区
        movzx cx, byte[0x4000+13]
.1:     
        mov dword[disk_address_packer+8], eax ;要读的扇区号
        mov word[disk_address_packer+4], bx
        mov byte[disk_address_packer+2], 1
        mov word[disk_address_packer+6], BASE_OF_LOADER
        push eax
        call ReadSector
        pop eax
        inc eax
        dec cx
        add bx, [BPB_BytsPerSec]
        cmp cx, 0
        jne .1

        pop ax
        call GetFATEntry
        cmp ax, 0FFFh
        jz JmpToLoader
        ; push ax
        ; mov dx, ROOT_DIR_SECTORS+SECTOR_NO_OF_ROOT_DIRECTORY-2
        ; add ax, dx
        ; add bx, [BPB_BytsPerSec]
        jmp .Loading

GetFATEntry:
    ; push es
    push bx
    ; push ax
    ; mov ax, FAT_ES
    ; mov es, ax
    ; pop ax
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
        ; mov bx, [BPB_BytsPerSec]
        mov bx, word[0x4000+11]
        div bx
        push dx
        mov bx, 0x5000
        ; add ax, [BPB_RsvdSecCnt]
        add ax, word[0x4000+14]
        mov dword[disk_address_packer+8], eax ;要读的扇区号
        mov word[disk_address_packer+4], bx
        mov word[disk_address_packer+6], FAT_ES
        mov byte[disk_address_packer+2], 2
        call ReadSector

        pop dx
        add bx, dx
        ; add word[disk_address_packer+4], dx
        mov ax, [es:bx]
        ; mov ax, word[disk_address_packer+4]
        cmp byte[bOdd], 1
        jnz .Label_even_2
        shr ax, 4
    .Label_even_2:
        and ax, 0FFFh

    pop bx
    ; pop es
    ret

JmpToLoader:
    jmp BASE_OF_LOADER:OFFSET_OF_LOADER

DispLoaderNotFound:
    ; mov ax, LoaderFileName
    mov bp, LoaderFileName
    ; mov ax, SELF_ES
    ; mov es, ax
    mov cx, 20
    mov ax, 01301h
    mov bx, 0007h
    mov dl, 0
    int 10h
    ; call DispStr
    jmp $

; DispStr: ;显示字符串, ax作为字符串地址
;     push es
;     push bx
;     mov bp, ax
;     mov ax, SELF_ES
;     mov es, ax
;     mov cx, 20
;     mov ax, 01301h
;     mov bx, 0007h
;     mov dl, 0
;     int 10h
;     pop bx
;     pop es
;     ret

; ClearScreen:
;     mov ax, 0600h
;     mov bx, 0700h
;     mov cx, 0
;     mov dx, 0184fh
;     int 10h
;     mov ax, 0200h
;     mov bh, 0
;     mov dx, 0
;     int 10h
;     ret

RootDirSectorCount equ ROOT_DIR_SIZE+4; db 0 ;根目录扇区循环次数, 每循环一次加一, 最大值为 ROOT_DIR_SECTORS
RootDirEntryCount equ RootDirSectorCount+4 ;db 0 ;根目录扇区的条目循环次数, 没读取一条根目录加一次, 最大值为 ROOT_DIR_ENTRY
RootDirSectorCount1 equ RootDirEntryCount+4 ;根目录扇区数
DataStartSector equ RootDirSectorCount1+4 ;数据区起始扇区
LoaderFileName db 'LOADER     ' ;Loader文件名
LoaderNotFoundMsg db 'not found'
bOdd equ DataStartSector + 4
; bOdd db 0 ;是奇数数还是偶数

; BPB_SecPerClus_hd: db 0
; BPB_RsvdSecCnt_hd: dw 1
; BPB_RootEntCnt_hd: dw 0

disk_address_packer equ bOdd+1
; disk_address_packer: db 0x10
;                      db 0
;                      db 1
;                      db 0
;                      dw OFFSET_OF_LOADER ;offset
;                      dw BASE_OF_LOADER ;seg
;                      dd 0
;                      dd 0
times 510-($-$$) db 0
dw 0xaa55