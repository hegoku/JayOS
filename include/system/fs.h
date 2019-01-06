#ifndef _SYSTEM_FS_H
#define _SYSTEM_FS_H

#include <sys/types.h>
#include <system/list.h>
#include <system/compiler_types.h>

#define PROC_FILES_MAX_COUNT 1024 //进程表文件最大数
#define FILE_DESC_TABLE_MAX_COUNT 2048 //f_desc_table最大数
#define INODE_TABLE_MAX_COUNT 2048 //inode_table最大数

#define FILE_MODE_REG 0 //常规文件
#define FILE_MODE_BLK 1 //块设备
#define FILE_MODE_CHR 2 //字符设备
#define FILE_MODE_DIR 3 //目录

struct file_system_type{
    const char *name;
    struct dir_entry *(*mount)(struct file_system_type *fs_type, int dev_num);
    struct file_system_type *next;
    struct super_block *sb_table;
};

struct super_block{
    unsigned int inodes_count; //节点数
    unsigned long sector_count; //扇区总数 BPB_TotSec16
    // unsigned int root_inode;
    unsigned long dir_entry_count; //BPB_NumFATs

    int dev_num; //设备号
    struct dir_entry *root_dir;
    struct inode *root_inode;
    struct file_system_type	*fs_type;
    void *s_fs_info; //各自超级快额外信息
};

struct inode{
    unsigned short mode;
    unsigned short uid;
    unsigned long size;
    unsigned long mtime;
    unsigned char gid;
    unsigned char nlinks;
    unsigned long start_pos;

    int dev_num; //设备号
    unsigned int num;        //节点号
    unsigned int used_count; //节点被使用次数
    struct file_operation *f_op;
    struct super_block *sb;
    struct inode_operation *inode_op;
    void *i_private; //额外信息
};

struct dir_entry{
    char name[256];
    struct inode *inode;
    struct super_block *sb;
    struct dir_entry *parent;
    struct list children;
    int dev_num;
    int is_mounted;
    struct dir_entry *mounted_dir;
    void *d_fsdata; //各自dentry的额外信息
};

struct file_descriptor{
    int mode;
    int pos;
    int used_count;
    struct inode *inode;
    struct file_operation *op;
};

struct linux_dirent{
    unsigned long inode_num;                 /* inode number */
    off_t d_off;                /* offset to next dirent */
    unsigned short d_reclen;    /* length of this dirent */
    char name [256];   /* filename (null-terminated) */

    char type;
};

struct file_operation{
    int (*lseek) (struct file_descriptor *fd, off_t, int);
	int (*read) (struct file_descriptor *fd, char *buf, int);
	int (*write) (struct file_descriptor *fd, char *buf, int);
	int (*readdir) (struct inode *inode, struct file_descriptor *fd, struct linux_dirent *, int);
	// int (*select) (struct inode *inode, struct file *, int, select_table *);
	int (*ioctl) (struct inode *inode, struct file_descriptor *fd, unsigned int, unsigned long);
	// int (*mmap) (struct inode *inode, struct file_descriptor *fd, unsigned long, size_t, int, unsigned long);
	int (*open) (struct inode *inode, struct file_descriptor *fd);
	void (*release) (struct inode *inode, struct file_descriptor *fd);
	int (*fsync) (struct inode *inode, struct file_descriptor *fd);
};

struct inode_operation {
	int (*create) (struct inode *,const char *,int,int,struct inode **);
	int (*lookup) (struct dir_entry *,const char *,int,struct dir_entry **);
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

struct nameidata {
    struct dir_entry *dir;
    char *last_name;
    int last_len;
};

struct s_stat{
    int dev_num;     /* ID of device containing file */
    unsigned long inode_num;     /* inode number */
    unsigned short mode;
    ssize_t size;
};

extern struct file_system_type *file_system_table;
extern struct file_descriptor f_desc_table[];
extern struct inode inode_table[];

int sys_open(const char __user *path, int flags, ...);
int sys_write(int fd, const char __user *buf, unsigned int nbyte);
int sys_read(int fd, char __user *buf, unsigned int nbyte);
int sys_close(int fd);
int sys_mkdir(const char __user *dirname, int mode);
int sys_chdir(const char __user *dirname);
int sys_mount(char __user *dev_name, char __user *dir_name, char __user *type);
off_t sys_lseek(int fd, off_t offset, int whence);
int sys_stat(char __user *filename, struct s_stat __user *statbuf);
int sys_chroot(const char __user *dirname);
int sys_dup(unsigned int oldfd);
int sys_getdents(unsigned int fd, struct linux_dirent __user *dirent, unsigned int count);

int get_inode_by_filename(const char *filename, struct inode **res_inode);

void register_filesystem(struct file_system_type *fs_type);
struct inode *get_inode();
struct dir_entry *get_dir();
struct super_block *get_block(int dev_num);

int lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir);
int namei(const char *pathname, struct dir_entry **res_dir);
void mount_root();
void lsdir(const char *path, struct dir_entry *dir);
#endif