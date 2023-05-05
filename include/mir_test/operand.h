#ifndef __MIR_TEST_OPERAND_DEFINE__
#define __MIR_TEST_OPERAND_DEFINE__

#include <mir_test/memory_stack.h>

#include <mir.h>

memory_type mir_operand_data_get(p_mir_operand p_operand, const memory_stack *global_stack, const memory_stack *stack);

#endif