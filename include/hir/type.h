#ifndef __HIR_TYPE__
#define __HIR_TYPE__

#include <util.h>

typedef enum basic_type basic_type;
enum basic_type {
    type_void,
    type_int,
    type_float,
};

typedef struct symbol_type symbol_type, *p_symbol_type;
struct symbol_type {
    enum {
        type_var, type_arrary,
        type_func, type_param,
    } kind;
    union {
        p_symbol_type p_item;
        basic_type basic;
    };
    union {
        size_t size;
        p_symbol_type p_params;
    };
};

struct type {
    enum {
        tval, tarr,
        tfunc, tparam,
    };
    union {
        struct type * p_son;
        basic_type basic;
    };
    struct type * params;
};

p_symbol_type symbol_type_var_gen(basic_type basic);
p_symbol_type symbol_type_arrary_gen(size_t size);
p_symbol_type symbol_type_func_gen(void);
p_symbol_type symbol_type_param_gen(p_symbol_type p_param);

void symbol_type_drop(p_symbol_type p_type);
void symbol_type_print(p_symbol_type p_type);

#endif
