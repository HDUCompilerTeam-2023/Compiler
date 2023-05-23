#ifndef __SYMBOL_STORE__
#define __SYMBOL_STORE__

#include <symbol.h>

struct program {
    list_head variable;
    list_head v_memory;
    list_head function;
    list_head string;

    uint64_t variable_cnt;
    uint64_t v_memory_cnt;
    uint64_t function_cnt;
};

#endif
