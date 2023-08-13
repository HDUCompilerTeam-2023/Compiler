#ifndef __IR_VMEM_BASE__
#define __IR_VMEM_BASE__
#include <ir.h>
#include <program/use.h>
struct ir_param_vmem_base {
    p_ir_vmem_base p_param_base;
    p_ir_vreg p_vreg;
    p_symbol_func p_func;
    size_t id;
    list_head node;
};
struct ir_vmem_base {
    bool is_vmem;
    union {
        p_symbol_var p_vmem_base;
        p_ir_param_vmem_base p_param_base;
    };
    size_t num;
    list_head varray_list;
    size_t id;
    void *p_info;
};

struct ir_varray {
    p_ir_vmem_base p_base;
    size_t id;
    enum {
        varray_func_def,
        varray_bb_phi_def,
        varray_instr_def,
        varray_global_def,
    } varray_def_type;
    union {
        p_ir_instr p_instr_def;
        p_ir_varray_bb_phi p_varray_bb_phi;
        p_symbol_func p_func;
    };
    list_head use_list;

    void *p_info;
    list_head node;
};

struct ir_varray_use {
    p_ir_varray p_varray_use;
    enum {
        varray_bb_param_use,
        varray_instr_use,
    } varray_use_type;
    union {
        p_ir_instr p_instr;
        p_ir_varray_bb_param p_varray_param;
    };
    list_head node;
};

struct ir_varray_def_pair {
    p_ir_varray p_des;
    p_ir_varray_use p_src;
    list_head node;
};
#endif