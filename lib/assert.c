#include <assert.h>
#include <stdio.h>
#include "../kernel/global.h"

void assertion_failure(char *exp, char *file, char *base_file, int line)
{
    if (is_in_ring0==0) {
        printk("assert(%s), failed: file: %s, base_file: %s, ln%d", exp, file, base_file, line);
    } else {
        printf("assert(%s), failed: file: %s, base_file: %s, ln%d", exp, file, base_file, line);
    }
    __asm__ __volatile__("ud2");
}