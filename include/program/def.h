#ifndef __PROGRAM_DEF__
#define __PROGRAM_DEF__

#include <util.h>

struct program {
    list_head variable;
    list_head function;
    list_head ext_function;
    list_head string;

    list_head arm_function;

    uint64_t variable_cnt;
    uint64_t function_cnt;
    uint64_t ext_function_cnt;

    char *input;
    char *output;
};

#endif
