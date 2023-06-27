#ifndef __SYMBOL__
#define __SYMBOL__

#include <util.h>

typedef struct symbol_init_val symbol_init_val, *p_symbol_init_val;
typedef struct symbol_init symbol_init, *p_symbol_init;
typedef struct symbol_var symbol_var, *p_symbol_var;
typedef struct symbol_func symbol_func, *p_symbol_func;
typedef struct symbol_str symbol_str, *p_symbol_str;

typedef enum {
    type_void,
    type_str,
    type_i32,
    type_f32,
} basic_type;
typedef struct symbol_type symbol_type, *p_symbol_type;
typedef struct symbol_type_array symbol_type_array, *p_symbol_type_array;

#endif
