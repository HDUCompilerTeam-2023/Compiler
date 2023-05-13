#include <symbol.h>
#include <util.h>

typedef struct mir_program mir_program, *p_mir_program;
typedef struct mir_func mir_func, *p_mir_func;

typedef enum mir_basic_block_branch_kind mir_basic_block_branch_kind;
typedef struct mir_basic_block_branch mir_basic_block_branch, *p_mir_basic_block_branch;
typedef struct mir_basic_block mir_basic_block, *p_mir_basic_block;
typedef struct mir_basic_block_branch_target mir_basic_block_branch_target, *p_mir_basic_block_branch_target;

typedef struct mir_bb_param mir_bb_param, *p_mir_bb_param;
typedef struct mir_bb_param_list mir_bb_param_list, *p_mir_bb_param_list;
typedef struct mir_bb_phi mir_bb_phi, *p_mir_bb_phi;
typedef struct mir_bb_phi_list mir_bb_phi_list, *p_mir_bb_phi_list;

typedef struct mir_basic_block_list_node mir_basic_block_list_node, *p_mir_basic_block_list_node;

typedef struct mir_param mir_param, *p_mir_param;
typedef struct mir_param_list mir_param_list, *p_mir_param_list;

typedef struct mir_operand mir_operand, *p_mir_operand;

typedef struct mir_vreg mir_vreg, *p_mir_vreg;
typedef struct mir_vmem mir_vmem, *p_mir_vmem;

typedef enum mir_instr_type mir_instr_type;
typedef enum mir_operand_kind mir_operand_kind;

typedef struct mir_instr mir_instr, *p_mir_instr;

typedef struct mir_binary_instr mir_binary_instr, *p_mir_binary_instr;
typedef struct mir_unary_instr mir_unary_instr, *p_mir_unary_instr;
typedef struct mir_addr_instr mir_addr_instr, *p_mir_addr_instr;
typedef struct mir_load_instr mir_load_instr, *p_mir_load_instr;
typedef struct mir_store_instr mir_store_instr, *p_mir_store_instr;
typedef struct mir_call_instr mir_call_instr, *p_mir_call_instr;
typedef struct mir_ret_instr mir_ret_instr, *p_mir_ret_instr;
typedef struct mir_br_instr mir_br_instr, *p_mir_br_instr;
typedef struct mir_condbr_instr mir_condbr_instr, *p_mir_condbr_instr;
