#include <system/fs.h>
#include <system/rootfs.h>
#include <string.h>

static struct super_block *ramfs_read_super(struct file_system_type *fs_type);
static int f_op_write(struct inode *, struct file_descriptor *fd, char *buf, int nbyte);
static int f_op_read(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte);

static int i_op_lookup(struct inode *base, const char *path, int len, struct inode **res_inode);

struct file_system_type rootfs_fs_type = {
    name: "rootfs",
    read_super: ramfs_read_super,
    next: 0
};
struct file_operation rootfs_f_op={
    0,
    f_op_read,
    f_op_write,
    // 0,
    // 0,
    0,
    // 0,
    0,
    0,
    0
};
struct inode_operation rootfs_inode_op = {
    0,
    i_op_lookup,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};

static struct super_block *ramfs_read_super(struct file_system_type *fs_type)
{
    struct super_block *sb=get_block(0);
    
    struct inode *new_inode=get_inode();
    new_inode->sb=sb;
    new_inode->dev_num=sb->dev_num;
    new_inode->f_op=&rootfs_f_op;
    new_inode->inode_op = &rootfs_inode_op;
    new_inode->mode = FILE_MODE_DIR;

    struct dir_entry *new_dir=get_dir();
    new_dir->name[0]='/';
    new_dir->dev_num=sb->dev_num;
    new_dir->inode=new_inode;
    new_dir->parent=new_dir;
    new_dir->sb=sb;

    sb->root_dir=new_dir;
    sb->root_inode=new_inode;

    return sb;
}

void init_rootfs()
{
    register_filesystem(&rootfs_fs_type);
}

static int f_op_write(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte)
{
    return nbyte;
}

static int f_op_read(struct inode *inode, struct file_descriptor *fd, char *buf, int nbyte)
{
    return nbyte;
}

int i_op_lookup (struct inode *base, const char *path, int len,struct inode **res_inode)
{
    return 0;
}

// int i_op_mkdir (struct inode *, struct dentry *, int umode)
// {
//     struct inode *new_inode=get_inode();
//     new_inode->sb=inode->sb;
//     new_inode->dev_num=inode->dev_num;
//     new_inode->f_op=&rootfs_f_op;
//     new_inode->inode_op = &rootfs_inode_op;
//     new_inode->mode = FILE_MODE_DIR;

//     struct dir_entry *new_dir=get_dir();
//     memcpy(new_dir->name, name, len);
//     new_dir->dev_num=inode->dev_num;
//     new_dir->inode=new_inode;
//     new_dir->parent=new_dir;
//     new_dir->sb=inode->sb;
// }