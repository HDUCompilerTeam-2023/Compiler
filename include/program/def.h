#ifndef __PROGRAM_DEF__
#define __PROGRAM_DEF__

#include <util.h>

struct program {
    list_head variable;
    list_head constant;
    list_head v_memory;
    list_head function;
    list_head string;

    uint64_t variable_cnt;
    uint64_t constant_cnt;
    uint64_t v_memory_cnt;
    uint64_t function_cnt;
};

#endif
