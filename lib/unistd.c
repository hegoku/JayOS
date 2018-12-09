#include <unistd.h>
#include <system/fs.h>
#include <system/system_call.h>

extern int sys_call_0_param(int index);
extern int sys_call_1_param(int index, ...);
extern int sys_call_2_param(int index, ...);
extern int sys_call_3_param(int index, ...);

void exit(int status)
{
    sys_call_1_param(SYS_CALL_EXIT, status);
}

pid_t fork(void)
{
    return sys_call_0_param(SYS_CALL_FORK);
}

ssize_t read(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(SYS_CALL_READ, fd, buf, nbytes);
}

ssize_t write(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(SYS_CALL_WRITE, fd, buf, nbytes);
}

int open(const char *path, int flags, ...)
{
    return sys_call_2_param(SYS_CALL_OPEN, path, flags);
}

int close(int fd)
{
    return sys_call_1_param(SYS_CALL_CLOSE, fd);
}

pid_t wait(int *status)
{
    return sys_call_1_param(SYS_CALL_WAITPID, status);
}

int mount(char *dev_name, char *dir_name, char *type)
{
    return sys_call_3_param(SYS_CALL_MOUNT, dev_name, dir_name, type);
}

int stat(char *filename, struct stat *statbuf)
{
    return sys_call_2_param(SYS_CALL_STAT, filename, statbuf);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return sys_call_3_param(SYS_CALL_LSEEK, fd, offset, whence);
}

pid_t getpid()
{
    return sys_call_0_param(SYS_CALL_GETPID);
}

int dup(unsigned int oldfd)
{
    return sys_call_2_param(SYS_CALL_DUP, oldfd);
}

pid_t getppid()
{
    return sys_call_0_param(SYS_CALL_GETPPID);
}

//临时
int get_ticks()
{
    return sys_call_0_param(0);
}