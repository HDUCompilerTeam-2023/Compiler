#ifndef __IRSYMBOL__
#define __IRSYMBOL__

#include <ir.h>

enum ir_operand_kind {
    reg,
    imme,
};
struct ir_operand {
    ir_operand_kind kind;
    union {
        struct {
            p_ir_vreg p_vreg;
            list_head use_node;
        };
        struct {
            union {
                INTCONST_t intconst;
                FLOATCONST_t floatconst;
                p_symbol_str strconst;
                p_symbol_var p_vmem;
            };
        };
    };
    p_symbol_type p_type;
};

#endif
