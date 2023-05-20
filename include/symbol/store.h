#ifndef __SYMBOL_STORE__
#define __SYMBOL_STORE__

#include <symbol.h>

struct symbol_store {
    list_head variable;
    list_head function;
    list_head string;

    uint64_t variable_cnt;
    uint64_t function_cnt;
};

#endif