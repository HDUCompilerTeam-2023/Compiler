#ifndef __UTIL_HLIST__
#define __UTIL_HLIST__

#include <stdbool.h>

typedef struct hlist_node hlist_node, *p_hlist_node, **pp_hlist_node;
typedef struct hlist_head hlist_head, *p_hlist_head, *hlist_hash;
struct hlist_node {
    p_hlist_node p_next;
    pp_hlist_node pp_prev;
};
struct hlist_head {
    p_hlist_node p_first;
};

#define hlist_init_node ((hlist_node) {.p_next = NULL, .pp_prev = NULL})
#define hlist_node_init(p_hlist_node) (*(p_hlist_node) = hlist_init_node)

#define hlist_init_head ((hlist_head) {.p_first = NULL})
#define hlist_head_init(p_hlist_head) (*(p_hlist_head) = hlist_init_head)

#define hlist_node_unhashed(hlist_node) (!(hlist_node)->pp_prev)
#define hlist_head_empty(hlist_head) (!(hlist_head)->p_first)

bool hlist_node_add(p_hlist_head p_head, p_hlist_node p_node);
bool hlist_node_del(p_hlist_node p_node);

#define hlist_entry(p_hlist_node, type, member) \
    ((type *) ((size_t)(p_hlist_node) - (size_t)(&((type *)0)->member)))
#define hlist_for_each(p_hlist_node, p_hlist_head) \
    for ((p_hlist_node) = (p_hlist_head)->p_first; (p_hlist_node); (p_hlist_node) = (p_hlist_node)->p_next)

#endif