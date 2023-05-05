#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mir.h>
#include <mir/operand.h>
#include <mir/temp_sym.h>

#include <mir_test.h>

#include <symbol/store.h>
#include <symbol/sym.h>
#include <symbol/type.h>

const int I32 = 0, F32 = 1;

const int STACK_MAX = 1e4;

void stack_push(memory_stack **top) {
    memory_stack *S = (memory_stack *) malloc(sizeof(memory_stack));
    S->mem_cnt = 0;
    S->pos = NULL;
    S->prev = NULL;
    S->mem = NULL;
    S->mem = (memory_type *) malloc(sizeof(memory_type) * STACK_MAX);
    S->pos = (size_t *) malloc(sizeof(size_t) * STACK_MAX);

    S->prev = (*top);
    (*top) = S;
}

void stack_pop(memory_stack **top) {
    memory_stack *s = (*top);
    (*top) = (*top)->prev;
    free(s->mem);
    free(s->pos);
    free(s);
}
