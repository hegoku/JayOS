#include <system/list.h>
#include <system/mm.h>
#include <sys/types.h>
#include "global.h"

struct list *create_list(void *value)
{
    struct list *a = kzmalloc(sizeof(struct list));
    if (a==NULL) {
        printk("Can't malloc list\n");
        return NULL;
    }
    a->value = value;
    return a;
}

void del_list(struct list *l)
{
    kfree(l, sizeof(struct list));
}