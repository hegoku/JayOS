#include <system/fs.h>
#include <sys/types.h>
#include <system/dev.h>
#include <string.h>
#include "ext2.h"
#include "../../kernel/global.h"

static struct dir_entry *ext2_mount(struct file_system_type *fs_type, int dev_num);
static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte);
static int ext2_lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir);

struct file_system_type ext2_fs_type = {
    name : "ext2",
    mount : ext2_mount,
    next : NULL
};
struct file_operation ext2_f_op={
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
struct inode_operation ext2_inode_op = {
    NULL,
    ext2_lookup,
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

void init_ext2()
{
    register_filesystem(&ext2_fs_type);
}

static struct dir_entry *ext2_mount(struct file_system_type *fs_type, int dev_num)
{
    struct super_block *sb=get_block(0);
    sb->fs_type = fs_type;

    struct ext2_super_block ext2_sb;
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&ext2_sb, 2, sizeof(ext2_sb)); //读super_block
    if (ext2_sb.s_magic!=0xEF53) {
        printk("Not Ext2 File System Type\n");
        return NULL;
    }
    printk("%x\n", ext2_sb.s_magic);

    int block_size = 1 << (ext2_sb.s_log_block_size + 10); //1个block大小
    int block_group_num = (ext2_sb.s_blocks_count - ext2_sb.s_first_data_block - 1) / ext2_sb.s_blocks_per_group + 1; //有多少个block_group
    struct ext2_group_desc group_desc[block_group_num];
    memset(group_desc, 0, sizeof(group_desc));
    dev_table[MAJOR(dev_num)].request_fn(dev_num, 0, (char *)&group_desc, block_size/512*2+2, sizeof(group_desc)); //读group desc
    while (1)
    {
    }
    // move_ext2_sb_to_sb(&sb, ext2_sb);

    struct inode *new_inode=get_inode();
    new_inode->sb=sb;
    new_inode->dev_num=sb->dev_num;
    new_inode->f_op=&ext2_f_op;
    new_inode->inode_op = &ext2_inode_op;
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

void move_ext2_sb_to_sb(struct super_block **sb, struct ext2_super_block esb)
{
    int block_size = 1 << (esb.s_log_block_size + 10); //1个block大小
    int block_group_num = (esb.s_blocks_count - esb.s_first_data_block - 1) / esb.s_blocks_per_group + 1; //有多少个block_group
}

static int f_op_read(struct file_descriptor *fd, char *buf, int nbyte)
{
    return 0;
}

static int ext2_lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir)
{
    return 0;
}