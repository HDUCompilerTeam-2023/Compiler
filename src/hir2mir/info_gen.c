#include <hir2mir.h>
#include <symbol/store.h>
#include <symbol/sym.h>
#include <symbol/type.h>

p_hir2mir_program_info hir2mir_program_info_gen(p_mir_program p_m_program) {
    size_t global_vmem_cnt = 0;
    p_mir_vmem *global_vmem_table = NULL;
    if (!list_head_alone(&p_m_program->p_store->variable)) {
        global_vmem_cnt = list_entry(p_m_program->p_store->variable.p_prev, symbol_sym, node)->id + 1;
        global_vmem_table = malloc(sizeof(p_mir_vmem) * global_vmem_cnt);
        for (size_t i = 0; i < global_vmem_cnt; ++i) {
            global_vmem_table[i] = NULL;
        }
    }
    p_hir2mir_program_info p_program_info = malloc(sizeof(*p_program_info));
    *p_program_info = (hir2mir_program_info) {
        .p_program = p_m_program,
        .func_table = p_m_program->func_table,
        .global_vmem_cnt = global_vmem_cnt,
        .global_vmem_table = global_vmem_table,
    };

    return p_program_info;
}

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_hir2mir_info hir2mir_info_gen(p_mir_func p_m_func, p_hir2mir_program_info p_program_info) {
    size_t local_addr_cnt = 0;
    p_mir_vreg *local_addr_table = NULL;
    if (!list_head_alone(&p_m_func->p_func_sym->variable)) {
        local_addr_cnt = list_entry(p_m_func->p_func_sym->variable.p_prev, symbol_sym, node)->id + 1;
        local_addr_table = malloc(sizeof(p_mir_vreg) * local_addr_cnt);
        for (size_t i = 0; i < local_addr_cnt; ++i) {
            local_addr_table[i] = NULL;
        }
    }
    size_t global_addr_cnt = 0;
    p_mir_vreg *global_addr_table = NULL;
    if (p_program_info->global_vmem_table) {
        global_addr_cnt = p_program_info->global_vmem_cnt;
        global_addr_table = malloc(sizeof(p_mir_vreg) * global_addr_cnt);
        for (size_t i = 0; i < global_addr_cnt; ++i) {
            global_addr_table[i] = NULL;
        }
    }
    size_t param_vmem_cnt = 0;
    p_mir_vmem *param_vmem_table = NULL;
    if (p_m_func->p_func_sym->last_param != &p_m_func->p_func_sym->variable) {
        param_vmem_cnt = list_entry(p_m_func->p_func_sym->last_param, symbol_sym, node)->id + 1;
        param_vmem_table = malloc(sizeof(p_mir_vmem) * param_vmem_cnt);
        p_list_head p_node;
        list_for_each(p_node, &p_m_func->p_func_sym->variable) {
            if (p_node->p_prev == p_m_func->p_func_sym->last_param) {
                break;
            }
            p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
            p_mir_vmem p_vmem = mir_vmem_sym_gen(p_sym);
            param_vmem_table[p_sym->id] = p_vmem;
            mir_func_vmem_add(p_m_func, p_vmem);
        }
    }

    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info) {
        .p_current_basic_block = NULL,
        .p_func = p_m_func,
        .p_ret_vmem = mir_vmem_temp_gen(p_m_func->p_func_sym->p_type->basic, 0),
        .p_ret_block = mir_basic_block_gen(),
        .param_vmem_table = param_vmem_table,
        .param_vmem_cnt = param_vmem_cnt,
        .local_addr_table = local_addr_table,
        .local_addr_cnt = local_addr_cnt,
        .global_addr_table = global_addr_table,
        .global_addr_cnt = global_addr_cnt,
        .p_program_info = p_program_info,
    };
    return p_info;
}

void hir2mir_program_info_drop(p_hir2mir_program_info p_program_info) {
    free(p_program_info->global_vmem_table);
    free(p_program_info);
}

void hir2mir_info_drop(p_hir2mir_info p_info) {
    free(p_info->param_vmem_table);
    free(p_info->local_addr_table);
    free(p_info->global_addr_table);
    free(p_info);
}

void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_new) {
    mir_func_add_basic_block(p_info->p_func, p_new);
    p_info->p_current_basic_block = p_new;
}

void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr) {
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_instr);
    p_mir_vreg p_des = mir_instr_get_des(p_instr);
    if (p_des) {
        mir_func_vreg_add(p_info->p_func, p_des);
    }
}
