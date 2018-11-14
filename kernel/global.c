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
    tty_write(&tty, buf, i);
    return i;
}

int sys_get_ticks()
{
    // DispStr("\nAAAAA:");
    // disp_int(a);
    // DispStr("   BBBBB:");
    // disp_int(b);
    // DispStr("\n");
    return ticks;
}

int get_ticks()
{
    return sys_call_0_param(0);
}