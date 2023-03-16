#include <util/list.h>

static inline void __list_add(p_list_head p_new, p_list_head p_prev, p_list_head p_next) {
    p_next->p_prev = p_new;
    p_new->p_next = p_next;
    p_new->p_prev = p_prev;
    p_prev->p_next = p_new;
}

bool list_add_next(p_list_head p_new, p_list_head p_pos) {
    if (!list_head_alone(p_new)) return false;
    __list_add(p_new, p_pos, p_pos->p_next);
    return true;
}

bool list_add_prev(p_list_head p_new, p_list_head p_pos) {
    if (!list_head_alone(p_new)) return false;
    __list_add(p_new, p_pos->p_prev, p_pos);
    return true;
}

static inline void __list_del(p_list_head p_prev, p_list_head p_next) {
    p_next->p_prev = p_prev;
    p_prev->p_next = p_next;
}

bool list_del(p_list_head p_del) {
    if (list_head_alone(p_del)) return false;
    __list_del(p_del->p_prev, p_del->p_next);
    list_head_init(p_del);
    return true;
}

static inline void __list_replace(p_list_head p_new, p_list_head p_pos) {
    p_new->p_next = p_pos->p_next;
    p_new->p_next->p_prev = p_new;
    p_new->p_prev = p_pos->p_prev;
    p_new->p_prev->p_next = p_new;
    list_head_init(p_pos);
}

bool list_replace(p_list_head p_new, p_list_head p_old) {
    if (!list_head_alone(p_new) || list_head_alone(p_old)) return false;
    __list_replace(p_new, p_old);
    list_head_init(p_old);
    return true;
}

static inline void __list_swap(p_list_head p_entry1, p_list_head p_entry2) {
    __list_replace(p_entry1, p_entry2);
}

bool list_swap(p_list_head p_entry1, p_list_head p_entry2) {
    if (list_head_alone(p_entry1) || list_head_alone(p_entry2)) return false;
    __list_swap(p_entry1, p_entry2);
    return true;
}

static inline void __list_add_list(p_list_head p_head, p_list_head p_prev, p_list_head p_next) {
    p_next->p_prev = p_head->p_prev;
    p_head->p_prev->p_next = p_next;
    p_head->p_next->p_prev = p_prev;
    p_prev->p_next = p_head->p_next;
}

bool list_blk_add_next(p_list_head p_new, p_list_head p_pos) {
    if (list_head_alone(p_new) || list_head_alone(p_pos)) return false;
    __list_add_list(p_new, p_pos, p_pos->p_next);
    return true;
}

bool list_blk_add_prev(p_list_head p_new, p_list_head p_pos) {
    if (list_head_alone(p_new) || list_head_alone(p_pos)) return false;
    __list_add_list(p_new, p_pos->p_prev, p_pos);
    return true;
}