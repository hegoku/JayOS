
#include "fat.h"
#include <string.h>
#include <sys/types.h>
#include <system/dev.h>
#include <system/fs.h>

static struct dir_entry *fat12_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte);

struct file_system_type fat12_fs_type = {
    name: "fat12",
    mount: fat12_mount,
    next: 0
};
struct file_operation fat12_f_op={
    NULL,
    NULL,
    NULL,
    // 0,
    // 0,
    NULL,
    // 0,
    NULL,
    NULL,
    NULL
};
struct inode_operation fat12_inode_op = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

struct dir_entry *fat12_mount(struct file_system_type *fs_type, int dev_num)
{
    struct super_block *sb=get_block(0);
    sb->fs_type = fs_type;

    struct fat12_super_block fat12_sb;
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&fat12_sb, 0, sizeof(fat12_sb)); //读引导扇区
    if (fat12_sb.BS_FileSysType!="FAT12") {
        printk("Unknow system type\n");
        return NULL;
    }

    int fat_start = fat12_sb.BPB_BytsPerSec * fat12_sb.BPB_RsvdSecCnt; //FAT表开始字节
    int fat_start_sector = (fat_start + fat12_sb.BPB_BytsPerSec - 1) / fat12_sb.BPB_BytsPerSec; //FAT起始扇区
    int fat_size = fat12_sb.BPB_BytsPerSec * fat12_sb.BPB_FATz16;                                          //一个FAT表的大小(字节)
    int root_dir_start = fat12_sb.BPB_BytsPerSec*fat12_sb.BPB_RsvdSecCnt + fat_size *fat12_sb.BPB_NumFATs; //根目录开始字节
    int root_dir_start_sector = (root_dir_start + fat12_sb.BPB_BytsPerSec -1) / fat12_sb.BPB_BytsPerSec; //根目录起始扇区
    int root_dir_size = fat12_sb.BPB_RootEntCnt * ROOT_DIR_ENTRY_SIZE; //根目录总大小(字节)
    int root_dir_block_size = (root_dir_size + fat12_sb.BPB_BytsPerSec - 1) / fat12_sb.BPB_BytsPerSec; //根目录占用扇区数
    int data_start_sector = root_dir_block_size + root_dir_start_sector; //数据区起始扇区

    struct fat12_fat_entry fat_table[fat_size];
    memset(fat_table, 0, sizeof(fat_table));
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&fat_table, fat_start_sector, sizeof(fat_table)); //读根FAT表

    struct fat12_root_dir_entry root_dir_entry[fat12_sb.BPB_RootEntCnt];
    memset(root_dir_entry, 0, sizeof(root_dir_entry));
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&root_dir_entry, root_dir_start_sector, sizeof(root_dir_entry)); //读根目录

    struct inode *new_inode=get_inode();
    new_inode->sb=sb;
    new_inode->dev_num=sb->dev_num;
    new_inode->f_op=&fat12_f_op;
    new_inode->inode_op = &fat12_inode_op;
    new_inode->mode = FILE_MODE_DIR;

    struct dir_entry *new_dir=get_dir();
    new_dir->name[0]='/';
    new_dir->dev_num=sb->dev_num;
    new_dir->inode=new_inode;
    new_dir->parent=new_dir;
    new_dir->sb=sb;

    sb->root_dir=new_dir;
    sb->root_inode=new_inode;

    for (int i = 0; i < fat12_sb.BPB_RootEntCnt;i++) {
        struct inode *new_inode=get_inode();
        new_inode->sb=sb;
        new_inode->dev_num=sb->dev_num;
        new_inode->f_op=&fat12_f_op;
        new_inode->inode_op = &fat12_inode_op;
        if (root_dir_entry[i].dir_attr==0x54) {
            new_inode->mode = FILE_MODE_REG;
        } else if (root_dir_entry[i].dir_attr==0x20) {
            new_inode->mode = FILE_MODE_DIR;
        }
        new_inode->size = root_dir_entry[i].file_size;
        new_inode->start_pos = root_dir_entry[i].fst_clus;

        struct dir_entry *new_dir=get_dir();
        char tmp1[9];
        memset(tmp1, 0, 9);
        memcpy(tmp1, root_dir_entry[i].dir_name, 8);
        char tmp2[4];
        memset(tmp2, 0, 4);
        memcpy(tmp2, root_dir_entry[i].dir_name, 3);
        sprintf(new_dir->name, "%s.%s", tmp1, tmp2);
        new_dir->dev_num = sb->dev_num;
        new_dir->inode=new_inode;
        new_dir->parent=new_dir;
        new_dir->sb=sb;
    }

    return new_dir;
}

void init_fat12()
{
    register_filesystem(&fat12_fs_type);
}