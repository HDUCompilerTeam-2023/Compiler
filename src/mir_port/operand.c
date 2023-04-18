#include <mir_port/operand.h>
#include <mir/operand.h>
mir_operand_kind mir_operand_get_kind(p_mir_operand p_operand)
{
    return p_operand->kind;
}

// 函数、全局变量使用名字
char* mir_operand_get_sym_name(p_mir_operand p_operand)
{
    return p_operand->name;
}
// 局部、临时变量使用 id
size_t mir_operand_get_sym_id(p_mir_operand p_operand)
{
    return p_operand->id;
}

int mir_operand_get_int(p_mir_operand p_operand)
{
    return p_operand->intconst;
}
float mir_operand_get_float(p_mir_operand p_operand)
{
    return p_operand->floatconst;
}
// 数组临时变量和局部、全局变量都使用 p_symbol_type
p_symbol_type mir_operand_get_sym_type(p_mir_operand p_operand)
{
    return p_operand->p_type;
}
basic_type mir_operand_get_basic_type(p_mir_operand p_operand)
{
    return p_operand->b_type;
}