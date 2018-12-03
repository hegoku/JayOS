
#ifndef __FAT_H
#define __FAT_H

struct fat16{
    unsigned char BS_jmpBoot : 3;
    unsigned char BS_OEMName;
    unsigned char BPB_BytsPerSec : 2;
    unsigned char BPB_SecPerClus : 1;
    unsigned char BPB_RsvdSecCnt : 2;
    unsigned char BPB_NumFATs : 1; //FAT表数量
    unsigned char BPB_RootEntCnt : 2; //更目录文件最大数
    unsigned char BPB_TotSec16 : 2; //扇区总数
    unsigned char BPB_Media : 1;
    unsigned char BPB_FATz16 : 2; //每个fat占用多少扇区
    unsigned char BPB_SecPerTrk : 2;
    unsigned char BPB_NumHeads : 4;
    unsigned char BPB_HiddSec : 4;
    unsigned char BPB_TotSec32 : 4; //如果BPB_TotSec16为0，由这个表述扇区总数
    unsigned char BS_DrvNum : 1;
    unsigned char BS_Reserved1 : 1;
    unsigned char BS_BootSig : 1;
    unsigned char BD_VolId : 4;
    unsigned short BS_VolLab : 11;
    unsigned char BS_FileSysType;
    unsigned char boot_code[56];
    unsigned char end_flag : 2;
};

void init_fat16();

#endif