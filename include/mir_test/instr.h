#ifndef __MIR_TEST_INSTR_DEFINE__
#define __MIR_TEST_INSTR_DEFINE__

#include <mir_test.h>

#include <mir/basic_block.h>

#include <util/list.h>

memory_type mir_instr_test(p_list_head p_list, memory_stack **global_stack, memory_stack **top_stack);
memory_type mir_ret_instr_test(p_mir_ret_instr p_instr, memory_stack *global_stack, memory_stack *top_stack);
void mir_binary_instr_test(mir_instr_type instr_type, p_mir_binary_instr p_instr, memory_stack *global_stack, memory_stack *top_stack);
void mir_unary_instr_test(mir_instr_type instr_type, p_mir_unary_instr p_instr, memory_stack *global_stack, memory_stack *top_stack);
void mir_array_assign_instr_test(p_mir_array_assign_instr p_instr, memory_stack *global_stack, memory_stack *top_stack);
void mir_array_instr_test(p_mir_array_instr p_instr, memory_stack *global_stack, memory_stack *top_stack);
void mir_call_instr_test(p_mir_call_instr p_instr, memory_stack **global_stack, memory_stack **top_stack);

#endif