#include <system/fs.h>
#include <string.h>
#include <sys/types.h>
#include <system/dev.h>
#include "../kernel/hd.h"
#include "../kernel/kernel.h"
#include "../kernel/global.h"
#include "../kernel/tty.h"

struct file_system_type **file_system_table;
struct file_descriptor f_desc_table[FILE_DESC_TABLE_MAX_COUNT];
struct inode inode_table[INODE_TABLE_MAX_COUNT];
struct super_block super_block_table[4];
struct dir_entry dir_table[INODE_TABLE_MAX_COUNT];
struct list list_table[1000];
static int inode_num = 0;
static int sb_num=0;
static int dir_num=0;
static int list_num = 0;

static int get_parent_dir_entry(const char *pathname, struct dir_entry *base, struct nameidata *nd);
static int _namei(const char *pathname, struct dir_entry *base, struct dir_entry **dir);
static int lookup_fs(const char *name, struct file_system_type **fs_type);

void mkfs()
{
    struct part_ent *pt;

    unsigned char *fsbuf = (unsigned char *)0x600000;

    struct super_block sb;
    sb.sector_count = pt->nr_sects;
    // sb.root_inode = 1;

    struct blk_request a;
    a.buffer = fsbuf;
    a.bh = fsbuf;
    a.cmd = 1;
    a.dev = 0;
    a.next = NULL;
    a.nr_sectors = 1;
    a.sector = pt->start_sect;

}

int sys_open(const char *path, int flags, ...)
{
    int fd = -1;
    int i;
    for (i = 0; i < PROC_FILES_MAX_COUNT; i++) {
        if (current_process->file_table[i]==0) {
            fd = i;
            break;
        }
    }

    if (fd<0 || fd>=PROC_FILES_MAX_COUNT) {
        printk("file_table is full (PID:%d)\n", current_process->pid);
        return -1;
    }

    for (i = 0; i < FILE_DESC_TABLE_MAX_COUNT; i++) {
        if (f_desc_table[i].inode==NULL) {
            break;
        }
    }

    if (i>=FILE_DESC_TABLE_MAX_COUNT) {
        printk("f_Desc_table is full (PID:%d)\n", current_process->pid);
        return -1;
    }

    struct inode *p_inode = NULL;

    if (get_inode_by_filename(path, &p_inode)) {
        printk("file: %s not found\n", path);
        return -1;
    }

    current_process->file_table[fd] = &f_desc_table[i];
    f_desc_table[i].pos = 0;
    f_desc_table[i].inode = p_inode;
    f_desc_table[i].op = p_inode->f_op;

    if (p_inode->f_op->open) {
        p_inode->f_op->open(p_inode, &f_desc_table[i]);
    }
    p_inode->used_count++;

    // if (p_inode->mode==FILE_MODE_CHR) {
    // }

    return fd;
}

struct dir_entry* find_children_by_name(struct dir_entry *dir, const char *filename)
{
    for (struct list *i = dir->children; i; i = i->next)
    {
        struct dir_entry *tmp = (struct dir_entry *)i->value;
        if (strcmp(filename, tmp->name) == 0)
        {
            return tmp;
        }
    }
    return 0;
}

int get_inode_by_filename(const char *filename, struct inode **res_inode)
{
    struct dir_entry *dir;
    const char *tmp_path;
    int n_len;
    char c;
    *res_inode = NULL;

    if (filename[0] == '/')
    {
        dir = current_process->root;
    } else {
        dir = current_process->pwd;
    }
    filename++;

    while (1)
    {
        tmp_path = filename;
        for (n_len = 0; (c = *(filename++)) && (c != '/'); n_len++)
        {
        }

        char str1[n_len+1];
        str1[n_len] = '\0';
        memcpy(str1, tmp_path, n_len);
        dir = find_children_by_name(dir, str1);
        if (dir==0) {
            return -1;
        }

        if (!c) {
            break;
        }
    }
    *res_inode = dir->inode;
    return 0;

    // if (strcmp(filename, "/tty1") == 0)
    // {
    //     *res_inode = &inode_table[0];
    //     return 1;
    // }
    // if (strcmp(filename, "/hd1")==0)
    // {
    //     *res_inode = &inode_table[3];
    //     return 1;
    // }
    // return 0;
}

int sys_close(int fd)
{
    current_process->file_table[fd]->inode->used_count--;
    current_process->file_table[fd]->inode = 0;
    current_process->file_table[fd] = 0;

    return 0;
}

int sys_write(int fd, const void *buf, unsigned int nbyte)
{
    struct file_descriptor *file;

    if (fd >= PROC_FILES_MAX_COUNT || (file=current_process->file_table[fd]) == 0)
    {
        printk("fd: %d not exist (PID: %d)\n", fd, current_process->pid);
        return -1;
    }
    if (nbyte==0) {
        return 0;
    }

    // if (!dev_table[MAJOR(file->inode->dev_num)].request_fn) {
    //     printk("dev: 0x%x\n",file->inode->dev_num);
	// 	printk("Trying to r/w from/to nonexistent character device\n");
    //     return -1;
    // }

    if (file->op->write) {
        return file->op->write(file, (char *)buf, nbyte);
    }

    // if (file->inode->mode == FILE_MODE_CHR)
    // {
    //     // chr_do_request_ptr call_addr = (chr_do_request_ptr)dev_table[MAJOR(file->inode->dev_num)].request_fn;
    //     // return call_addr(MINOR(file->inode->dev_num), DEV_WRITE, (char *)buf, nbyte);
    //     return tty_write(MINOR(file->inode->dev_num), (char *)buf, nbyte);
    // } else if (file->inode->mode == FILE_MODE_BLK)
    // {
    //     // blk_do_request_ptr call_addr = (blk_do_request_ptr)dev_table[MAJOR(file->inode->dev_num)].request_fn;
    //     // return call_addr(MINOR(file->inode->dev_num), DEV_WRITE, file->inode->start_sector, (char *)buf, nbyte);
    // }

    printk ("(Write)inode->i_mode=%d\n", file->inode->mode);
	return -1;
}

int sys_read(int fd,char * buf, unsigned int nbyte)
{
    struct file_descriptor *file;

    if (fd >= PROC_FILES_MAX_COUNT || nbyte < 0 || (file=current_process->file_table[fd]) == 0)
    {
        printk("fd: %d not exist (PID: %d)\n", fd, current_process->pid);
        return -1;
    }
    if (nbyte==0 || nbyte>file->inode->size) {
        return 0;
    }

    if (file->op->read) {
        return file->op->read(file, (char *)buf, nbyte);
    }

    printk ("(Write)inode->i_mode=%d\n", file->inode->mode);
	return -1;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
    struct file_descriptor *file;
    int tmp;

    if (fd >= PROC_FILES_MAX_COUNT || (file=current_process->file_table[fd]) == 0)
    {
        printk("fd: %d not exist (PID: %d)\n", fd, current_process->pid);
        return -1;
    }

    if (file->op && file->op->lseek) {
        return file->op->lseek(file, offset, whence);
    }

    switch (whence) {
    case 0: //SEEK_SET
        if (offset<0) {
            return -1;
        }
        file->pos = offset;
        break;
    case 1: //SEEK_CUR
        if (file->pos+offset<0) {
            return -1;
        }
        file->pos += offset;
        break;
    case 2:
        if ((tmp = file->inode->size + offset) < 0){
            return -1;
        }
        file->pos = tmp;
        break;
    default:
        return -1;
    }
    return file->pos;
}

void register_filesystem(struct file_system_type *fs_type)
{
    if (file_system_table==0) {
        *file_system_table=fs_type;
    } else {
        struct file_system_type *tmp=*file_system_table;
        while(tmp->next) {
            tmp=tmp->next;
        }
        tmp->next=fs_type;
    }
}

int do_mount(const char *dev_name, const char *dir, char *type)
{
    struct dir_entry *dev_dir;
    struct dir_entry *dir_dir;
    struct dir_entry *new_dir;
    struct file_system_type * fs_type;

    if (lookup_fs(type, &fs_type)) {
        printk("fs %s not exist\n", type);
        return -1;
    }

    // if (namei(dev_name, &dev_dir)) {
    //     printk("device %s not found\n", dev_name);
    //     return -1;
    // }
    
    // if (namei(dir, &dir_dir)) {
    //     printk("dir %s not found\n", dir);
    //     return -1;
    // }
    // if (dir_dir->is_mounted) { //只能被挂载一次
    //     printk("dir %s has been mounted\n", dir);
    //     return -1;
    // }

    // new_dir = fs_type->mount(fs_type, dev_dir->dev_num);

    // while (dir_dir->is_mounted) {
    //     dir_dir = dir_dir->mounted_dir;
    // }
    // dir_dir->is_mounted = 1;
    // dir_dir->mounted_dir = new_dir;
    return 0;
}

void mount_root()
{
    struct file_system_type * fs_type;
	struct dir_entry * dir;
	struct inode * inode;

    fs_type = *file_system_table;

    // do {
        dir=fs_type->mount(fs_type, ROOT_DEV);
        fs_type=fs_type->next;
        current_process->root = dir;
        current_process->pwd = dir;
    // } while (fs_type);
    sys_mkdir("/dev", 1);
    sys_mkdir("/root", 1);
}

struct inode *get_inode()
{
    struct inode a={
        num: inode_num
    };
    inode_table[inode_num]=a;
    return &inode_table[inode_num++];
}

struct super_block *get_block(int dev_num)
{
    struct super_block a={
        dev_num: dev_num
    };
    super_block_table[sb_num]=a;
    return &super_block_table[sb_num++];
}

struct dir_entry *get_dir()
{
    struct dir_entry a;
    dir_table[dir_num]=a;
    return &dir_table[dir_num++];
}

struct list *create_list()
{
    struct list a={

    };
    list_table[list_num]=a;
    return &list_table[list_num++];
}

int sys_mkdir(const char *dirname, int mode)
{
    struct dir_entry *dir;
    char last_name[12];
    memset(last_name, 0, sizeof(last_name));
    // const char *tmp_path;
    // int n_len;
    // char c;

    // if (dirname[0] == '/')
    // {
    //     dir = current_process->root;
    // } else {
    //     dir = current_process->pwd;
    // }
    // dirname++;

    // while (1) {
    //     tmp_path = dirname;
    //     for (n_len = 0; (c = *(dirname++)) && (c != '/'); n_len++)
    //     {
    //     }

    //     if (!c) {
    //         break;
    //     }

    //     char str1[n_len+1];
    //     str1[n_len] = '\0';
    //     memcpy(str1, tmp_path, n_len);
    //     dir = find_children_by_name(dir, str1);
    //     if (dir==0) {
    //         return 0;
    //     }
    // }

    struct nameidata ns = {
        last_name: last_name
    };

    if (get_parent_dir_entry(dirname, current_process->root, &ns)) {
        printk("fdsa\n");
        return -1;
    }
    dir = ns.dir;

    if (dir->inode->mode!=FILE_MODE_DIR) {
        printk("%s is not a dir\n", dir->name);
        return -1;
    }

    struct inode *new_inode=get_inode();
    new_inode->sb=dir->sb;
    new_inode->dev_num=dir->dev_num;
    new_inode->mode = FILE_MODE_DIR;

    struct dir_entry *dev_dir=get_dir();
    memcpy(dev_dir->name, ns.last_name, ns.last_len);
    dev_dir->dev_num=dir->dev_num;
    dev_dir->inode=new_inode;
    dev_dir->parent=dir;
    dev_dir->sb=dir->sb;
    dev_dir->is_mounted = 0;
    dev_dir->mounted_dir = NULL;

    struct list *tmp = create_list();
    tmp->value = dev_dir;
    tmp->next = dir->children;
    dir->children = tmp;

    //
    // new_inode=get_inode();
    // new_inode->sb=dev_dir->sb;
    // new_inode->dev_num=MKDEV(4, 0);
    // new_inode->mode = FILE_MODE_CHR;
    // new_inode->f_op = dev_table[4].f_op;

    // struct dir_entry *new_dir=get_dir();
    // memcpy(new_dir->name, "tty1", 4);
    // new_dir->dev_num=MKDEV(4, 0);
    // new_dir->inode=new_inode;
    // new_dir->parent=dev_dir;
    // new_dir->sb=dev_dir->sb;

    // tmp = create_list();
    // tmp->value = new_dir;
    // tmp->next = dev_dir->children;
    // dev_dir->children = tmp;

    //
    // new_inode=get_inode();
    // new_inode->sb=dev_dir->sb;
    // new_inode->dev_num=MKDEV(3, 0);
    // new_inode->mode = FILE_MODE_BLK;
    // new_inode->f_op = dev_table[3].f_op;

    // new_dir=get_dir();
    // memcpy(new_dir->name, "hd0", 3);
    // new_dir->dev_num=MKDEV(3, 0);
    // new_dir->inode=new_inode;
    // new_dir->parent=dev_dir;
    // new_dir->sb=dev_dir->sb;

    // tmp = create_list();
    // tmp->value = new_dir;
    // tmp->next = dev_dir->children;
    // dev_dir->children = tmp;

    // for (struct list *i = current_process->root->children; i; i = i->next)
    // {
    //     struct dir_entry *tmp = (struct dir_entry *)i->value;
    //     printk("name:%s\n", tmp->name);
    // }

    // for (struct list *i = dev_dir->children; i; i = i->next)
    // {
    //     struct dir_entry *tmp = (struct dir_entry *)i->value;
    //     printk("name:%s\n", tmp->name);
    // }

    return 0;
}

int sys_mount(char *dev_name, char *dir_name, char *type)
{
    struct dir_entry *dev_dir;
    struct dir_entry *dir_dir;
    struct dir_entry *new_dir;
    struct file_system_type * fs_type;

    if (lookup_fs(type, &fs_type)) {
        printk("fs %s not exist\n", type);
        return -1;
    }

    if (namei(dev_name, &dev_dir)) {
        printk("device %s not found\n", dev_name);
        return -1;
    }
    if (dev_dir->inode->mode!=FILE_MODE_BLK) {
        printk("%s is not a blk dev\n", dev_name);
        return -1;
    }

    if (namei(dir_name, &dir_dir))
    {
        printk("dir %s not found\n", dir_name);
        return -1;
    }
    if (dir_dir->inode->mode!=FILE_MODE_DIR) {
        printk("%s is not a dir\n", dir_name);
        return -1;
    }
    if (dir_dir->is_mounted) { //只能被挂载一次
        printk("dir %s has been mounted\n", dir_name);
        return -1;
    }

    new_dir = fs_type->mount(fs_type, dev_dir->dev_num);

    while (dir_dir->is_mounted) {
        dir_dir = dir_dir->mounted_dir;
    }
    dir_dir->is_mounted = 1;
    dir_dir->mounted_dir = new_dir;
    sys_stat("/root", NULL);while(1){}
    return 0;
}

static int get_parent_dir_entry(const char *pathname, struct dir_entry *base, struct nameidata *nd)
{
    struct dir_entry *dir;
    const char *thisname;
    int len, error;
    char c;
    nd->dir = NULL;

    if (!base) {
        base = current_process->pwd;
    }
    if (pathname[0] == '/')
    {
        base = current_process->root;
    }
    pathname++;
    dir = base;

    while (1)
    {
        thisname = pathname;
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
        {
        }

        if (!c) {
            break;
        }

        error = lookup(dir, thisname, len, &dir);
        if (error)
        {
            printk("get_parent_dir_entry lookup error\n");
            return error;
        }        
    }

    nd->last_len = len;
    nd->last_name = (char *)thisname;
    nd->dir = dir;
    return 0;
}

int lookup(struct dir_entry *dir, const char *name, int len, struct dir_entry **res_dir)
{
    if (len == 2 && name[0] == '.' && name[1] == '.')
    {
        if (dir==current_process->root) {
            *res_dir = dir;
            return 0;
        }
    }

    if (!len) {
        *res_dir = dir;
        return 0;
    }

    for (struct list *i = dir->children; i; i = i->next)
    {
        struct dir_entry *tmp = (struct dir_entry *)i->value;
        char tmp_name[len+1];
        memset(tmp_name, 0, len+1);
        memcpy(tmp_name, name, len);
        if (strcmp(tmp_name, tmp->name) == 0)
        {
            while (tmp->is_mounted) {
                tmp = tmp->mounted_dir;
            }
            *res_dir = tmp;
            return 0;
        }
    }

    return -1;
}

static int _namei(const char * pathname, struct dir_entry * base, struct dir_entry **dir)
{
	int error;
	// struct dir_entry tmp;

    char n[12];
    memset(n, 0, sizeof(n));
    struct nameidata nd={
        last_name: n
    };
    *dir = NULL;

    error = get_parent_dir_entry(pathname, base, &nd);
	if (error)
		return error;
	error = lookup(nd.dir , nd.last_name, nd.last_len , dir);
    if (error)
    {
        return error;
    }

    return 0;
}

int namei(const char *pathname, struct dir_entry **res_dir)
{
    return _namei(pathname, NULL, res_dir);
}

static int lookup_fs(const char *name, struct file_system_type **fs_type)
{
    struct file_system_type *tmp=*file_system_table;
    *fs_type = NULL;
    while (tmp)
    {
        if (strcmp(tmp->name, name)==0) {
            *fs_type = tmp;
            return 0;
        }
        tmp=tmp->next;
    }
    return -1;
}

int sys_stat(char *filename, struct stat *statbuf)
{
    struct dir_entry *dir;
    if (namei(filename, &dir)) {
        printk("%s not exist\n", filename);
        return -1;
    }

    printk("name: %s :\n", dir->name);
    for (struct list *i = dir->children; i; i = i->next)
    {
        struct dir_entry *tmp = (struct dir_entry *)i->value;
        if (tmp->inode->mode==FILE_MODE_REG) {
            printk("    - %dB name:%s\n", tmp->inode->size, tmp->name);
        } else if (tmp->inode->mode==FILE_MODE_DIR) {
            printk("    D %dB name:%s\n", tmp->inode->size, tmp->name);
        }
    }
    return 0;
}