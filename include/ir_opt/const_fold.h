#ifndef __IR_OPT_CONST_FOLD__
#define __IR_OPT_CONST_FOLD__

#include <ir/operand.h>

p_ir_operand ir_opt_const_fold(ir_binary_op b_op, p_ir_operand p_src1, p_ir_operand p_src2);

#endif
