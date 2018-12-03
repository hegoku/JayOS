
#include "fat.h"
#include <system/fs.h>

static struct dir_entry *fat16_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_write(struct inode *, struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte);

struct file_system_type fat16_fs_type = {
    name: "fat16",
    mount: fat16_mount,
    next: 0
};

struct dir_entry *fat16_mount(struct file_system_type *fs_type, int dev_num)
{

}

void init_fat16()
{
    register_filesystem(&fat16_fs_type);
}