#include <frontend/syntax/info/def.h>

#include <frontend/syntax/decl/gen.h>
#include <frontend/syntax/decl_head/gen.h>
#include <frontend/syntax/init/gen.h>
#include <frontend/syntax/symbol_table/gen.h>

#include <program/gen.h>

#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
#include <symbol_gen/var.h>

#include <ast2ir.h>

#include <ast_gen/block.h>
#include <ast_gen/exp.h>
#include <ast_gen/stmt.h>
#include <ast_gen/param.h>

p_program syntax_info_get_program(p_syntax_info p_info) {
    return p_info->p_program;
}

void syntax_set_block(p_syntax_info p_info, p_ast_block p_block) {
    p_info->p_block = p_block;
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
void syntax_program_add_function(p_syntax_info p_info, p_symbol_func p_func) {
    symbol_table_func_add(p_info->p_table, p_func);
    program_add_function(p_info->p_program, p_func);
}
void syntax_program_add_ext_function(p_syntax_info p_info, p_symbol_func p_func) {
    symbol_table_func_add(p_info->p_table, p_func);
    program_add_ext_function(p_info->p_program, p_func);
}
void syntax_func_head(p_syntax_info p_info, basic_type type, char *name) {
    p_symbol_func p_func = symbol_func_gen(name, type, false);
    syntax_program_add_function(p_info, p_func);
    p_info->p_func = p_func;
    free(name);

    syntax_zone_push(p_info);
}
void syntax_func_end(p_syntax_info p_info, p_ast_block p_block) {
    syntax_zone_pop(p_info);
    p_symbol_func p_func = p_info->p_func;
    p_info->p_func = NULL;
    switch (p_func->ret_type) {
    case type_void:
        ast_block_add(p_block, ast_stmt_return_gen(type_void, NULL));
        break;
    case type_i32:
        ast_block_add(p_block, ast_stmt_return_gen(type_i32, ast_exp_int_gen(0)));
        break;
    case type_f32:
        ast_block_add(p_block, ast_stmt_return_gen(type_f32, ast_exp_float_gen(0.0)));
        break;
    case type_str:
        break;
    }
    ast2ir_symbol_func_gen(p_block, p_func, p_info->p_program);
    symbol_func_set_varible_id(p_func);
    ast_block_drop(p_block);
}

static inline p_symbol_func syntax_rtlib_decl(p_syntax_info p_info, basic_type type, char *name, p_symbol_type p_param1, p_symbol_type p_param2, p_symbol_type p_param3, bool is_va) {
    p_symbol_func p_func = symbol_func_gen(name, type, is_va);
    syntax_program_add_ext_function(p_info, p_func); // true
    p_info->p_func = p_func;

    syntax_zone_push(p_info);
    if (p_param1) {
        p_symbol_var p_var = symbol_var_gen("arg1", p_param1, false, NULL);
        syntax_func_add_param(p_info, p_var);
        if (p_param2) {
            p_var = symbol_var_gen("arg2", p_param2, false, NULL);
            syntax_func_add_param(p_info, p_var);
            if (p_param3) {
                p_var = symbol_var_gen("arg3", p_param3, false, NULL);
                syntax_func_add_param(p_info, p_var);
            }
        }
    }
    syntax_zone_pop(p_info);
    p_info->p_func = NULL;
    return p_func;
}
void syntax_rtlib_func_init(p_syntax_info p_info) {
    syntax_rtlib_decl(p_info, type_i32, "getint", NULL, NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_i32, "getch", NULL, NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_f32, "getfloat", NULL, NULL, NULL, false);

    p_symbol_type p_type = symbol_type_var_gen(type_i32);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_i32, "getarray", p_type, NULL, NULL, false);
    p_type = symbol_type_var_gen(type_f32);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_i32, "getfarray", p_type, NULL, NULL, false);

    syntax_rtlib_decl(p_info, type_void, "putint", symbol_type_var_gen(type_i32), NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_void, "putch", symbol_type_var_gen(type_i32), NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_void, "putfloat", symbol_type_var_gen(type_f32), NULL, NULL, false);

    p_type = symbol_type_var_gen(type_i32);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_void, "putarray", symbol_type_var_gen(type_i32), p_type, NULL, false);
    p_type = symbol_type_var_gen(type_f32);
    symbol_type_push_ptr(p_type);
    syntax_rtlib_decl(p_info, type_void, "putfarray", symbol_type_var_gen(type_i32), p_type, NULL, false);

    syntax_rtlib_decl(p_info, type_void, "putf", symbol_type_var_gen(type_str), NULL, NULL, true);

    syntax_rtlib_decl(p_info, type_void, "_sysy_starttime", symbol_type_var_gen(type_i32), NULL, NULL, false);
    syntax_rtlib_decl(p_info, type_void, "_sysy_stoptime", symbol_type_var_gen(type_i32), NULL, NULL, false);

    p_type = symbol_type_var_gen(type_void);
    symbol_type_push_ptr(p_type);
    p_info->p_mem_set = syntax_rtlib_decl(p_info, type_void, "memset", p_type, symbol_type_var_gen(type_i32), symbol_type_var_gen(type_i32), false);
}

static inline size_t syntax_init_by_assign_gen(p_symbol_type p_type, p_syntax_init p_init, size_t index, p_ast_exp p_addr, p_ast_block p_block) {
    if (list_head_alone(&p_addr->p_type->array)) {
        assert(p_addr->p_type->ref_level == 1);
        p_ast_exp p_rval = syntax_init_find_exp(p_init, p_type, index++);
        if (!p_rval) p_rval = ast_exp_int_gen(0);
        p_ast_stmt p_assign = ast_stmt_assign_gen(p_addr, p_rval);
        ast_block_add(p_block, p_assign);
        return index;
    }
    p_ast_exp p_element_addr = ast_exp_gep_gen(p_addr, ast_exp_int_gen(0), true);
    size_t length = symbol_type_get_size(p_addr->p_type) / symbol_type_get_size(p_element_addr->p_type);
    for (size_t i = 1; i < length; ++i) {
        index = syntax_init_by_assign_gen(p_type, p_init, index, p_element_addr, p_block);
        p_element_addr = ast_exp_gep_gen(ast_exp_use_gen(p_element_addr), ast_exp_int_gen(1), false);
    }
    return syntax_init_by_assign_gen(p_type, p_init, index, p_element_addr, p_block);
}
static inline bool syntax_init_by_assign_gen_no_zero(p_symbol_type p_type, p_syntax_init p_init, p_ast_exp p_addr, p_ast_block p_block) {
    if (list_head_alone(&p_addr->p_type->array)) {
        assert(p_addr->p_type->ref_level == 1);
        p_ast_exp p_rval = syntax_init_get_exp(p_init);
        assert(p_rval);
        if (p_rval->kind == ast_exp_num && p_rval->i32const == 0) {
            ast_exp_drop(p_rval);
            return false;
        }
        p_ast_stmt p_assign = ast_stmt_assign_gen(p_addr, p_rval);
        ast_block_add(p_block, p_assign);
        return true;
    }
    p_symbol_type_array p_arr = symbol_type_pop_array(p_type);
    p_ast_exp p_element_addr = ast_exp_gep_gen(p_addr, ast_exp_int_gen(0), true);
    p_ast_exp p_first_addr = p_element_addr;

    bool have_assign = false;
    bool last_addr_hav_assign = false;

    p_list_head p_node, p_head = syntax_init_get_head(p_init);
    list_for_each(p_node, p_head) {
        p_syntax_init p_sub_init = syntax_init_get_entry(p_node);
        last_addr_hav_assign = syntax_init_by_assign_gen_no_zero(p_type, p_sub_init, p_element_addr, p_block);
        have_assign |= last_addr_hav_assign;
        if (p_node->p_next == p_head)
            continue;
        if (last_addr_hav_assign)
            p_element_addr = ast_exp_gep_gen(ast_exp_use_gen(p_element_addr), ast_exp_int_gen(1), false);
        else
            p_element_addr = ast_exp_gep_gen(p_element_addr, ast_exp_int_gen(1), false);
    }
    if (!last_addr_hav_assign) {
        if (!have_assign) {
            p_first_addr->p_addr = ast_exp_use_gen(p_addr);
        }
        ast_exp_drop(p_element_addr);
    }

    symbol_type_push_array(p_type, p_arr);
    return have_assign;
}
p_syntax_decl_head syntax_declaration(p_syntax_info p_info, p_syntax_decl_head p_head, p_syntax_decl p_decl) {
    p_symbol_type p_type = syntax_type_trans(syntax_decl_list_node_get_array(p_decl), syntax_decl_head_get_basic(p_head));
    p_syntax_init p_s_init = syntax_init_regular(syntax_decl_list_node_get_init(p_decl), p_type);
    bool is_const = syntax_decl_head_get_is_const(p_head);
    const char *name = syntax_decl_list_node_get_name(p_decl);

    if (!p_info->p_func) {
        p_symbol_var p_var;
        if (p_s_init) {
            p_symbol_init p_init = symbol_init_gen(symbol_type_get_size(p_type), p_type->basic);
            for (size_t i = 0; i < p_init->size; ++i) {
                symbol_init_val init_val;
                p_ast_exp p_rval = syntax_init_find_exp(p_s_init, p_type, i);
                if (!p_rval) p_rval = ast_exp_int_gen(0);
                else
                    p_rval = ast_exp_ptr_check_const(p_rval);
                if (p_init->basic == type_i32) {
                    init_val.i = p_rval->i32const;
                }
                else {
                    init_val.f = p_rval->f32const;
                }
                ast_exp_drop(p_rval);
                symbol_init_add(p_init, i, init_val);
            }
            p_var = symbol_var_gen(name, p_type, is_const, p_init);
        }
        else {
            assert(!is_const);
            p_var = symbol_var_gen(name, p_type, is_const, NULL);
        }
        syntax_program_add_variable(p_info, p_var);
    }
    else if (!p_info->p_block) {
        assert(!p_s_init);
        p_symbol_var p_var = symbol_var_gen(name, p_type, false, NULL);
        syntax_func_add_param(p_info, p_var);
    }
    else if (is_const) {
        p_symbol_init p_init = symbol_init_gen(p_type->size, syntax_decl_head_get_basic(p_head));
        for (size_t i = 0; i < p_type->size; ++i) {
            symbol_init_val init_val;
            p_ast_exp p_rval = syntax_init_find_exp(p_s_init, p_type, i);
            if (!p_rval) p_rval = ast_exp_int_gen(0);
            else
                p_rval = ast_exp_ptr_check_const(p_rval);
            if (p_init->basic == type_i32) {
                init_val.i = p_rval->i32const;
            }
            else {
                init_val.f = p_rval->f32const;
            }
            ast_exp_drop(p_rval);
            symbol_init_add(p_init, i, init_val);
        }
        p_symbol_var p_var = symbol_var_gen(name, p_type, true, p_init);
        syntax_func_add_variable(p_info, p_var);
    }
    else {
        p_symbol_var p_var = symbol_var_gen(name, p_type, false, NULL);
        syntax_func_add_variable(p_info, p_var);
        if (list_head_alone(&p_var->p_type->array)) {
            p_ast_exp p_lval = ast_exp_ptr_gen(p_var);
            p_ast_exp p_rval = syntax_init_find_exp(p_s_init, p_type, 0);
            if (!p_rval) p_rval = ast_exp_int_gen(0);
            p_ast_stmt p_assign = ast_stmt_assign_gen(p_lval, p_rval);
            ast_block_add(p_info->p_block, p_assign);
        }
        else if (p_s_init) {
            if (symbol_type_get_size(p_var->p_type) > 30) {
                p_ast_param_list p_pl = ast_param_list_init();
                ast_param_list_add(p_pl, ast_exp_ptr_gen(p_var));
                ast_param_list_add(p_pl, ast_exp_int_gen(0));
                ast_param_list_add(p_pl, ast_exp_int_gen(symbol_type_get_size(p_var->p_type) * basic_type_get_size(p_var->p_type->basic)));
                p_ast_exp p_call_mem_set = ast_exp_call_gen(p_info->p_mem_set, p_pl);
                p_ast_stmt p_stmt = ast_stmt_exp_gen(p_call_mem_set);
                ast_block_add(p_info->p_block, p_stmt);
                p_ast_exp p_lval = ast_exp_ptr_gen(p_var);
                bool have_assign = syntax_init_by_assign_gen_no_zero(p_type, p_s_init, p_lval, p_info->p_block);
                if (!have_assign)
                    ast_exp_drop(p_lval);
            }
            else {
                p_ast_exp p_lval = ast_exp_ptr_gen(p_var);
                syntax_init_by_assign_gen(p_type, p_s_init, 0, p_lval, p_info->p_block);
            }
        }
    }
    syntax_init_drop(p_s_init);
    syntax_decl_drop(p_decl);

    return p_head;
}
