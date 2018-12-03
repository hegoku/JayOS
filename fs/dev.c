#include <system/dev.h>
#include <system/fs.h>
#include <string.h>
#include "../kernel/global.h"

/* blk_dev_struct 块设备结构是：(kernel/blk_drv/blk.h,23)
* do_request-address //对应主设备号的请求处理程序指针。
* current-request // 该设备的下一个请求。
*/
// 该数组使用主设备号作为索引（下标）。
struct dev_struct dev_table[5] = {
	{0, "", 0, 0, 0},		/* no_dev */// 0 - 无设备。
	{0, "", 0, 0, 0},		/* dev mem */// 1 - 内存。
	{0, "", 0, 0, 0},		/* dev fd */// 2 - 软驱设备。
	{0, "", 0, 0, 0},		/* dev hd */// 3 - 硬盘设备。
	{0, "", 0, 0, 0},		/* dev tty */// 4 - tty 设备。
};

void install_dev(int dev_num, char *name, int type, struct file_operation *f_op)
{
    dev_table[dev_num].type = type;
    dev_table[dev_num].name = name;
    dev_table[dev_num].f_op = f_op;

    struct dir_entry *dev_dir;
    if (namei("/dev", &dev_dir)) {
        printk("/dev not found\n");
        return;
    }
    struct inode *new_inode = get_inode();
    new_inode->sb=dev_dir->sb;
    new_inode->dev_num=MKDEV(dev_num, 0);
    if (type==DEV_TYPE_CHR) {
        new_inode->mode = FILE_MODE_CHR;
    } else if (type==DEV_TYPE_BLK) {
        new_inode->mode = FILE_MODE_BLK;
    }
    new_inode->f_op = f_op;

    struct dir_entry *new_dir=get_dir();
    memcpy(new_dir->name, name, strlen(name));
    new_dir->dev_num=MKDEV(dev_num, 0);
    new_dir->inode=new_inode;
    new_dir->parent=dev_dir;
    new_dir->sb=dev_dir->sb;

    struct list *tmp = create_list();
    tmp->value = new_dir;
    tmp->next = dev_dir->children;
    dev_dir->children = tmp;
}

void mount_dev()
{
    
}