#ifndef __FRONTEND_SYNTAX_GEN__
#define __FRONTEND_SYNTAX_GEN__

#include <frontend/syntax.h>
#include <frontend/symbol_table.h>

struct syntax_init {
    bool is_exp;
    bool syntax_const;
    union {
        p_hir_exp p_exp;
        list_head list;
    };

    list_head node;
};
p_syntax_init syntax_init_list_gen(void);
p_syntax_init syntax_init_exp_gen(p_hir_exp p_exp);
p_syntax_init syntax_init_list_add(p_syntax_init p_list, p_syntax_init p_init);

p_symbol_func syntax_func_head(p_symbol_table p_table, basic_type type, char *name, p_syntax_param_list p_param_list);
p_symbol_func syntax_func_end(p_symbol_table p_table, p_symbol_func p_func, p_hir_block p_block);

struct syntax_type_array {
    uint64_t size;
    p_syntax_type_array p_prev;
};
p_syntax_type_array syntax_type_array_gen(uint64_t size);

struct syntax_decl {
    char *name;
    p_syntax_type_array p_array;
    p_syntax_init p_init;

    list_head node;
};
p_syntax_decl syntax_decl_gen(char *name);
p_syntax_decl syntax_decl_arr(p_syntax_decl p_decl, p_hir_exp p_exp);
p_syntax_decl syntax_decl_init(p_syntax_decl p_decl, p_syntax_init p_init);

p_symbol_type syntax_type_trans(p_syntax_type_array p_array, basic_type b_type);

struct syntax_decl_list {
    list_head decl;
    bool is_const;
    basic_type type;
};
p_syntax_decl_list syntax_decl_list_gen(void);
p_syntax_decl_list syntax_decl_list_add(p_syntax_decl_list p_decl_list, p_syntax_decl p_decl);
p_syntax_decl_list syntax_decl_list_set(p_syntax_decl_list p_decl_list, bool is_const, basic_type type);

p_hir_exp syntax_val_offset(p_hir_exp p_val, p_hir_exp p_offset);

struct syntax_param_decl {
    char *name;
    p_symbol_type p_type;
    list_head node;
};
p_syntax_param_decl syntax_param_decl_gen(basic_type type, p_syntax_decl p_decl);
struct syntax_param_list {
    list_head param_decl;
};
p_syntax_param_list syntax_param_list_gen(void);
p_syntax_param_list syntax_param_list_add(p_syntax_param_list p_list, p_syntax_param_decl p_decl);

p_hir_block syntax_local_vardecl(p_symbol_table p_table, p_hir_block p_block, p_syntax_decl_list p_decl_list);
void syntax_global_vardecl(p_symbol_table p_table, p_syntax_decl_list p_decl_list);

p_hir_exp syntax_const_check(p_hir_exp p_exp);

void syntax_rtlib_decl(p_symbol_table p_table, basic_type type, char *name, p_symbol_type p_param1, p_symbol_type p_param2, bool is_va);

#include <hir_gen.h>
#include <symbol_gen.h>
#include <program/gen.h>

#endif
