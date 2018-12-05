
#ifndef __FAT_H
#define __FAT_H

#define ROOT_DIR_ENTRY_SIZE 32 //根目录一条大小(字节)
#define FILE_ATTR_DIR_MASK 0x10 //目录
#define FILE_ATTR_FILE_MASK 0x20 //文件
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

struct fat12_fat_entry{
    unsigned short entry : 12;
};

void init_fat12();

#endif