#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>

enum mir_operand_kind{
    declared_var,
    temp_var,
    immedicate_int_val,
    immedicate_float_val,
    immedicate_void_val,
};
struct mir_operand{
    mir_operand_kind kind;
    union{
        p_mir_temp_sym p_temp_sym;
        p_symbol_sym p_sym;
        int intconst;
        float floatconst;
    };
};



#endif