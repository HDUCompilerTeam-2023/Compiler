#ifndef __SYMBOL__
#define __SYMBOL__

#include <util.h>

typedef struct symbol_store symbol_store, *p_symbol_store;
typedef struct symbol_init_val symbol_init_val, *p_symbol_init_val;
typedef struct symbol_init symbol_init, *p_symbol_init;
typedef struct symbol_sym symbol_sym, *p_symbol_sym;
typedef struct symbol_str symbol_str, *p_symbol_str;

typedef enum {
    type_void,
    type_str,
    type_int,
    type_float,
} basic_type;
typedef struct symbol_type symbol_type, *p_symbol_type;

#endif