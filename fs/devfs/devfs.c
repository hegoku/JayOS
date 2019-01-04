
#include "devfs.h"
#include "../../kernel/global.h"
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <system/dev.h>
#include <system/fs.h>
#include <system/mm.h>
#include <system/list.h>
#include <math.h>

static struct dir_entry *dev_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte);
static int dev_lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir);

struct file_system_type dev_fs_type = {
    name: "devfs",
    mount: dev_mount,
    next: 0
};
struct file_operation dev_f_op={
    NULL,
    f_op_read,
    NULL,
    NULL,
    // 0,
    NULL,
    // 0,
    NULL,
    NULL,
    NULL
};
struct inode_operation dev_inode_op = {
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

void init_devfs()
{
    register_filesystem(&dev_fs_type);
}

struct dir_entry *dev_mount(struct file_system_type *fs_type, int dev_num)
{
    struct super_block *sb=get_block(0);
    fs_type->sb_table=sb;
    sb->fs_type = fs_type;
    sb->dev_num=dev_num;

    struct inode *new_inode=get_inode();
    new_inode->sb=sb;
    new_inode->dev_num=sb->dev_num;
    new_inode->f_op=&dev_f_op;
    new_inode->inode_op = &dev_inode_op;
    new_inode->mode = FILE_MODE_DIR;

    struct dir_entry *new_dir=get_dir();
    new_dir->name[0]='/';
    new_dir->dev_num=sb->dev_num;
    new_dir->inode=new_inode;
    new_dir->parent=new_dir;
    new_dir->sb=sb;

    sb->root_dir=new_dir;
    sb->root_inode=new_inode;

    return new_dir;
}

static int f_op_write(struct file_descriptor *fd, char *buf, int nbyte)
{
    return nbyte;
}

static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte)
{
    return nbyte;
}

int rootfs_lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir)
{
    return 0;
}