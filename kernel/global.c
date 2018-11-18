#include "global.h"
#include <stdio.h>
#include <stdlib.h>
#include "kernel.h"

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
    tty_write(current_tty, buf, i);
    return i;
}
