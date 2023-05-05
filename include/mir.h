#include<util.h>
#include<hir/symbol.h>

typedef struct mir_func mir_func, *p_mir_func;
typedef struct mir_linear_instr mir_linear_instr, *p_mir_linear_instr;
typedef struct mir_basic_block mir_basic_block, *p_mir_basic_block;

typedef struct mir_param mir_param, *p_mir_param;
typedef struct mir_param_list mir_param_list, *p_mir_param_list;

typedef struct mir_symbol mir_symbol, *p_mir_symbol;
typedef struct mir_operand mir_operand, *p_mir_operand;


typedef struct mir_instr mir_instr, *p_mir_instr;

typedef struct mir_binary_instr mir_binary_instr, *p_mir_binary_instr;
typedef struct mir_unary_instr mir_unary_instr, *p_mir_unary_instr;
typedef struct mir_array_instr mir_array_instr, *p_mir_array_instr;
typedef struct mir_call_instr mir_call_instr, *p_mir_call_instr;
typedef struct mir_ret_instr mir_ret_instr, *p_mir_ret_instr;
typedef struct mir_br_instr mir_br_instr, *p_mir_br_instr;
typedef struct mir_condbr_instr mir_condbr_instr, *p_mir_condbr_instr;
// typedef struct mir_alloca_instr mir_alloca_instr, *p_mir_alloca_instr;
// typedef struct mir_store_instr mir_store_instr, *p_mir_store_instr;
// typedef struct mir_load_instr mir_load_instr, *p_mir_load_instr;
