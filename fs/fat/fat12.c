
#include "fat.h"
#include "../../kernel/global.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <system/dev.h>
#include <system/fs.h>
#include <system/mm.h>
#include <system/list.h>

static struct dir_entry *fat12_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte);
static int getStringFromDate(char *dest, char *src, int len);
static int setLongNameToArray(struct fat12_long_name *long_name_buf, struct fat12_root_dir_entry *root_dir_entry, char *res);

struct file_system_type fat12_fs_type = {
    name: "fat12",
    mount: fat12_mount,
    next: 0
};
struct file_operation fat12_f_op={
    NULL,
    f_op_read,
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
    fs_type->sb_table=sb;
    sb->fs_type = fs_type;

    struct fat12_super_block *fat12_sb=kzmalloc(sizeof(struct fat12_super_block));
    memset(fat12_sb, 0, sizeof(struct fat12_super_block));
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)fat12_sb, 0, sizeof(struct fat12_super_block)); //读引导扇区
    char *fs_type_tmp=kzmalloc(9);
    memset(fs_type_tmp, 0, 9);
    getStringFromDate(fs_type_tmp, fat12_sb->BS_FileSysType, 8);
    if (strcmp(fs_type_tmp, "FAT12"))
    {
        printk("Not FAT12 File System Type\n");
        return NULL;
    }
    kfree(fs_type_tmp, 9);

    sb->s_fs_info = &fat12_sb;

    int fat_start = fat12_sb->BPB_BytsPerSec * fat12_sb->BPB_RsvdSecCnt; //FAT表开始字节
    int fat_start_sector = (fat_start + fat12_sb->BPB_BytsPerSec - 1) / fat12_sb->BPB_BytsPerSec; //FAT起始扇区
    int fat_size = fat12_sb->BPB_BytsPerSec * fat12_sb->BPB_FATz16;                                          //一个FAT表的大小(字节)
    int root_dir_start = fat12_sb->BPB_BytsPerSec*fat12_sb->BPB_RsvdSecCnt + fat_size *fat12_sb->BPB_NumFATs; //根目录开始字节
    int root_dir_start_sector = (root_dir_start + fat12_sb->BPB_BytsPerSec -1) / fat12_sb->BPB_BytsPerSec; //根目录起始扇区
    int root_dir_size = fat12_sb->BPB_RootEntCnt * ROOT_DIR_ENTRY_SIZE; //根目录总大小(字节)
    int root_dir_block_size = (root_dir_size + fat12_sb->BPB_BytsPerSec - 1) / fat12_sb->BPB_BytsPerSec; //根目录占用扇区数
    int data_start_sector = root_dir_block_size + root_dir_start_sector; //数据区起始扇区

    // struct fat12_fat_entry fat_table[fat_size];
    // memset(fat_table, 0, sizeof(fat_table));
    // dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&fat_table, fat_start_sector, sizeof(fat_table)); //读根FAT表

    // struct fat12_root_dir_entry root_dir_entry[fat12_sb->BPB_RootEntCnt];
    int rde_size = sizeof(struct fat12_root_dir_entry) * fat12_sb->BPB_RootEntCnt;
    struct fat12_root_dir_entry *root_dir_entry = kzmalloc(rde_size);
    // memset(root_dir_entry, 0, sizeof(root_dir_entry));
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)root_dir_entry, root_dir_start_sector, rde_size); //读根目录

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

    char *filename=kzmalloc(256);
    // struct fat12_root_dir_entry *root_dir_entry=kzmalloc(sizeof(struct fat12_root_dir_entry));
    char *tmp1=kzmalloc(9);
    char *tmp2 = kzmalloc(4);
    struct fat12_long_name *long_name_buf=kzmalloc(sizeof(struct fat12_long_name));
    printk("%x %x\n", &root_dir_entry[0], &root_dir_entry[1]);
    struct list *tmp;
    struct dir_entry *c_new_dir;
    for (int i = 0; i < fat12_sb->BPB_RootEntCnt; i++)
    {
        // memset(root_dir_entry, 0, sizeof(struct fat12_root_dir_entry));
        memset(filename, 0, sizeof(256));

        // dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)root_dir_entry, root_dir_start_sector, sizeof(struct fat12_root_dir_entry)); //读根目录
        // // printk("%d\n", root_dir_start_sector);
        // root_dir_start = sizeof(struct fat12_root_dir_entry);
        // root_dir_start_sector = (root_dir_start + fat12_sb->BPB_BytsPerSec -1) / fat12_sb->BPB_BytsPerSec;
        // if (i==2) {
        //     printk("%x\n", &root_dir_entry[i]);
        //     while(1){}
        // }

        if (root_dir_entry[i].dir_attr==0 || root_dir_entry[i].dir_attr==0x80 || root_dir_entry[i].dir_name[0]==FILE_NAME_DOT || root_dir_entry[i].dir_name[0]==FILE_NAME_DEL) {
            continue;
        }

        if ((root_dir_entry[i].dir_attr & FILE_ATTR_LONG_NAME_MASK) == FILE_ATTR_LONG_NAME_MASK)
        {
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
            若长文件名长度大于13个字符，则长文件名占用多个长文件名目录项。第一个目录项的序号为ASCII码01，第二项的序号为ASCII码02，......，最后一项的序号采用公式CHR(X-1+字母“A”的ASCII码）确定（一定是个英文字母）。
            其中X表示长文件名占用的目录项数，其计算方法如下：
            L=长文件名/13
            若L是整数，则X=L;若L不是整数，则X=（取L的整数部分+1）
            例如，文件名为“123456789abcdefghijk.txt”的长文件名目录项和对应的短文件名目录项为：
            （长文件名目录项2）：
            42 65 00 66 00 67 00 68-00 69 00 0F 00 2A 6A 00 Be.f.g.h.i...*j.
            6B 00 2E 00 74 00 78 00-74 00 00 00 00 00 FF FF k...t.x.t.....
            （长文件名目录项1）：
            01 31 00 32 00 33 00 34-00 35 00 0F 00 2A 36 00 .1.2.3.4.5...*6.
            37 00 38 00 39 00 61 00-62 00 00 00 63 00 64 00 7.8.9.a.b...c.d.
            （短文件名目录项）：
            31 32 33 34 35 36 7E 31-54 58 54 20 00 6C 69 60 123456～1TXT.li`
            A6 2A A6 2A 00 00 50 60-A6 2A 02 00 9C 00 00 00 .*.*..P`.*....
            用DIR命令列表显示为：
            123456～1 TXT 156 05-06-01 12:02 123456789abcdefghijk.txt
            在查看长文件名的目录项的时候，应按照图6-11所示的说明，注意观察长文件名每个目录项中的第一个字节。例如，上述“（长文件名目录项2）”中的“42”，表示该项为第2项，且为最后一个目录项。
            */
            int x = root_dir_entry[i].dir_name[0] - 'A' + 1;
            int y = x;
            char name_tmp[x][14];

            while (y)
            {
                setLongNameToArray(long_name_buf, &root_dir_entry[i], name_tmp[y - 1]);
                i++;
                y--;

                // memset(root_dir_entry, 0, sizeof(struct fat12_root_dir_entry));
                // dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)root_dir_entry, root_dir_start_sector, sizeof(struct fat12_root_dir_entry)); //读根目录
                // root_dir_start = sizeof(struct fat12_root_dir_entry);
                // root_dir_start_sector = (root_dir_start + fat12_sb->BPB_BytsPerSec -1) / fat12_sb->BPB_BytsPerSec;
                // printk("%x\n", root_dir_entry);while(1){}
            }
            char *name_p;
            name_p = filename;
           
            for (int j =0; j <x; j++)
            {
                sprintf(name_p, "%s", name_tmp[j]);
                name_p += 13;
            }
        }
        else
        {
            // char tmp1[9];
            memset(tmp1, 0, 9);
            getStringFromDate(tmp1, root_dir_entry[i].dir_name, 8);
            // memcpy(tmp1, &root_dir_entry[i].dir_name, 8);
            // char tmp2[4];
            memset(tmp2, 0, 4);
            getStringFromDate(tmp2, root_dir_entry[i].ext_name, 3);
            if ((root_dir_entry[i].dir_attr & FILE_ATTR_FILE_MASK) == FILE_ATTR_FILE_MASK)
            {
                sprintf(filename, "%s.%s", tmp1, tmp2);
            } else if ((root_dir_entry[i].dir_attr & FILE_ATTR_DIR_MASK) == FILE_ATTR_DIR_MASK) {
                sprintf(filename, "%s", tmp1, tmp2);
            }
        }

        new_inode=get_inode();
        new_inode->sb=sb;
        new_inode->dev_num=sb->dev_num;
        new_inode->f_op=&fat12_f_op;
        new_inode->inode_op = &fat12_inode_op;
        if ((root_dir_entry[i].dir_attr & FILE_ATTR_FILE_MASK) == FILE_ATTR_FILE_MASK)
        {
            new_inode->mode = FILE_MODE_REG;
        } else if ((root_dir_entry[i].dir_attr & FILE_ATTR_DIR_MASK) == FILE_ATTR_DIR_MASK) {
            new_inode->mode = FILE_MODE_DIR;
        }
        new_inode->size = root_dir_entry[i].file_size;
        new_inode->start_pos = root_dir_entry[i].fst_clus;

        c_new_dir=get_dir();
        
        sprintf(c_new_dir->name, "%s", filename);
        c_new_dir->dev_num = sb->dev_num;
        c_new_dir->inode=new_inode;
        c_new_dir->parent=new_dir;
        c_new_dir->sb=sb;

        tmp = create_list((void*)c_new_dir);
        list_add(tmp, &new_dir->children);
        // printk("%s %d\n", c_new_dir->name, i);
        // if (i>100)while(1){}
        // struct list *tmp = create_list();
        // tmp->value = c_new_dir;
        // tmp->next = new_dir->children;
        // new_dir->children = tmp;
    }
    kfree(long_name_buf, sizeof(struct fat12_long_name));
    kfree(tmp2, 4);
    kfree(tmp1, 9);
    // kfree(root_dir_entry, sizeof(struct fat12_root_dir_entry));
    kfree(filename, 256);
    kfree(root_dir_entry, rde_size);

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

//将root_dir_entry长文件名的一段取出给res
int setLongNameToArray(struct fat12_long_name *long_name_buf, struct fat12_root_dir_entry *root_dir_entry, char *res)
{
    memset(long_name_buf, 0, sizeof(struct fat12_long_name));
    memcpy(long_name_buf, root_dir_entry, sizeof(struct fat12_long_name));
    sprintf(res, "%c%c%c%c%c%c%c%c%c%c%c%c%c", long_name_buf->c1, long_name_buf->c2, long_name_buf->c3, long_name_buf->c4,
            long_name_buf->c5, long_name_buf->c6, long_name_buf->c7, long_name_buf->c8, long_name_buf->c9, long_name_buf->c10, long_name_buf->c11,
            long_name_buf->c12, long_name_buf->c13);
}

int f_op_read(struct file_descriptor *fd, char *buf, int nbyte)
{
    struct fat12_super_block *fat12_sb = (struct fat12_super_block*)fd->inode->sb->s_fs_info;

    int fat_start = fat12_sb->BPB_BytsPerSec * fat12_sb->BPB_RsvdSecCnt; //FAT表开始字节
    int fat_size = fat12_sb->BPB_BytsPerSec * fat12_sb->BPB_FATz16; //一个FAT表的大小(字节)
    int root_dir_start = fat12_sb->BPB_BytsPerSec*fat12_sb->BPB_RsvdSecCnt + fat_size *fat12_sb->BPB_NumFATs; //根目录开始字节
    int root_dir_size = fat12_sb->BPB_RootEntCnt * ROOT_DIR_ENTRY_SIZE; //根目录总大小(字节)
    int data_start = root_dir_start + root_dir_size; //数据区起始字节

    // unsigned long start_block = ((fd->inode->start_pos - 2) * fat12_sb->BPB_SecPerClus+data_start_sector);
    // unsigned long end_block = (fd->pos + nbyte) / dev_table[MAJOR(fd->inode->dev_num)].block_size;
    // int len = (end_block - start_block + 1) * dev_table[MAJOR(fd->inode->dev_num)].block_size;
    // dev_table[MAJOR(fd->inode->dev_num)].f_op->read(fd->inode->dev_num, 0, buf, start_block, len);
    // dev_table[MAJOR(fd->inode->dev_num)].request_fn(fd->inode->dev_num, 0, buf, start_block, len);
    fd->pos += nbyte;
    return nbyte;
}