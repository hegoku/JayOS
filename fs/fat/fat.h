
#ifndef __FAT_H
#define __FAT_H

//fat项占用位数
#define FAT12_FAT_ENTRY_SIZE 12
#define FAT16_FAT_ENTRY_SIZE 16
#define FAT32_FAT_ENTRY_SIZE 32

#define FAT12_MAX_CLUS_COUNT 4096
#define FAT16_MAX_CLUS_COUNT 65536
#define FAT32_MAX_CLUS_COUNT 268435456

#define ROOT_DIR_ENTRY_SIZE 32 //根目录一条大小(字节)

//一下为文件名的第一个字节的特殊值
#define FILE_NAME_0 0x00 //这个条目有用并且后面没有被占用条目
#define FILE_NAME_5 0x5 //最初字符确实是0xE5
#define FILE_NAME_DOT 0x2E //'点'条目；'.'或者'..'
#define FILE_NAME_DEL 0xE5 //这个条目曾经被删除不再有用。取消删除文件工具作为取消删除的一步必须使用一个正常的字符取代它

#define FILE_ATTR_READONLY_MASK 0x1 //只读
#define FILE_ATTR_HIDDEN_MASK 0x2 //隐藏
#define FILE_ATTR_SYS_MASK 0x4 //系统
#define FILE_ATTR_VOL_MASK 0x8 //卷标
#define FILE_ATTR_DIR_MASK 0x10 //目录
#define FILE_ATTR_FILE_MASK 0x20 //文件
#define FILE_ATTR_DEV_MASK 0x40 //设备（内部使用，磁盘上看不到）
#define FILE_ATTR_UNUSED 0x80 // 没有使用
#define FILE_ATTR_LONG_NAME_MASK 0x0F //长文件名

struct fat12_super_block{
    unsigned char BS_jmpBoot[3];
    unsigned char BS_OEMName[8];
    unsigned short BPB_BytsPerSec;
    unsigned short BPB_SecPerClus:8;
    unsigned short BPB_RsvdSecCnt;
    unsigned char BPB_NumFATs; //FAT表数量
    unsigned short BPB_RootEntCnt; //更目录文件最大数
    unsigned short BPB_TotSec16; //扇区总数
    unsigned char BPB_Media;
    unsigned short BPB_FATz16; //每个fat占用多少扇区
    unsigned short BPB_SecPerTrk;
    unsigned short BPB_NumHeads;
    unsigned int BPB_HiddSec;
    unsigned int BPB_TotSec32; //如果BPB_TotSec16为0，由这个表述扇区总数
    unsigned char BS_DrvNum;
    unsigned char BS_Reserved1;
    unsigned char BS_BootSig;
    unsigned int BD_VolId;
    unsigned char BS_VolLab[11];
    unsigned char BS_FileSysType[8];
    unsigned char boot_code[448];
    unsigned char end_flag[2];
} __attribute__ ((packed)) ;

struct fat12_root_dir_entry{
    unsigned char dir_name[8];
    unsigned char ext_name[3];
    unsigned char dir_attr;
    unsigned char reserved1[10];
    unsigned short write_time;
    unsigned short write_date;
    unsigned short fst_clus;
    unsigned int file_size;
} __attribute__ ((packed));

struct fat12_long_name{
    unsigned char count;
    unsigned short c1;
    unsigned short c2;
    unsigned short c3;
    unsigned short c4;
    unsigned short c5;
    unsigned char dir_attr;
    unsigned short reserved1;
    unsigned short c6; //16
    unsigned short c7;
    unsigned short c8;
    unsigned short c9;
    unsigned short c10;
    unsigned short c11;
    unsigned short reserved2;
    unsigned short c12;
    unsigned short c13;
} __attribute__((packed));

struct fat12_s_fs_info{
    int BPB_SecPerClus;
    int BPB_RsvdSecCnt;
    int BPB_BytsPerSec;
    int BPB_RootEntCnt;
    int fat_start;                                         //FAT表开始字节
    int fat_size;                                          //一个FAT表的大小(字节)
    int root_dir_start; //根目录开始字节
    int root_dir_start_sector; //根目录起始扇区
    int root_dir_size; //根目录总大小(字节)
    int root_dir_block_size; //根目录占用扇区数
    int data_start_sector; //数据区起始扇区
};

struct fat12_i_fs
{
    unsigned short fst_clus;
};

void init_fat12();

#endif