#include <sys/types.h>
#include <system/mm.h>
#include <system/page.h>
#include <string.h>

unsigned int page_start = 1;
unsigned int  page_table_count = 1;
unsigned int kernel_page_count = 1;

inline struct PageDir *create_dir()
{
    return kzmalloc(sizeof(struct PageDir));
}

inline struct PageTable *create_table()
{
    return kzmalloc(sizeof(struct PageTable));
}

//清除并释放pd下所有pt
inline void clearDir(struct PageDir *pd)
{
    for (int i = 0; i < 1024; i++) {
        if (pd->entry[i]!= NULL) {
            kfree(pd->entry[i], sizeof(struct PageTable));
            pd->entry[i] = NULL;
        }
    }
}

//清除并释放pd
inline void removeDir(struct PageDir *pd)
{
    clearDir(pd);
    kfree(pd, sizeof(struct PageDir));
}

void copy_page(struct PageDir *pd, struct PageDir **res)
{
     for (int i = 0; i < 768; i++) {
        if (pd->entry[i]!= NULL) {
            (*res)->entry[i] = create_table();
            for (int j = 0; i < 1024;j++) {
                if (pd->entry[i]->entry[j]!= 0) {
                    (*res)->entry[i]->entry[j] = (unsigned int)kzmalloc(1024*4);
                    memcpy((void*)((*res)->entry[i]->entry[j]), (void*)pd->entry[i]->entry[j], 1024*4);
                }
                else
                {
                    (*res)->entry[i]->entry[j] = 0;
                }
            }
        }
        else
        {
            (*res)->entry[i] = NULL;
        }
    }
}

void init_process_page(struct PageDir **res)
{
    *res = create_dir();

    // struct PageTable *pt = (struct PageTable *)kzmalloc(page_table_count * 1024 * 4);
    unsigned int a = 0;
    int k = 768;
    // pt += 1024 * (page_table_count - kernel_page_count);
    for (int i = page_table_count - kernel_page_count; i < page_table_count; i++)
    {
        struct PageTable *pt = create_table();
        (*res)->entry[k] = (void *)((unsigned int)pt | PG_P | PG_RWW | PG_USS);
        for (int j = 0; j < 1024; j++)
        {
            pt->entry[j] = a | PG_P | PG_RWW | PG_USS;
            a += 4096;
        }
        k++;
        // pt += 1024;
    }
}