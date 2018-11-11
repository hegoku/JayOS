#ifndef	_STDIO_H_
#define	__STDIO_H_

#include "stdarg.h"

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list arg );
int printf(const char *fmt, ...);

#endif
