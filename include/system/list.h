#ifndef __SYSTEM_LIST_H
#define __SYSTEM_LIST_H

#include <sys/types.h>

struct list{
    struct list *prev;
    void *value;
    struct list *next;
};

#define list_entry(ptr, type) ((type *)(ptr)->value)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos; pos = pos->next)


// #define LIST_HEAD_INIT(name) { &(name), &(name) }

// #define LIST_HEAD(name) \
// 	struct list_head name = LIST_HEAD_INIT(name)

// static inline void INIT_LIST_HEAD(struct list_head *list)
// {
// 	list->prev = list;
// }

// #define INIT_LIST_HEAD(ptr) do { \
// 	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
// } while (0)

// #define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

// #define container_of(ptr, type, member) ({			\
//         const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
//         (type *)( (char *)__mptr - offsetof(type,member) );})

// #define list_entry(ptr, type, member) container_of(ptr, type, member)

// #define list_first_entry(ptr, type, member) \
// 	list_entry((ptr)->next, type, member)

// #define list_last_entry(ptr, type, member) \
// 	list_entry((ptr)->prev, type, member)

// #define list_for_each(pos, head) \
// 	for (pos = (head)->next; pos != (head); pos = pos->next)

// #define list_next_entry(pos, member) \
// 	list_entry((pos)->member.next, typeof(*(pos)), member)
    
// #define list_for_each_entry(pos, head, member)				\
// 	for (pos = list_first_entry(head, typeof(*pos), member);	\
// 	     &pos->member != (head);					\
// 	     pos = list_next_entry(pos, member))

// static inline void __list_add(struct list_head *new, struct list_head *prev, struct list_head *next)
// {
// 	next->prev = new;
// 	new->next = next;
// 	new->prev = prev;
// }

static inline void list_add(struct list *new_list, struct list *head)
{
	if (head->next==NULL) {
		head->next = new_list;
        new_list->prev = head;
    }
    else
    {
        new_list->next = head->next;
        new_list->next->prev=new_list;
        new_list->prev = head;
		head->next = new_list;
	}
}

struct list *create_list(void *value);
void del_list(struct list *l);

#endif