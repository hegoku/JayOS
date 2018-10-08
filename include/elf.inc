; ELF Header 32bit 各参数偏移量
ELF32_HDR_IDENT equ 0x0 ;
ELF32_HDR_TYPE equ 0x10 ;
ELF32_HDR_MACHINE equ 0x12 ;
ELF32_HDR_VER equ 0x14 ;
ELF32_HDR_ENTRY equ 0x18 ;入口地址
ELF32_HDR_PHOFF equ 0x1C ;Program header table 在文件中的偏移量
ELF32_HDR_SHOFF equ 0x20 ;Section header table 在文件中偏移量
ELF32_HDR_FLAGS equ 0x24
ELF32_HDR_EHSIZE equ 0x28 ;ELF header大小
ELF32_HDR_PHENTSOZE equ 0x2A ;Program header table 每个条目大小
ELF32_HDR_PHNUM equ 0x2C ;Program header table 有多少条目
ELF32_HDR_SHENTSIZE equ 0x2E ;Section header table中每条大小
ELF32_HDR_SHNUM equ 0x30 ;Section header table 有多少条目
ELF32_HDR_SHSTRNDX equ 0x32

;Program header 32bit 各参数偏移量
ELF32_PHDR_TYPE equ 0x0
ELF32_PHDR_OFFSET equ 0x4 ;段的第一个字节在文件中的偏移量
ELF32_PHDR_VADDR equ 0x8 ;段第一个字节在内存中的虚拟地址
ELF32_PHDR_PADDR equ 0xC ;段第一个字节在内存中的物理地址
ELF32_PHDR_FILESZ equ 0x10 ;段在文件中的长度
ELF32_PHDR_MEMSZ equ 0x14 ;段在内存中长度
ELF32_PHDR_FLAGS equ 0x18
ELF32_PHDR_ALIGN equ 0x1C
