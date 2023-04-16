#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>

enum mir_operand_kind{
    global_var,
    local_var,
    temp_var,
    immedicate_val,
};
struct mir_operand{
    mir_operand_kind kind;
    union{
        size_t id;
        char* name;
        int intconst;
        float floatconst;
    };
    union {
        p_symbol_type p_type;
        basic_type b_type;
    };
};




#endif