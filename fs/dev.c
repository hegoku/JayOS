#include <system/dev.h>
#include <system/fs.h>
#include <string.h>
#include "../kernel/global.h"
#include <sys/types.h>
#include <stdio.h>

/* blk_dev_struct 块设备结构是：(kernel/blk_drv/blk.h,23)
* do_request-address //对应主设备号的请求处理程序指针。
* current-request // 该设备的下一个请求。
*/
// 该数组使用主设备号作为索引（下标）。
struct dev_struct dev_table[5] = {
	{0, 0, NULL, NULL, NULL},		/* no_dev */// 0 - 无设备。
	{0, 1, NULL, NULL, NULL},		/* dev mem */// 1 - 内存。
	{0, 512, NULL, NULL, NULL},		/* dev fd */// 2 - 软驱设备。
	{0, 512, NULL, NULL, NULL},		/* dev hd */// 3 - 硬盘设备。
	{0, 1, NULL, NULL, NULL},		/* dev tty */// 4 - tty 设备。
};

void install_dev(int major, char *name, int type, struct file_operation *f_op)
{
    dev_table[major].type = type;
    dev_table[major].name = name;
    dev_table[major].f_op = f_op;

    struct dir_entry *dev_dir;
    if (namei("/dev", &dev_dir)) {
        printk("/dev not found\n");
        return;
    }
    struct inode *new_inode = get_inode();
    new_inode->sb=dev_dir->sb;
    new_inode->dev_num=MKDEV(major, 0);
    if (type==DEV_TYPE_CHR) {
        new_inode->mode = FILE_MODE_CHR;
    } else if (type==DEV_TYPE_BLK) {
        new_inode->mode = FILE_MODE_BLK;
    }
    new_inode->f_op = f_op;

    struct dir_entry *new_dir=get_dir();
    sprintf(new_dir->name, "%s0", name);
    // memcpy(new_dir->name, name, strlen(name));
    new_dir->dev_num=MKDEV(major, 0);
    new_dir->inode=new_inode;
    new_dir->parent=dev_dir;
    new_dir->sb=dev_dir->sb;

    struct list *tmp = create_list();
    tmp->value = new_dir;
    tmp->next = dev_dir->children;
    dev_dir->children = tmp;

    //
    new_inode = get_inode();
    new_inode->sb=dev_dir->sb;
    new_inode->dev_num=MKDEV(major, 1);
    if (type==DEV_TYPE_CHR) {
        new_inode->mode = FILE_MODE_CHR;
    } else if (type==DEV_TYPE_BLK) {
        new_inode->mode = FILE_MODE_BLK;
    }
    new_inode->f_op = f_op;

    new_dir=get_dir();
    sprintf(new_dir->name, "%s1", name);
    // memcpy(new_dir->name, name, strlen(name));
    new_dir->dev_num=MKDEV(major, 1);
    new_dir->inode=new_inode;
    new_dir->parent=dev_dir;
    new_dir->sb=dev_dir->sb;

    tmp = create_list();
    tmp->value = new_dir;
    tmp->next = dev_dir->children;
    dev_dir->children = tmp;
}

void mount_dev()
{
    
}