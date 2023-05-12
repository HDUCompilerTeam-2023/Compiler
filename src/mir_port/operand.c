#include <mir/operand.h>
#include <mir/vreg.h>
#include <mir_port/operand.h>

#include <symbol/sym.h>
#include <symbol/type.h>
mir_operand_kind mir_operand_get_kind(p_mir_operand p_operand) {
    return p_operand->kind;
}

// 局部、临时变量使用 id
size_t mir_operand_get_sym_id(p_mir_operand p_operand) {
    return p_operand->p_vreg->id;
}
INTCONST_t mir_operand_get_int(p_mir_operand p_operand) {
    return p_operand->intconst;
}
FLOATCONST_t mir_operand_get_float(p_mir_operand p_operand) {
    return p_operand->floatconst;
}
