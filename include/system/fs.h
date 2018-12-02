#ifndef _SYSTEM_FS_H
#define _SYSTEM_FS_H

#include <sys/types.h>

#define PROC_FILES_MAX_COUNT 1024 //进程表文件最大数
#define FILE_DESC_TABLE_MAX_COUNT 2048 //f_desc_table最大数
#define INODE_TABLE_MAX_COUNT 2048 //inode_table最大数

#define FILE_MODE_REG 0 //常规文件
#define FILE_MODE_BLK 1 //块设备
#define FILE_MODE_CHR 2 //字符设备
#define FILE_MODE_DIR 3 //目录

struct file_system_type{
    const char *name;
    struct super_block *(*read_super)(struct file_system_type *fs_type);
    struct file_system_type *next;
    struct super_block *sb;
};

struct super_block{
    unsigned int inodes_count; //节点数
    unsigned long sector_count; //扇区总数 BPB_TotSec16
    // unsigned int root_inode;
    unsigned long dir_entry_count; //BPB_NumFATs

    int dev_num; //设备号
    struct dir_entry *root_dir;
    struct inode *root_inode;
};

struct inode{
    unsigned short mode;
    unsigned short uid;
    unsigned long size;
    unsigned long mtime;
    unsigned char gid;
    unsigned char nlinks;
    unsigned long start_sector;
    unsigned long sector_count;

    int dev_num; //设备号
    unsigned int num;        //节点号
    unsigned int used_count; //节点被使用次数
    struct file_operation *f_op;
    struct super_block *sb;
    struct inode_operation *inode_op;
};

struct list{
    void *value;
    struct list *next;
};

struct dir_entry{
    char name[12];
    struct inode *inode;
    struct super_block *sb;
    struct dir_entry *parent;
    struct list *children;
    int dev_num;
    unsigned long start_sector;
    unsigned long sector_count;
};

struct fat16{
    unsigned char BS_jmpBoot : 3;
    unsigned char BS_OEMName;
    unsigned char BPB_BytsPerSec : 2;
    unsigned char BPB_SecPerClus : 1;
    unsigned char BPB_RsvdSecCnt : 2;
    unsigned char BPB_NumFATs : 1; //FAT表数量
    unsigned char BPB_RootEntCnt : 2; //更目录文件最大数
    unsigned char BPB_TotSec16 : 2; //扇区总数
    unsigned char BPB_Media : 1;
    unsigned char BPB_FATz16 : 2; //每个fat占用多少扇区
    unsigned char BPB_SecPerTrk : 2;
    unsigned char BPB_NumHeads : 4;
    unsigned char BPB_HiddSec : 4;
    unsigned char BPB_TotSec32 : 4; //如果BPB_TotSec16为0，由这个表述扇区总数
    unsigned char BS_DrvNum : 1;
    unsigned char BS_Reserved1 : 1;
    unsigned char BS_BootSig : 1;
    unsigned char BD_VolId : 4;
    unsigned short BS_VolLab : 11;
    unsigned char BS_FileSysType;
    unsigned char boot_code[56];
    unsigned char end_flag : 2;
};

struct file_descriptor{
    int mode;
    int pos;
    struct inode *inode;
    struct file_operation *op;
};

struct file_operation{
    int (*lseek) (struct inode *inode, struct file_descriptor *fd, off_t, int);
	int (*read) (struct inode *inode, struct file_descriptor *fd, char *buf, int);
	int (*write) (struct inode *inode, struct file_descriptor *fd, char *buf, int);
	// int (*readdir) (struct inode *inode, struct file_descriptor *fd, struct dirent *, int);
	// int (*select) (struct inode *inode, struct file *, int, select_table *);
	int (*ioctl) (struct inode *inode, struct file_descriptor *fd, unsigned int, unsigned long);
	// int (*mmap) (struct inode *inode, struct file_descriptor *fd, unsigned long, size_t, int, unsigned long);
	int (*open) (struct inode *inode, struct file_descriptor *fd);
	void (*release) (struct inode *inode, struct file_descriptor *fd);
	int (*fsync) (struct inode *inode, struct file_descriptor *fd);
};

struct inode_operation {
	int (*create) (struct inode *,const char *,int,int,struct inode **);
	int (*lookup) (struct inode *,const char *,int,struct inode **);
	int (*link) (struct inode *,struct inode *,const char *,int);
	int (*unlink) (struct inode *,const char *,int);
	int (*symlink) (struct inode *,const char *,int,const char *);
	int (*mkdir) (struct inode *, struct dir_entry *, int umode);
	int (*rmdir) (struct inode *,const char *,int);
	int (*mknod) (struct inode *,const char *,int,int,int);
	int (*rename) (struct inode *,const char *,int,struct inode *,const char *,int);
	int (*readlink) (struct inode *,char *,int);
	int (*follow_link) (struct inode *,struct inode *,int,int,struct inode **);
	int (*bmap) (struct inode *,int);
	void (*truncate) (struct inode *);
	int (*permission) (struct inode *, int);
};

extern struct file_system_type **file_system_table;
extern struct file_descriptor f_desc_table[];
extern struct inode inode_table[];
extern struct super_block super_block_table[];

int sys_open(const char *path, int flags, ...);
int sys_write(int fd, const void *buf, unsigned int nbyte);
int sys_read(int fd, char *buf, unsigned int nbyte);
int sys_close(int fd);
int sys_mkdir(const char *dirname, int mode);
off_t sys_lseek(int fd, off_t offset, int whence);
int get_inode_by_filename(const char *filename, struct inode **res_inode);

void register_filesystem(struct file_system_type *fs_type);
struct inode *get_inode();
struct dir_entry *get_dir();
struct super_block *get_block(int dev_num);

void mount_root();
#endif