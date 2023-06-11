#include <frontend/syntax/info/def.h>

#include <frontend/syntax/init/gen.h>
#include <frontend/syntax/symbol_table/gen.h>
#include <frontend/syntax/decl_list/gen.h>
#include <frontend/syntax/decl_list/node/gen.h>

#include <program/gen.h>

#include <symbol_gen/func.h>
#include <symbol_gen/var.h>
#include <symbol_gen/type.h>

#include <hir2mir.h>

#include <hir_gen/exp.h>
#include <hir_gen/stmt.h>
#include <hir_gen/block.h>

p_program syntax_info_get_program(p_syntax_info p_info) {
    return p_info->p_program;
}

void syntax_zone_push(p_syntax_info p_info) {
    symbol_table_zone_push(p_info->p_table);
}
void syntax_zone_pop(p_syntax_info p_info) {
    symbol_table_zone_pop(p_info->p_table);
}
p_symbol_var syntax_find_var(p_syntax_info p_info, const char *name) {
    return symbol_table_var_find(p_info->p_table, name);
}
p_symbol_func syntax_find_func(p_syntax_info p_info, const char *name) {
    return symbol_table_func_find(p_info->p_table, name);
}

void syntax_func_add_constant(p_syntax_info p_info, p_symbol_var p_var) {
    symbol_table_var_add(p_info->p_table, p_var);
    symbol_func_add_constant(p_info->p_func, p_var);
}
void syntax_func_add_variable(p_syntax_info p_info, p_symbol_var p_var) {
    symbol_table_var_add(p_info->p_table, p_var);
    symbol_func_add_variable(p_info->p_func, p_var);
}
void syntax_func_add_param(p_syntax_info p_info, p_symbol_var p_var) {
    symbol_table_var_add(p_info->p_table, p_var);
    symbol_func_add_param(p_info->p_func, p_var);
}

p_symbol_str syntax_get_str(p_syntax_info p_info, const char *string) {
    p_symbol_str p_str = symbol_table_str_find(p_info->p_table, string);
    if (p_str) return p_str;

    p_str = symbol_table_str_add(p_info->p_table, string);
    program_add_str(p_info->p_program, p_str);
    return p_str;
}
void syntax_program_add_variable(p_syntax_info p_info, p_symbol_var p_var) {
    symbol_table_var_add(p_info->p_table, p_var);
    program_add_global(p_info->p_program, p_var);
}
void syntax_program_add_constant(p_syntax_info p_info, p_symbol_var p_var) {
    symbol_table_var_add(p_info->p_table, p_var);
    program_add_constant(p_info->p_program, p_var);
}
void syntax_program_add_function(p_syntax_info p_info, p_symbol_func p_func) {
    symbol_table_func_add(p_info->p_table, p_func);
    program_add_function(p_info->p_program, p_func);
}
void syntax_func_head(p_syntax_info p_info, basic_type type, char *name, p_syntax_decl_list p_decl_list) {
    p_symbol_func p_func = symbol_func_gen(name, type, false);
    syntax_program_add_function(p_info, p_func);
    p_info->p_func = p_func;
    free(name);

    syntax_zone_push(p_info);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, syntax_decl_list_get_list(p_decl_list)) {
        p_syntax_decl p_decl = syntax_decl_list_node_get_entry(p_node);
        list_del(syntax_decl_list_node_get_node(p_decl));

        p_symbol_type p_type = syntax_type_trans(syntax_decl_list_node_get_array(p_decl), syntax_decl_list_node_get_basic(p_decl));
        p_symbol_var p_var = symbol_var_gen(syntax_decl_list_node_get_name(p_decl), p_type, false, false, NULL);
        syntax_func_add_param(p_info, p_var);
        syntax_decl_drop(p_decl);
    }
    syntax_decl_list_drop(p_decl_list);
}
void syntax_func_end(p_syntax_info p_info, p_hir_block p_block) {
    syntax_zone_pop(p_info);
    p_symbol_func p_func = p_info->p_func;
    p_info->p_func = NULL;
    p_func->p_m_func = mir_func_gen();
    p_func->p_m_func->p_func_sym = p_func;
    hir2mir_func_gen(p_block, p_func->p_m_func, p_info->p_program);
    hir_block_drop(p_block);
}

static inline void syntax_rtlib_decl(p_syntax_info p_info, basic_type type, char *name, p_symbol_type p_param1, p_symbol_type p_param2, bool is_va) {
    p_symbol_func p_func = symbol_func_gen(name, type, is_va);
    syntax_program_add_function(p_info, p_func); // true
    p_info->p_func = p_func;

    syntax_zone_push(p_info);
    if (p_param1) {
        p_symbol_var p_var = symbol_var_gen("arg1", p_param1, false, false, NULL);
        syntax_func_add_param(p_info, p_var);
        if (p_param2) {
            p_var = symbol_var_gen("arg2", p_param2, false, false, NULL);
            syntax_func_add_param(p_info, p_var);
        }
    }
    syntax_zone_pop(p_info);
    p_info->p_func = NULL;
    p_func->p_m_func = mir_func_gen();
    p_func->p_m_func->p_func_sym = p_func;
}
void syntax_rtlib_func_init(p_syntax_info p_info) {
    syntax_rtlib_decl(p_info, type_int, "getint", NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_int, "getch", NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_float, "getfloat", NULL, NULL, false);

    p_symbol_type p_type = symbol_type_var_gen(type_int);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_int, "getarray", p_type, NULL, false);
    p_type = symbol_type_var_gen(type_float);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_int, "getfarray", p_type, NULL, false);

    syntax_rtlib_decl(p_info, type_void, "putint", symbol_type_var_gen(type_int), NULL, false);
    syntax_rtlib_decl(p_info, type_void, "putch", symbol_type_var_gen(type_int), NULL, false);
    syntax_rtlib_decl(p_info, type_void, "putfloat", symbol_type_var_gen(type_float), NULL, false);

    p_type = symbol_type_var_gen(type_int);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_void, "putarray", symbol_type_var_gen(type_int), p_type, false);
    p_type = symbol_type_var_gen(type_float);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_void, "putfarray", symbol_type_var_gen(type_int), p_type, false);

    syntax_rtlib_decl(p_info, type_void, "putf", symbol_type_var_gen(type_str), NULL, true);

    syntax_rtlib_decl(p_info, type_void, "starttime", NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_void, "stoptime", NULL, NULL, false);
}

static inline size_t syntax_init_by_assign_gen(p_syntax_init_mem p_init, size_t index, p_hir_exp p_addr, p_hir_block p_block) {
    if (list_head_alone(&p_addr->p_type->array)) {
        assert(p_addr->p_type->ref_level == 1);
        p_hir_stmt p_assign = hir_stmt_assign_gen(p_addr, syntax_init_mem_get_exp(p_init, index));
        syntax_init_mem_clear_exp(p_init, index++);
        hir_block_add(p_block, p_assign);
        return index;
    }
    p_hir_exp p_element_addr = hir_exp_gep_gen(p_addr, hir_exp_int_gen(0), true);
    size_t length = symbol_type_get_size(p_addr->p_type) / symbol_type_get_size(p_element_addr->p_type);
    for (size_t i = 1; i < length; ++i) {
        index = syntax_init_by_assign_gen(p_init, index, p_element_addr, p_block);
        p_element_addr = hir_exp_gep_gen(hir_exp_use_gen(p_element_addr), hir_exp_int_gen(1), false);
    }
    return syntax_init_by_assign_gen(p_init, index, p_element_addr, p_block);
}
p_hir_block syntax_local_vardecl(p_syntax_info p_table, p_hir_block p_block, p_syntax_decl_list p_decl_list) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, syntax_decl_list_get_list(p_decl_list)) {
        p_syntax_decl p_decl = syntax_decl_list_node_get_entry(p_node);
        list_del(p_node);

        p_symbol_type p_type = syntax_type_trans(syntax_decl_list_node_get_array(p_decl), syntax_decl_list_get_basic(p_decl_list));

        p_syntax_init_mem p_h_init = syntax_init_mem_gen(syntax_decl_list_node_get_init(p_decl), p_type);
        if (syntax_decl_list_get_is_const(p_decl_list)) {
            p_symbol_init p_init = symbol_init_gen(p_type->size, syntax_decl_list_get_basic(p_decl_list));
            for (size_t i = 0; i < p_type->size; ++i) {
                symbol_init_val init_val;
                if (p_init->basic == type_int) {
                    init_val.i = hir_exp_ptr_check_const(syntax_init_mem_get_exp(p_h_init, i))->intconst;
                }
                else {
                    init_val.f = hir_exp_ptr_check_const(syntax_init_mem_get_exp(p_h_init, i))->floatconst;
                }
                symbol_init_add(p_init, i, init_val);
            }
            p_symbol_var p_var = symbol_var_gen(syntax_decl_list_node_get_name(p_decl), p_type, syntax_decl_list_get_is_const(p_decl_list), false, p_init);
            syntax_func_add_constant(p_table, p_var);
        }
        else {
            p_symbol_var p_var = symbol_var_gen(syntax_decl_list_node_get_name(p_decl), p_type, syntax_decl_list_get_is_const(p_decl_list), false, NULL);
            syntax_func_add_variable(p_table, p_var);
            if (p_h_init) {
                if (list_head_alone(&p_var->p_type->array)) {
                    p_hir_exp p_lval = hir_exp_ptr_gen(p_var);
                    p_hir_exp p_rval = syntax_init_mem_get_exp(p_h_init, 0);
                    syntax_init_mem_clear_exp(p_h_init, 0);
                    p_hir_stmt p_assign = hir_stmt_assign_gen(p_lval, p_rval);
                    hir_block_add(p_block, p_assign);
                }
                else {
                    p_hir_exp p_lval = hir_exp_ptr_gen(p_var);
                    syntax_init_by_assign_gen(p_h_init, 0, p_lval, p_block);
                }
            }
        }
        syntax_init_mem_drop(p_h_init);
        syntax_decl_drop(p_decl);
    }
    syntax_decl_list_drop(p_decl_list);
    return p_block;
}
void syntax_global_vardecl(p_syntax_info p_info, p_syntax_decl_list p_decl_list) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, syntax_decl_list_get_list(p_decl_list)) {
        p_syntax_decl p_decl = syntax_decl_list_node_get_entry(p_node);
        list_del(p_node);

        p_symbol_type p_type = syntax_type_trans(syntax_decl_list_node_get_array(p_decl), syntax_decl_list_get_basic(p_decl_list));

        p_syntax_init_mem p_h_init = syntax_init_mem_gen(syntax_decl_list_node_get_init(p_decl), p_type);
        p_symbol_init p_init = symbol_init_gen(symbol_type_get_size(p_type), p_type->basic);
        if (p_h_init) {
            for (size_t i = 0; i < p_init->size; ++i) {
                symbol_init_val init_val;
                if (p_init->basic == type_int) {
                    init_val.i = hir_exp_ptr_check_const(syntax_init_mem_get_exp(p_h_init, i))->intconst;
                }
                else {
                    init_val.f = hir_exp_ptr_check_const(syntax_init_mem_get_exp(p_h_init, i))->floatconst;
                }
                symbol_init_add(p_init, i, init_val);
            }
        }
        else {
            for (size_t i = 0; i < p_init->size; ++i) {
                symbol_init_val init_val;
                if (p_init->basic == type_int) {
                    init_val.i = 0;
                }
                else {
                    init_val.f = 0;
                }
                symbol_init_add(p_init, i, init_val);
            }
        }
        syntax_init_mem_drop(p_h_init);
        p_symbol_var p_var = symbol_var_gen(syntax_decl_list_node_get_name(p_decl), p_type, syntax_decl_list_get_is_const(p_decl_list), true, p_init);
        if (syntax_decl_list_get_is_const(p_decl_list)) {
            syntax_program_add_constant(p_info, p_var);
        }
        else {
            syntax_program_add_variable(p_info, p_var);
        }

        syntax_decl_drop(p_decl);
    }
    syntax_decl_list_drop(p_decl_list);
}
