#ifndef __SYMBOL_STORE__
#define __SYMBOL_STORE__

#include <symbol.h>

struct symbol_store {
    list_head variable;
    list_head constant;
    list_head function;
    list_head string;

    uint64_t next_id;
};

#endif