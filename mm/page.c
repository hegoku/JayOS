#include <sys/types.h>
#include <system/mm.h>
#include <system/page.h>
#include <string.h>

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
     for (int i = 0; i < 1024; i++) {
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