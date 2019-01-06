#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include "kernel.h"
#include <system/dev.h>
#include <string.h>

void disp_int(int input)
{
	char output[16];
	itoa(output, input);
	DispStr(output);
}

int printk(const char * format, ...)
{
    int i;
    char buf[1024];
    va_list arg = (va_list)((char*)(&format)+4);//4为format所占堆栈中大小
    i = vsprintf(buf, format, arg);
    tty_write(0, buf, i);
    return i;
}

void panic(const char * info)
{
    printk("Kernel panic: %s\n",info);
	for(;;);
}
