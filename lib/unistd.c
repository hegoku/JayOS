#include "unistd.h"

extern int sys_call_0_param(int index);
extern int sys_call_1_param(int index, ...);
extern int sys_call_2_param(int index, ...);
extern int sys_call_3_param(int index, ...);

ssize_t write(int fildes, const void *buf, unsigned int nbyte)
{
    return sys_call_3_param(1, fildes, buf, nbyte);
}