#include "global.h"
#include "stdlib.h"

// char * itoa(char * str, int num)
// {
// 	char *	p = str;
// 	char	ch;
// 	int	i;
// 	int	flag = 0;

// 	*p++ = '0';
// 	*p++ = 'x';

// 	if(num == 0){
// 		*p++ = '0';
// 	}
// 	else{	
// 		for(i=28;i>=0;i-=4){
// 			ch = (num >> i) & 0xF;
// 			if(flag || (ch > 0)){
// 				flag = 1;
// 				ch += '0';
// 				if(ch > '9'){
// 					ch += 7;
// 				}
// 				*p++ = ch;
// 			}
// 		}
// 	}

// 	*p = 0;

// 	return str;
// }

void disp_int(int input)
{
	char output[16];
	itoa(output, input);
	DispStr(output);
}

static char buf[1024];

// #define __va_rounded_size(TYPE)  \
//   (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
// typedef char *va_list;
// #define va_start(AP, LASTARG) 						\
//  (__builtin_saveregs (),						\
//   AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))

// void va_end (char *);		/* Defined in gnulib */
// #define va_end(AP)

// int printk(const char *fmt, ...)
// {
// 	va_list args;
// 	int i;

// 	va_start(args, fmt);
// 	i=vsprintf(buf,fmt,args);
// 	va_end(args);
// 	__asm__("push %%fs\n\t"
// 		"push %%ds\n\t"
// 		"pop %%fs\n\t"
// 		"pushl %0\n\t"
// 		"pushl $_buf\n\t"
// 		"pushl $0\n\t"
// 		"call _tty_write\n\t"
// 		"addl $8,%%esp\n\t"
// 		"popl %0\n\t"
// 		"pop %%fs"
// 		::"r" (i):"ax","cx","dx");
// 	return i;
// }