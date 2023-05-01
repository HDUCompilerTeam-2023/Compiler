#include <assert.h>
#include <stdlib.h>
#include <util/bitmap.h>
#include <stdio.h>
p_bitmap bitmap_gen(size_t element_num) {
    p_bitmap p_b = malloc(sizeof(*p_b));
    p_b->size = (element_num + UNIT_LENGTH - 1) / UNIT_LENGTH;
    p_b->p_base = malloc(p_b->size * sizeof(BASIC_T));
    return p_b;
}
p_bitmap bitmap_merge_new(p_bitmap p_b1, p_bitmap p_b2) {
    assert(p_b1->size == p_b2->size);
    p_bitmap p_new = bitmap_gen(p_b1->size);
    p_new->size = p_b1->size;
    for (size_t i = 0; i < p_b1->size; i++)
        p_new->p_base[i] = p_b1->p_base[i] | p_b2->p_base[i];
    return p_new;
}
p_bitmap bitmap_cut_new(p_bitmap p_b1, p_bitmap p_b2) {
    assert(p_b1->size == p_b2->size);
    p_bitmap p_new = bitmap_gen(p_b1->size);
    p_new->size = p_b1->size;
    for (size_t i = 0; i < p_b1->size; i++)
        p_new->p_base[i] = p_b1->p_base[i] & ~p_b2->p_base[i];
    return p_new;
}
p_bitmap bitmap_and_new(p_bitmap p_b1, p_bitmap p_b2) {
    assert(p_b1->size == p_b2->size);
    p_bitmap p_new = bitmap_gen(p_b1->size);
    p_new->size = p_b1->size;
    for (size_t i = 0; i < p_b1->size; i++)
        p_new->p_base[i] = p_b1->p_base[i] & p_b2->p_base[i];
    return p_new;
}

p_bitmap bitmap_neg_new(p_bitmap p_b1)
{
    p_bitmap p_new = bitmap_gen(p_b1->size);
    p_new->size = p_b1->size;
    for (size_t i = 0; i < p_b1->size; i++)
        p_new->p_base[i] = ~p_b1->p_base[i];
    return p_new;
}

p_bitmap bitmap_copy_new(p_bitmap p_b1)
{
    p_bitmap p_new = bitmap_gen(p_b1->size * UNIT_LENGTH);
    p_new->size = p_b1->size;
    for (size_t i = 0; i < p_b1->size; i++)
        p_new->p_base[i] = p_b1->p_base[i];
    return p_new;
}

void bitmap_merge_not_new(p_bitmap p_b1, p_bitmap p_b2)
{
    assert(p_b1->size == p_b2->size);
    for (size_t i = 0; i < p_b1->size; i++)
        p_b1->p_base[i] = p_b1->p_base[i] | p_b2->p_base[i];
}

void bitmap_cut_not_new(p_bitmap p_b1, p_bitmap p_b2)
{
    assert(p_b1->size == p_b2->size);
    for (size_t i = 0; i < p_b1->size; i++)
        p_b1->p_base[i] = p_b1->p_base[i] & ~p_b2->p_base[i];
}

void bitmap_and_not_new(p_bitmap p_b1, p_bitmap p_b2)
{
    assert(p_b1->size == p_b2->size);
    for (size_t i = 0; i < p_b1->size; i++)
        p_b1->p_base[i] = p_b1->p_base[i] & p_b2->p_base[i];
}

void bitmap_neg_not_new(p_bitmap p_b1)
{
    for (size_t i = 0; i < p_b1->size; i++)
        p_b1->p_base[i] = ~p_b1->p_base[i];
}

void bitmap_copy_not_new(p_bitmap p_des, p_bitmap p_src)
{
    assert(p_src->size == p_des->size);
    for (size_t i = 0; i < p_src->size; i++)
        p_des->p_base[i] = p_src->p_base[i];
}

bool bitmap_if_in(p_bitmap p_b, size_t id) {
    assert((id / UNIT_LENGTH) < p_b->size);
    return (bool)((p_b->p_base[id / UNIT_LENGTH] >> (id % UNIT_LENGTH)) & (BASIC_T)1);
}

bool bitmap_if_equal(p_bitmap p_b1, p_bitmap p_b2)
{
    assert(p_b1->size == p_b2->size);
    for(size_t i = 0; i < p_b1->size; i ++)
        if((bool)((p_b1->p_base + i) != (p_b2->p_base + i)))
            return false;
    return true;
}

void bitmap_add_element(p_bitmap p_b, size_t id)
{
    assert((id / UNIT_LENGTH) < p_b->size);
    p_b->p_base[id / UNIT_LENGTH] |= (BASIC_T)1 << (id % UNIT_LENGTH);
}

void bitmap_cut_element(p_bitmap p_b, size_t id)
{
    assert((id / UNIT_LENGTH) < p_b->size);
    p_b->p_base[id / UNIT_LENGTH] &= ~((BASIC_T) 1 << (id % UNIT_LENGTH));
}

void bitmap_set_empty(p_bitmap p_b)
{
    for(size_t i = 0; i < p_b->size; i ++)
        p_b->p_base[i] = (BASIC_T) 0;
}

void bitmap_set_full(p_bitmap p_b) {
    for (size_t i = 0; i < p_b->size; i++)
        p_b->p_base[i] = ~(BASIC_T) 0;
}

void bitmap_print(p_bitmap p_b)
{
    printf("{");
    bool first = false;
    for(size_t i = p_b->size - 1; i < p_b->size; i --){
        for(size_t j = UNIT_LENGTH - 1; j < UNIT_LENGTH; j --){
            if (bitmap_if_in(p_b, (i * UNIT_LENGTH) + j)){
                if (first){
                    printf(", ");
                }
                printf("%ld", (i * UNIT_LENGTH) + j);
                first = true;
            }
        }
    }
    printf("}");
}
void bitmap_drop(p_bitmap p_b) {
    free(p_b->p_base);
    free(p_b);
}