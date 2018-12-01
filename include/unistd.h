#ifndef _UNISTD_H
#define _UNISTD_H

#include <sys/types.h>
#include <system/system_call.h>

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

int get_ticks(); //临时
int open(const char *path, int flags, ...);
ssize_t write(int fd, const void *buf, size_t nbytes);
ssize_t read(int fd, const void *buf, size_t nbytes);
off_t lseek(int fd, off_t offset, int whence);
pid_t fork(void);
int close(int fd);

#endif