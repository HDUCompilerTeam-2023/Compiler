
#include <mir_gen.h>
#include <mir_gen/operand.h>

p_mir_operand mir_operand_int_gen(int intconst)
{
    p_mir_operand p_mir_int = malloc(sizeof(*p_mir_int));
    *p_mir_int = (mir_operand){
        .intconst = intconst,
        .kind = immedicate_val,
        .b_type = type_int,
    };
    return p_mir_int;
}

p_mir_operand mir_operand_float_gen(float floatconst)
{
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand){
        .floatconst = floatconst,
        .kind = immedicate_val,
        .b_type = type_float,
    };
    return p_mir_float;
}

p_mir_operand mir_operand_void_gen(void)
{
    p_mir_operand p_mir_float = malloc(sizeof(*p_mir_float));
    *p_mir_float = (mir_operand){
        .kind = immedicate_val,
        .b_type = type_void,
    };
    return p_mir_float;
}

p_mir_operand mir_operand_declared_sym_gen(p_symbol_sym p_h_sym)
{
    p_mir_operand p_sym = malloc(sizeof(*p_sym));
    *p_sym = (mir_operand){
        .p_type = p_h_sym->p_type,
    };
    if (p_h_sym->is_global) {
        p_sym->name = p_h_sym->name;
        p_sym->kind = global_var;
    }
    else{
        p_sym->id = p_h_sym->id;
        p_sym->kind = local_var;
    }
    return p_sym;
}

p_mir_operand mir_operand_temp_sym_gen(size_t id, p_symbol_type p_type)
{
    p_mir_operand p_sym = malloc(sizeof(*p_sym));
    *p_sym = (mir_operand){
        .kind = temp_var,
        .id = id,
        .p_type = p_type,
    };
    return p_sym;
}
// 临时变量在一些情况下需要将 立即数的基本类型转换为 type
p_symbol_type mir_operand_sym_type_gen(basic_type b_type)
{
    p_symbol_type p_type = malloc(sizeof(*p_type));
    *p_type = (symbol_type){
        .kind = type_var,
        .basic = b_type,
    };
    return p_type;
}

void mir_operand_drop(p_mir_operand p_operand)
{
    assert(p_operand);
    free(p_operand);
}