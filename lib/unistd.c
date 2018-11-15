#include <unistd.h>

extern int sys_call_0_param(int index);
extern int sys_call_1_param(int index, ...);
extern int sys_call_2_param(int index, ...);
extern int sys_call_3_param(int index, ...);

ssize_t write(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(1, fd, buf, nbytes);
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