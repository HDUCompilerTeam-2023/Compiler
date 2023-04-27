#include <hir2mir/info_gen.h>
#include <symbol/sym.h>
#include <symbol/type.h>

// 根据函数信息 生成接下来需要收集的信息的返回值类型
p_hir2mir_info hir2mir_info_gen(p_mir_func p_m_func, p_mir_func m_func_table) {
    p_hir2mir_info p_info = malloc(sizeof(*p_info));
    *p_info = (hir2mir_info) {
        .p_current_basic_block = NULL,
        .p_func = p_m_func,
        .func_table = m_func_table,
        .p_ret_block = mir_basic_block_gen(),
    };
    p_mir_temp_sym p_temp_sym = mir_temp_sym_basic_gen(p_m_func->p_func_sym->p_type->basic, p_m_func);
    p_info->p_ret_operand = mir_operand_temp_sym_gen(p_temp_sym);

    p_mir_instr p_ret = mir_ret_instr_gen(p_info->p_ret_operand);
    mir_basic_block_addinstr(p_info->p_ret_block, p_ret);
    return p_info;
}

void hir2mir_info_drop(p_hir2mir_info p_info) {
    free(p_info);
}

void hir2mir_info_add_basic_block(p_hir2mir_info p_info, p_mir_basic_block p_new) {
    mir_func_add_basic_block(p_info->p_func, p_new);
    p_info->p_current_basic_block = p_new;
}

// 在 prev 块添加 跳转， 若 prev 块为空， 将 prev 的所有前驱的后继块置位 next 否则，为 p_prev 创建跳转， 返回后继的那个空块
p_mir_instr hir2mir_info_add_br_instr(p_hir2mir_info p_info, p_mir_basic_block p_next) {
    if (list_head_alone(&p_info->p_current_basic_block->instr_list)) {
        p_list_head p_node;
        list_for_each(p_node, &p_info->p_current_basic_block->prev_basic_block_list) {
            p_mir_basic_block p_basic_block = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block;
            p_mir_instr p_last_instr = list_entry(p_basic_block->instr_list.p_prev, mir_instr, node);
            if (p_last_instr->irkind == mir_br) {
                p_last_instr->mir_br.p_target = p_next;
            }
            else if (p_last_instr->irkind == mir_condbr) {
                if (p_last_instr->mir_condbr.p_target_true == p_info->p_current_basic_block)
                    p_last_instr->mir_condbr.p_target_true = p_next;
                if (p_last_instr->mir_condbr.p_target_false == p_info->p_current_basic_block)
                    p_last_instr->mir_condbr.p_target_false = p_next;
            }
            mir_basic_block_add_prev(p_basic_block, p_next);
        }
        list_del(&p_info->p_current_basic_block->node);
        mir_basic_block_drop(p_info->p_current_basic_block);
        return NULL;
    }
    else {
        p_mir_instr p_br_instr = mir_br_instr_gen(p_info->p_current_basic_block, p_next);
        mir_basic_block_addinstr(p_info->p_current_basic_block, p_br_instr);
        return p_br_instr;
    }
}

void hir2mir_info_add_instr(p_hir2mir_info p_info, p_mir_instr p_instr) {
    mir_basic_block_addinstr(p_info->p_current_basic_block, p_instr);
}