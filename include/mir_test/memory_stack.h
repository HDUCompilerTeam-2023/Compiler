#ifndef __MIR_TEST_MEMORY_STACK__
#define __MIR_TEST_MEMORY_STACK__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Memory_stack memory_stack;
typedef struct Memory_type memory_type;

const int I32, F32;

struct Memory_type {
    union {
        int i;
        float f;
    };

    int type;
};

struct Memory_stack {
    size_t mem_cnt;

    size_t *pos;
    memory_type *mem;

    memory_stack *prev;
};

void stack_push(memory_stack **top);
void stack_pop(memory_stack **top);

#endif