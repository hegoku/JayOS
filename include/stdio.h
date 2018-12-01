#ifndef	_STDIO_H_
#define	__STDIO_H_

#include <stdarg.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned int size_t;
#endif

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef struct _file_desc {

} FILE;

static int vsscanf(const char *buf, const char *s, va_list ap);

int sprintf(char *str, const char *format, ...);
int vsprintf(char *str, const char *format, va_list arg );
int printf(const char *format, ...);
int scanf(const char *format, ...);
int sscanf(const char *str, const char *format, ...);

FILE *fopen(const char * restrict filename, const char * restrict mode);
int fclose(FILE *steam);
size_t fwrite(const void *restrict filename, size_t size, size_t, FILE *restrict);
size_t fread(void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream);

#endif
