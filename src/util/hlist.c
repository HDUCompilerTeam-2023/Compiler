#include <util/hlist.h>
#include <stddef.h>

static inline void __hlist_add(p_hlist_head p_head, p_hlist_node p_node) {
    p_node->p_next = p_head->p_first;
    p_head->p_first = p_node;

    if (p_node->p_next) p_node->p_next->pp_prev = &p_node->p_next;
    p_head->p_first->pp_prev = &p_head->p_first;
}
bool hlist_node_add(p_hlist_head p_head, p_hlist_node p_node) {
    if (!hlist_node_unhashed(p_node)) return false;
    __hlist_add(p_head, p_node);
    return true;
}

static inline void __hlist_del(p_hlist_node p_node) {
    *p_node->pp_prev = p_node->p_next;
    if (p_node->p_next) p_node->p_next->pp_prev = p_node->pp_prev;
}
bool hlist_node_del(p_hlist_node p_node) {
    if (hlist_node_unhashed(p_node)) return false;
    __hlist_del(p_node);
    hlist_node_init(p_node);
    return true;
}