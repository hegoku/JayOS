#include <system/fs.h>
#include <string.h>
#include "../kernel/kernel.h"
#include "../kernel/global.h"

int lookup(struct dir_entry *dir, char *name, int len, struct dir_entry **res_dir);
static int get_parent_dir_entry(char *pathname, struct dir_entry *base, struct nameidata **nd);

static int get_parent_dir_entry(char *pathname, struct dir_entry *base, struct nameidata **nd)
{
    struct dir_entry *dir;
    char *thisname;
    int len, error;
    char c;
    (*nd)->dir = 0;

    if (!base) {
        base = current_process->pwd;
    }
    if (pathname[0] == '/')
    {
        base = current_process->root;
    }
    pathname++;

    while (1)
    {
        thisname = pathname;
        for (len = 0; (c = *(pathname++)) && (c != '/'); len++)
        {
        }

        if (!c) {
            break;
        }

        error = lookup(base, thisname, len, &dir);
        if (error) {
            printk("get_parent_dir_entry error\n");
            return error;
        }        
    }

    (*nd)->last_len = len;
    (*nd)->last_name = thisname;
    (*nd)->dir = dir;
    return 0;
}

int lookup(struct dir_entry *dir, char *name, int len, struct dir_entry **res_dir)
{
    *res_dir = 0;

    if (len==2 && name[0]=='.' && name[1]=='.') {
        if (dir==current_process->root) {
            *res_dir = dir;
            return 0;
        }
    }

    if (!len) {
        *res_dir = dir;
        return 0;
    }

    for (struct list *i = dir->children; i; i = i->next)
    {
        struct dir_entry *tmp = (struct dir_entry *)i->value;
        if (strcmp(name, tmp->name) == 0)
        {
            *res_dir = tmp;
            return 0;
        }
    }

    return -1;
}