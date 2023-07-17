#include <symbol.h>
#include <util.h>

typedef enum ir_basic_block_branch_kind ir_basic_block_branch_kind;
typedef struct ir_basic_block_branch ir_basic_block_branch, *p_ir_basic_block_branch;
typedef struct ir_basic_block ir_basic_block, *p_ir_basic_block;
typedef struct ir_basic_block_branch_target ir_basic_block_branch_target, *p_ir_basic_block_branch_target;

typedef struct ir_bb_param ir_bb_param, *p_ir_bb_param;
typedef struct ir_bb_phi ir_bb_phi, *p_ir_bb_phi;
typedef struct ir_vreg_list ir_vreg_list, *p_ir_vreg_list;
typedef struct ir_vreg_list_node ir_vreg_list_node, *p_ir_vreg_list_node;

typedef struct ir_basic_block_list_node ir_basic_block_list_node, *p_ir_basic_block_list_node;

typedef struct ir_param ir_param, *p_ir_param;
typedef struct ir_param_list ir_param_list, *p_ir_param_list;

typedef struct ir_operand ir_operand, *p_ir_operand;

typedef struct ir_vreg ir_vreg, *p_ir_vreg;

typedef enum ir_instr_type ir_instr_type;
typedef enum ir_binary_op ir_binary_op;
typedef enum ir_unary_op ir_unary_op;
typedef enum ir_operand_kind ir_operand_kind;

typedef struct ir_instr ir_instr, *p_ir_instr;

typedef struct ir_binary_instr ir_binary_instr, *p_ir_binary_instr;
typedef struct ir_unary_instr ir_unary_instr, *p_ir_unary_instr;
typedef struct ir_alloca_instr ir_alloca_instr, *p_ir_alloca_instr;
typedef struct ir_gep_instr ir_gep_instr, *p_ir_gep_instr;
typedef struct ir_load_instr ir_load_instr, *p_ir_load_instr;
typedef struct ir_store_instr ir_store_instr, *p_ir_store_instr;
typedef struct ir_call_instr ir_call_instr, *p_ir_call_instr;
typedef struct ir_ret_instr ir_ret_instr, *p_ir_ret_instr;
typedef struct ir_br_instr ir_br_instr, *p_ir_br_instr;
typedef struct ir_condbr_instr ir_condbr_instr, *p_ir_condbr_instr;
