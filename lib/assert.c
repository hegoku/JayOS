#include <assert.h>
#include "../kernel/global.h"

void assertion_failure(char *exp, char *file, char *base_file, int line)
{
    printk("assert(%s), failed: file: %s, base_file: %s, ln%d", exp, file, base_file, line);
    __asm__ __volatile__("ud2");
}