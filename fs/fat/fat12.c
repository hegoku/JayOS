
#include "fat.h"
#include "../../kernel/global.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <system/dev.h>
#include <system/fs.h>

static struct dir_entry *fat12_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte);
static int getStringFromDate(char *dest, char *src, int len);

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

void init_fat12()
{
    register_filesystem(&fat12_fs_type);
}

struct dir_entry *fat12_mount(struct file_system_type *fs_type, int dev_num)
{
    struct super_block *sb=get_block(0);
    sb->fs_type = fs_type;

    struct fat12_super_block fat12_sb;
    memset(&fat12_sb, 0, sizeof(fat12_sb));
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&fat12_sb, 0, sizeof(fat12_sb)); //读引导扇区
    char fs_type_tmp[9];
    memset(fs_type_tmp, 0, 9);
    getStringFromDate(fs_type_tmp, fat12_sb.BS_FileSysType, 8);
    if (strcmp(fs_type_tmp, "FAT12"))
    {
        printk("Not FAT12 File System Type\n");
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

    // struct fat12_fat_entry fat_table[fat_size];
    // memset(fat_table, 0, sizeof(fat_table));
    // dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&fat_table, fat_start_sector, sizeof(fat_table)); //读根FAT表

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

    // for (int i = 0; i < fat12_sb.BPB_RootEntCnt; i++)
    for (int i = 0; i < 10; i++)
    {
        if (root_dir_entry[i].dir_attr & FILE_ATTR_LONG_NAME_MASK == FILE_ATTR_LONG_NAME_MASK) {
            /*若长文件名长度小于等于13个字，则长文件名仅占用一个长文件名目录项，并且第一个字节为“A”（第6位是“1”），表明该目录项既是第一个又是最后一个
            例如，文件名为“Forest.bmp”的长文件名目录项和对应的短文件名目录项为：
            （长文件名目录项）：
            41 46 00 6F 00 72 00 65-00 73 00 0F 00 91 74 00 AF.o.r.e.s....t.
            2E 00 62 00 6D 00 70 00-00 00 00 00 FF FF FF FF ..b.m.p........
            （短文件名目录项）：
            46 4F 52 45 53 54 20 20-42 4D 50 20 00 00 00 00 FOREST BMP...
            00 00 CA 20 00 00 40 4E-88 1F 10 24 62 02 01 00 ... ..@N...$b...
            用DIR命令列表显示为：
            FOREST BMP 66,146 12-08-95 9:50 Forest.bmp
            */
            if (root_dir_entry[i].dir_name[0]=='A') {
                i++; //所以直接取下一个dir_entry, 因为真实的文件信息存在下一个里面
            } else {

            }
        }

        new_inode=get_inode();
        new_inode->sb=sb;
        new_inode->dev_num=sb->dev_num;
        new_inode->f_op=&fat12_f_op;
        new_inode->inode_op = &fat12_inode_op;
        printk("%x %x\n", root_dir_entry[i].dir_attr, root_dir_entry[i].dir_attr & FILE_ATTR_DIR_MASK);
        if ((root_dir_entry[i].dir_attr & FILE_ATTR_FILE_MASK) == FILE_ATTR_FILE_MASK)
        {
            new_inode->mode = FILE_MODE_REG;
        } else if ((root_dir_entry[i].dir_attr & FILE_ATTR_DIR_MASK) == FILE_ATTR_DIR_MASK) {
            new_inode->mode = FILE_MODE_DIR;
        }
        new_inode->size = root_dir_entry[i].file_size;
        new_inode->start_pos = root_dir_entry[i].fst_clus;

        struct dir_entry *c_new_dir=get_dir();
        char tmp1[9];
        memset(tmp1, 0, 9);
        getStringFromDate(tmp1, root_dir_entry[i].dir_name, 8);
        // memcpy(tmp1, &root_dir_entry[i].dir_name, 8);
        char tmp2[4];
        memset(tmp2, 0, 4);
        getStringFromDate(tmp2, root_dir_entry[i].ext_name, 3);
        // memcpy(tmp2, &root_dir_entry[i].ext_name, 3);
        if (new_inode->mode == FILE_MODE_REG) {
            sprintf(c_new_dir->name, "%s.%s", tmp1, tmp2);
        } else {
            sprintf(c_new_dir->name, "%s", tmp1, tmp2);
        }
        c_new_dir->dev_num = sb->dev_num;
        c_new_dir->inode=new_inode;
        c_new_dir->parent=new_dir;
        c_new_dir->sb=sb;

        struct list *tmp = create_list();
        tmp->value = c_new_dir;
        tmp->next = new_dir->children;
        new_dir->children = tmp;
    }

    return new_dir;
}

//截取字符串开始到第一个空格的串
int getStringFromDate(char *dest, char *src, int len)
{
    char *tail;
    tail = src + len-1;
    int act_len = 0;
    int i = 0;
    while (tail != src)
    {
        if (*tail!=' ') {
            break;
        }
        tail--;
        i++;
    }
    act_len = len - i;
    memcpy(dest, src, act_len);
    return 0;
}