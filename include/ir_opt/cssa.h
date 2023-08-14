#ifndef __IR_OPT_CSSA__
#define __IR_OPT_CSSA__

#include <program/use.h>
#include <ir/vreg.h>

typedef struct {
    p_ir_vreg p_vreg;
    list_head node;
} pcc_node, *p_pcc_node;

typedef struct {
    list_head set;
    list_head node;
    p_symbol_type p_mem_type;
} phi_congruence_class, *p_phi_congruence_class;

void phi_congruence_class_drop(p_phi_congruence_class pcc);
void phi_congruence_class_print(p_phi_congruence_class pcc);

size_t ir_opt_cssa_no_pcc(p_program p_ir);
size_t ir_opt_cssa(p_symbol_func p_func, p_list_head pcc_head);

#endif
