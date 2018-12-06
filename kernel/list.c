#include <system/list.h>

inline int list_empty(const struct list_head *head)
{
    return head->next == head;
}

inline void list_add(struct list_head *new, struct list_head *head)
{
    head->prev = new;
    new->next = head;
    head = new;
}