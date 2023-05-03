#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>

enum mir_operand_kind {
    declared_var,
    temp_var,
    immedicate_val,
};
struct mir_operand {
    mir_operand_kind kind;
    union {
        struct {
            union {
                p_mir_temp_sym p_temp_sym;
                p_symbol_sym p_sym;
            };
            size_t ssa_id;
        };
        struct {
            union {
                INTCONST_t intconst;
                FLOATCONST_t floatconst;
            };
            basic_type b_type;
        };
    };
};

#endif