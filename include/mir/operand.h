#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>
struct mir_symbol{
    union{
        size_t id; // 局部临时变量符号编号
        char *name; // 全局变量、函数符号, 是否需要 symbol 的全部信息？
    };
    enum{
        global_var, 
        local_var,
        temp_var,
    }irsym_kind;
    p_symbol_type p_type;
};

struct mir_operand{
    union{
        p_mir_symbol p_sym;
        int intconst;
        float floatconst;
    };
    enum{
         int_val,
         float_val,
         sym,
    }irop_kind;
};




#endif