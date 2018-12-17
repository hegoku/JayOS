#include <sys/types.h>
#include <system/mm.h>
#include <system/page.h>

inline struct Page_Dir *create_dir()
{
    return kzmalloc(sizeof(struct Page_Dir));
}

inline struct Page_table *create_table()
{
    return kzmalloc(sizeof(struct Page_table));
}

//清除并释放pd下所有pt
inline void clearDir(struct Page_Dir *pd)
{
    for (int i = 0; i < 1024; i++) {
        if (pd->entry[i]!= NULL) {
            kfree(pd->entry[i], sizeof(struct Page_table));
            pd->entry[i] = NULL;
        }
    }
}

//清除并释放pd
inline void removeDir(struct Page_Dir *pd)
{
    clearDir(pd);
    kfree(pd, sizeof(struct Page_Dir));
}