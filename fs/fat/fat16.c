
#include "fat.h"
#include <system/fs.h>

static struct super_block *fat16_read_super(struct file_system_type *fs_type);
static int f_op_write(struct inode *, struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte);

struct file_system_type fat16_fs_type = {
    name: "fat16",
    read_super: fat16_read_super,
    next: 0
};

struct super_block *fat16_read_super(struct file_system_type *fs_type)
{

}

void init_fat16()
{
    register_filesystem(&fat16_fs_type);
}