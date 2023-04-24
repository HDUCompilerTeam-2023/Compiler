#include <mir_port/operand.h>
#include <mir/operand.h>
#include <mir/temp_sym.h>

#include <symbol/sym.h>
#include <symbol/type.h>
mir_operand_kind mir_operand_get_kind(p_mir_operand p_operand)
{
    return p_operand->kind;
}

static inline bool mir_operand_can_get_name(p_mir_operand p_operand)
{
    return (p_operand->kind == declared_var && (p_operand->p_sym->is_global || p_operand->p_sym->p_type->kind == type_func));
}

// 函数、全局变量使用名字
char* mir_operand_get_sym_name(p_mir_operand p_operand)
{
    assert(mir_operand_can_get_name(p_operand));
    return p_operand->p_sym->name;
}
// 局部、临时变量使用 id
size_t mir_operand_get_sym_id(p_mir_operand p_operand)
{
    assert(!mir_operand_can_get_name(p_operand));
    if(p_operand->kind == temp_var)
        return p_operand->p_temp_sym->id;
    else 
        return p_operand->p_sym->id;
}

int mir_operand_get_int(p_mir_operand p_operand)
{
    return p_operand->intconst;
}
float mir_operand_get_float(p_mir_operand p_operand)
{
    return p_operand->floatconst;
}