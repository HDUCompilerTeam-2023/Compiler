#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>

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