#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>
enum mir_operand_kind{
    int_val,
    float_val,
    void_val, // 主要用做函数返回值 
    sym,
};
struct mir_operand{
    union{
        p_mir_symbol p_sym;
        int intconst;
        float floatconst;
    };
    mir_operand_kind kind;
};




#endif