#ifndef __SYMBOL_TYPE__
#define __SYMBOL_TYPE__

#include <symbol.h>

uint64_t symbol_type_get_size(p_symbol_type p_type);
uint64_t symbol_type_array_get_size(p_symbol_type_array p_array);

struct symbol_type_array {
    uint64_t size;
    list_head node;
};

struct symbol_type {
    list_head array;
    basic_type basic;
    uint64_t size;
};

#endif