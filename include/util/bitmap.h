#ifndef __UTIL_BITMAP__
#define __UTIL_BITMAP__

#include <stdbool.h>
#include <stddef.h>
// bitmap : 须保证元素从零开始编号
#define BASIC_T size_t
#define UNIT_LENGTH (sizeof(BASIC_T))

struct bitmap {
    size_t size;
    BASIC_T *p_base;
};

typedef struct bitmap bitmap, *p_bitmap;

p_bitmap bitmap_gen(size_t element_num);
p_bitmap bitmap_merge_new(p_bitmap p_b1, p_bitmap p_b2);
p_bitmap bitmap_cut_new(p_bitmap p_b1, p_bitmap p_b2);
p_bitmap bitmap_and_new(p_bitmap p_b1, p_bitmap p_b2);
p_bitmap bitmap_neg_new(p_bitmap p_b1);

p_bitmap bitmap_copy_new(p_bitmap p_b1);

bool bitmap_if_in(p_bitmap p_b, size_t id);
bool bitmap_if_equal(p_bitmap p_b1, p_bitmap p_b2);

void bitmap_merge_not_new(p_bitmap p_b1, p_bitmap p_b2);
void bitmap_cut_not_new(p_bitmap p_b1, p_bitmap p_b2);
void bitmap_and_not_new(p_bitmap p_b1, p_bitmap p_b2);
void bitmap_neg_not_new(p_bitmap p_b1);
void bitmap_copy_not_new(p_bitmap p_des, p_bitmap p_src);

void bitmap_add_element(p_bitmap p_b, size_t id);
void bitmap_cut_element(p_bitmap p_b, size_t id);

void bitmap_set_empty(p_bitmap p_b);
void bitmap_set_full(p_bitmap p_b);

void bitmap_print(p_bitmap p_b);
void bitmap_drop(p_bitmap b);
#endif