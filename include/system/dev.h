#ifndef	_SYSTEM_DEV_H_
#define	_SYSTEM_DEV_H_

#include <system/fs.h>

#define DEV_OPEN 0
#define DEV_WRITE 1
#define DEV_READ 2

#define DEV_TYPE_CHR 0 //字符设备
#define DEV_TYPE_BLK 1 //块设备

//设备号宏， 来自linux
#define MINORBITS 20 /*次设备号*/

#define MINORMASK ((1U << MINORBITS) - 1 ) /*次设备号掩码*/

#define MAJOR(dev) ((unsigned int) ((dev) >> MINORBITS)) /*dev右移20位得到主设备号*/

#define MINOR(dev) ((unsigned int) ((dev) & MINORMASK)) /*与次设备掩码与，得到次设备号*/

#define MKDEV(ma,mi) (((ma) << MINORBITS) | (mi)) //通过主设备号和此设备号求出总设备号

struct blk_request
{
    int dev;			/* -1 if no request */// 使用的设备号。
    int cmd;			
    int errors;			//操作时产生的错误次数。
    unsigned long sector;		// 起始扇区。(1 块=2 扇区)
    unsigned long nr_sectors;	// 读/写扇区数。
    unsigned long bytes;
    char *buffer;             // 数据缓冲区。
    struct s_proc *waiting;	// 任务等待操作执行完成的地方。
    char *bh;	// 缓冲区头指针
    struct blk_request *next;		// 指向下一请求项。
};

// 块设备结构。
struct dev_struct
{
    int type; //设备快类型
    int (*request_fn)(void);           // 请求操作的函数指针。
    void *current_request;	// 请求信息结构。
    struct file_operation *f_op;
};

// typedef (*chr_do_request_ptr)(int cmd, unsigned mi_dev, char * buf, int len);
// typedef (*blk_do_request_ptr)(int cmd, unsigned mi_dev, unsigned long pos, char * buf, int len);

void init_dev();

extern struct dev_struct dev_table[];

void install_dev(int dev_num, struct file_operation *f_op);
#endif