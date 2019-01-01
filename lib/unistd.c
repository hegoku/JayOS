#include <unistd.h>
#include <system/fs.h>
#include <system/system_call.h>

extern int sys_call_0_param(int index);
extern int sys_call_1_param(int index, ...);
extern int sys_call_2_param(int index, ...);
extern int sys_call_3_param(int index, ...);

inline void exit(int status)
{
    sys_call_1_param(SYS_CALL_EXIT, status);
}

inline pid_t fork()
{
    return sys_call_0_param(SYS_CALL_FORK);
}

inline ssize_t read(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(SYS_CALL_READ, fd, buf, nbytes);
}

inline ssize_t write(int fd, const void *buf, unsigned int nbytes)
{
    return sys_call_3_param(SYS_CALL_WRITE, fd, buf, nbytes);
}

inline int open(const char *path, int flags, ...)
{
    return sys_call_2_param(SYS_CALL_OPEN, path, flags);
}

inline int close(int fd)
{
    return sys_call_1_param(SYS_CALL_CLOSE, fd);
}

inline pid_t waitpid(pid_t pid, int *wstatus, int options)
{
    return sys_call_3_param(SYS_CALL_WAITPID, pid, wstatus, options);
}

inline int execve(const char *filename, const char *argv[], const char *envp[])
{
    return sys_call_3_param(SYS_CALL_EXECVE, filename, argv, envp);
}

inline int mount(char *dev_name, char *dir_name, char *type)
{
    return sys_call_3_param(SYS_CALL_MOUNT, dev_name, dir_name, type);
}

inline int stat(char *filename, struct stat *statbuf)
{
    return sys_call_2_param(SYS_CALL_STAT, filename, statbuf);
}

inline off_t lseek(int fd, off_t offset, int whence)
{
    return sys_call_3_param(SYS_CALL_LSEEK, fd, offset, whence);
}

inline pid_t getpid()
{
    return sys_call_0_param(SYS_CALL_GETPID);
}

inline int alarm(unsigned int seconds)
{
    return sys_call_0_param(SYS_CALL_ALARM);
}

inline int pause()
{
    return sys_call_0_param(SYS_CALL_PAUSE);
}

inline int dup(unsigned int oldfd)
{
    return sys_call_2_param(SYS_CALL_DUP, oldfd);
}

inline pid_t getppid()
{
    return sys_call_0_param(SYS_CALL_GETPPID);
}

//临时
inline int get_ticks()
{
    return sys_call_0_param(0);
}