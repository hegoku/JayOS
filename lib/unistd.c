#include <unistd.h>
#include <system/system_call.h>

extern int sys_call_0_param(int index);
extern int sys_call_1_param(int index, ...);
extern int sys_call_2_param(int index, ...);
extern int sys_call_3_param(int index, ...);

int open(const char *path, int flags, ...)
{
    return sys_call_2_param(SYS_CALL_OPEN, path, flags);
}

ssize_t write(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(SYS_CALL_WRITE, fd, buf, nbytes);
}

ssize_t read(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(SYS_CALL_READ, fd, buf, nbytes);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return sys_call_3_param(SYS_CALL_LSEEK, fd, offset, whence);
}

int close(int fd)
{
    return sys_call_1_param(SYS_CALL_CLOSE, fd);
}

pid_t fork(void)
{
    return 0;
}

//临时
int get_ticks()
{
    return sys_call_0_param(0);
}