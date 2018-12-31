FAT_ES equ 0x26A0
ROOT_DIR_SECTORS equ 14 ;根目录占用多少扇区
SECTOR_NO_OF_ROOT_DIRECTORY equ 19 ;根目录第一个扇区号
ROOT_DIR_ENTRY equ 16 ;一个扇区有多少个根目录条目 512/32=16

ROOT_DIRECTORY_ENTRY_SIZE equ 32 ;目录一条条目占用字节
ROOT_DIRECTORY_ENTRY_ATTR_OFFSET equ 0xB ;目录条目 存放文件属性的偏离位置
ROOT_DIRECTORY_ENTRY_FSTCLUS_OFFSET equ 0x1A ;根目录条目 存放开始簇号的偏离位置
ROOT_DIRECTORY_ENTRY_FILESIZE_OFFSET equ 0x1C ;根目录条目 存放文件大小的偏离位置

BS_OEMName db 'JACK    ' ;OEM
BPB_BytsPerSec dw 512 ;每扇区字节数
BPB_SecPerClus db 1 ;每簇多少扇区
BPB_RsvdSecCnt dw 1 ;Boot记录占用多少扇区
BPB_NumFATs db 2 ;共有多少FAT表
BPB_RootEntCnt dw 224; 根目录文件数最大值
BPB_TotSec16 dw 2880 ;逻辑扇区数
BPB_Media db 0xf0 ;媒体描述符
BPB_FATSz16 dw 9 ;每FAT扇区数
BPB_SecPerTrk dw 18 ;每磁道扇区数，18个扇区
BPB_NumHeads dw 2 ;磁头数
BPB_HiddSec dd 0 ;隐藏扇区数
BPB_TotSec32 dd 0 ;
BS_DrvNum db 0 ;中断13的驱动器号
BS_Reserved1 db 0;
BS_BootSig db 29h ;引导扩展标记
BS_VolID dd 0 ;卷序列号
BS_VolLab db 'JayOS      '; 卷标
BS_FileSysType db 'FAT12   '