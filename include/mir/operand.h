#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>

enum mir_operand_kind {
    mem,
    reg,
    imme,
};
struct mir_operand {
    mir_operand_kind kind;
    union {
        struct {
            union {
                p_mir_temp_sym p_temp_sym;
                p_symbol_sym p_sym;
            };
        };
        struct {
            union {
                int intconst;
                float floatconst;
            };
            basic_type b_type;
        };
    };
};

#endif