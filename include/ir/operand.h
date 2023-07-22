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
                I32CONST_t i32const;
                F32CONST_t f32const;
                p_symbol_str strconst;
                struct {
                    p_symbol_var p_vmem;
                    I32CONST_t offset;
                };
            };
        };
    };
    enum {
        bb_param_ptr,
        cond_ptr,
        ret_ptr,
        instr_ptr,
    } used_type;
    union {
        p_ir_bb_param p_bb_param;
        p_ir_basic_block p_basic_block;
        p_ir_instr p_instr;
    };
    p_symbol_type p_type;
};

#endif
