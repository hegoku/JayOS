
#include "fat.h"
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

struct dir_entry *fat12_mount(struct file_system_type *fs_type, int dev_num)
{
    struct super_block *sb=get_block(0);

    struct fat16_super_block fat12_sb;
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&fat12_sb, 2, sizeof(fat12_sb)); //读引导扇区
}

void init_fat12()
{
    register_filesystem(&fat12_fs_type);
}