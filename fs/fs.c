#include <system/fs.h>
#include <string.h>
#include "../kernel/hd.h"
#include "../kernel/kernel.h"
#include "../kernel/global.h"

struct file_descriptor f_desc_table[FILE_DESC_TABLE_MAX_COUNT];
struct inode inode_table[INODE_TABLE_MAX_COUNT];

void fat16_to_a(struct super_block *sb, struct fat16 *fat)
{
    sb->sector_count = fat->BPB_TotSec16;
    sb->dir_entry_count = fat->BPB_NumFATs;
}

void mkfs()
{
    struct part_ent *pt;

    unsigned char *fsbuf = (unsigned char *)0x600000;

    struct super_block sb;
    sb.sector_count = pt->nr_sects;
    sb.root_inode = 1;

    struct request a;
    a.buffer = fsbuf;
    a.bh = fsbuf;
    a.cmd = 1;
    a.dev = 0;
    a.next = NULL;
    a.nr_sectors = 1;
    a.sector = pt->start_sect;
  

}

int sys_open(const char *path, int flags, ...)
{
    int fd = -1;
    int i;
    for (i = 0; i < PROC_FILES_MAX_COUNT; i++) {
        if (current_process->file_table[i]==0) {
            fd = i;
            break;
        }
    }

    if (fd<0 || fd>=PROC_FILES_MAX_COUNT) {
        printk("file_table is full (PID:%d)\n", current_process->pid);
        return -1;
    }

    for (i = 0; i < FILE_DESC_TABLE_MAX_COUNT; i++) {
        if (f_desc_table[i].inode==0) {
            break;
        }
    }

    if (i>=FILE_DESC_TABLE_MAX_COUNT) {
        printk("f_Desc_table is full (PID:%d)\n", current_process->pid);
        return -1;
    }

    struct inode *p_inode = 0;

    if (get_inode_by_filename(path, p_inode)==0) {
        return -1;
    }

    current_process->file_table[fd] = &f_desc_table[i];
    f_desc_table[i].pos = 0;
    f_desc_table[i].inode = p_inode;

    if (p_inode->mode==FILE_MODE_CHR) {
        if (MAJOR(p_inode->dev_num)==4) { //tty
            current_process->tty = MINOR(p_inode->dev_num);
        }
    }

    return fd;
}

int get_inode_by_filename(const char *filename, struct inode *res_inode)
{
    if (strcmp(filename, "/tty1")==0)
    {
        res_inode = &inode_table[0];
        return 1;
    }
    return 0;
}