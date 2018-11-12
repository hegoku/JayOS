#ifndef _UNISTD_H
#define _UNISTD_H

#include "sys/types.h"

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2

ssize_t write(int fildes, const void *buf, size_t nbyte);
// ssize_t write(int, const void *, size_t);

#endif