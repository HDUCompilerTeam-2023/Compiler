#ifndef __SYMBOL_STR__
#define __SYMBOL_STR__

#include <symbol.h>

struct symbol_str {
    char * string;
    size_t length;

    hlist_node h_node;
    list_head node;
};

#endif