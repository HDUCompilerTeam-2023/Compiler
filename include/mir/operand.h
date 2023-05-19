#ifndef __MIRSYMBOL__
#define __MIRSYMBOL__

#include <mir.h>

enum mir_operand_kind {
    reg,
    imme,
};
struct mir_operand {
    mir_operand_kind kind;
    union {
        struct {
            p_mir_vreg p_vreg;
            list_head use_node;
        };
        struct {
            union {
                INTCONST_t intconst;
                FLOATCONST_t floatconst;
                p_symbol_str strconst;
            };
        };
    };
    basic_type b_type;
    size_t ref_level;
};

#endif