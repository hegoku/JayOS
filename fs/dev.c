#include <system/dev.h>
#include <system/fs.h>
#include <sys/types.h>

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

void install_dev(int dev_num, struct file_operation *f_op)
{
    dev_table[dev_num].f_op = f_op;
}

void mount_dev()
{
    
}