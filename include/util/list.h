#ifndef __UTIL_LIST__
#define __UTIL_LIST__

#include <stdbool.h>

typedef struct list_head list_head, *p_list_head;
struct list_head {
    p_list_head p_prev, p_next;
};

#define list_init_head(p_head) ((list_head) { p_head, p_head })
#define list_head_init(p_head) (*(p_head) = list_init_head(p_head))
#define list_head_alone(p_head) ((p_head)->p_next == (p_head))

#define list_entry(p_list_node, type, member) \
    ((type *) ((size_t)(p_list_node) - (size_t)(&((type *)0)->member)))

#define list_for_each(p_list_node, p_list_head) \
    for ((p_list_node) = (p_list_head)->p_next; (p_list_node) != (p_list_head); (p_list_node) = (p_list_node)->p_next)

#define list_for_each_tail(p_list_node, p_list_head) \
    for ((p_list_node) = (p_list_head)->p_prev; (p_list_node) != (p_list_head); (p_list_node) = (p_list_node)->p_prev)

bool list_add_next(p_list_head p_new, p_list_head p_pos);
bool list_add_prev(p_list_head p_new, p_list_head p_pos);
bool list_del(p_list_head p_del);
bool list_replace(p_list_head p_new, p_list_head p_old);
bool list_blk_add_next(p_list_head p_new, p_list_head p_pos);
bool list_blk_add_prev(p_list_head p_new, p_list_head p_pos);

#endif