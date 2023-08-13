#ifndef __IR_BB_PARAM__
#define __IR_BB_PARAM__

#include <ir.h>
struct ir_bb_param {
    p_ir_operand p_bb_param;
    p_ir_basic_block_branch_target p_target;
    list_head node;
};

struct ir_bb_phi {
    p_ir_vreg p_bb_phi;
    p_ir_basic_block p_basic_block;
    list_head node;
};

struct ir_varray_bb_phi {
    p_ir_varray p_varray_phi;
    p_ir_basic_block p_basic_block;
    list_head varray_param_list;
    list_head node;
};

struct ir_varray_bb_param {
    p_ir_varray_use p_varray_bb_param;
    p_ir_basic_block_branch_target p_target;
    p_ir_varray_bb_phi p_des_phi;
    list_head phi_node;
    list_head node;
};
#endif
