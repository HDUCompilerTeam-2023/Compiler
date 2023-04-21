#ifndef __SYMBOL_STORE__
#define __SYMBOL_STORE__

#include <symbol.h>

struct symbol_store {
    list_head global;
    list_head def_function;
    list_head string;
};

#endif