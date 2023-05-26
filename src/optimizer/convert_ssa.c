#include <mir_manager.h>
#include <optimizer/convert_ssa.h>
#include <program/def.h>
#include <symbol/var.h>
#include <symbol/func.h>
#include <symbol/type.h>
void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_mir_basic_block p_basic_block, size_t current_num) {
    dfs_seq[current_num] = (convert_ssa) {
        .dom_frontier = bitmap_gen(block_num),
        .p_def_var = bitmap_gen(var_num),
        .p_phi_var = bitmap_gen(var_num),
        .p_basic_block = p_basic_block,
        .if_in = false,
    };
    p_basic_block->dfn_id = current_num;
    bitmap_set_empty(dfs_seq[current_num].dom_frontier);
    bitmap_set_empty(dfs_seq[current_num].p_def_var);
    bitmap_set_empty(dfs_seq[current_num].p_phi_var);
}

size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_mir_basic_block p_entry, size_t current_num) {
    if (p_entry->if_visited) return current_num;
    p_entry->if_visited = true;
    convert_ssa_gen(dfs_seq, block_num, var_num, p_entry, current_num);
    current_num++;

    p_mir_basic_block_branch_target p_true_target = p_entry->p_branch->p_target_1;
    p_mir_basic_block_branch_target p_false_target = p_entry->p_branch->p_target_2;

    if (p_true_target)
        current_num = convert_ssa_init_dfs_sequence(dfs_seq, block_num, var_num, p_true_target->p_block, current_num);
    if (p_false_target)
        current_num = convert_ssa_init_dfs_sequence(dfs_seq, block_num, var_num, p_false_target->p_block, current_num);
    return current_num;
}

p_ssa_var_list_info convert_ssa_init_var_list(p_mir_func p_func) {
    p_ssa_var_list_info p_var_list = malloc(sizeof(*p_var_list));
    *p_var_list = (ssa_var_list_info) {
        .vmem_num = p_func->vmem_cnt,
        .p_func = p_func,
    };

    p_var_list->p_base = malloc(p_var_list->vmem_num * sizeof(*p_var_list->p_base));
    p_list_head p_node;
    // 全局变量, 已初始化
    list_for_each(p_node, &p_func->vmem_list) {
        p_mir_vmem p_vmem = list_entry(p_node, mir_vmem, node);
        *(p_var_list->p_base + p_vmem->id) = (ssa_var_info) {
            .p_vmem = p_vmem,
            .p_current_vreg = NULL,
            .sym_stack = list_head_init(&(p_var_list->p_base + p_vmem->id)->sym_stack),
        };
    }

    return p_var_list;
}

// 将变量转换到对应的标号，若不存在标号返回 -1
static inline size_t get_var_index(p_mir_operand p_operand, p_ssa_var_list_info p_var_list) {
    if (!p_operand) return -1;
    if (p_operand->kind == reg) return -1;
    if (p_operand->ref_level == 0) return -1;

    p_mir_vmem p_vmem = p_operand->p_global_vmem;

    if (p_vmem->is_array) return -1;
    if (p_vmem->p_var && p_vmem->p_var->is_global) return -1;
    return p_vmem->id;
}

void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num) {
    for (size_t i = block_num - 1; i < block_num; i--) {
        p_convert_ssa p_info = dfs_seq + i;

        p_mir_basic_block_branch_target p_true_block = p_info->p_basic_block->p_branch->p_target_1;
        p_mir_basic_block_branch_target p_false_block = p_info->p_basic_block->p_branch->p_target_2;

        // 将直接后继做为 DF_up 的候选
        if (p_true_block)
            bitmap_add_element(p_info->dom_frontier, p_true_block->p_block->dfn_id);
        if (p_false_block)
            bitmap_add_element(p_info->dom_frontier, p_false_block->p_block->dfn_id);

        p_list_head p_node;
        // 记录 直接支配点
        p_bitmap p_son_list = bitmap_gen(block_num);
        bitmap_set_empty(p_son_list);
        // 支配边界不包括自身
        bitmap_add_element(p_son_list, p_info->p_basic_block->dfn_id);
        // 将支配树上的直接儿子的支配边界作为候选
        list_for_each(p_node, &p_info->p_basic_block->dom_son_list) {
            size_t son_id = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block->dfn_id;
            p_convert_ssa p_son_info = dfs_seq + son_id;
            bitmap_add_element(p_son_list, p_son_info->p_basic_block->dfn_id);
            bitmap_merge_not_new(p_info->dom_frontier, p_son_info->dom_frontier);
        }
        // 所有候选中不受当前节点直接支配的节点为支配边界
        bitmap_neg_not_new(p_son_list);
        bitmap_and_not_new(p_info->dom_frontier, p_son_list);
        bitmap_drop(p_son_list);
    }
}

void convert_ssa_insert_phi(p_convert_ssa dfs_seq, size_t block_num, p_ssa_var_list_info p_var_list) {
    // 入口块对所有变量已经定值
    bitmap_set_full(dfs_seq->p_def_var);
    size_t work_num = block_num + 1;
    // 记录原来的集合
    p_bitmap p_old = bitmap_gen(p_var_list->vmem_num);
    // 工作队列
    size_t *p_work_list = malloc(work_num * sizeof(*p_work_list));
    size_t work_tail = 0;
    // 第一遍得到块的指令定值集合，同时得到部分 phi 集合
    for (size_t i = 0; i < block_num; i++) {
        p_convert_ssa p_info = dfs_seq + i;
        // 将 phi 集合加入到定值集合
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        p_list_head p_node;
        list_for_each(p_node, &p_info->p_basic_block->instr_list) {
            p_mir_instr p_instr = list_entry(p_node, mir_instr, node);
            p_mir_operand p_operand = mir_instr_get_store_addr(p_instr);
            //  将已经声明的变量和ret变量加入到该块定值集合
            size_t id = get_var_index(p_operand, p_var_list);
            if (id != -1)
                bitmap_add_element(p_info->p_def_var, id);
        }
        // 遍历支配边界将定值集合并入到 边界的 phi 集合
        for (size_t j = 0; j < block_num; j++) {
            if (bitmap_if_in(p_info->dom_frontier, j)) { // 若 phi 集合发生变化且已经遍历过且没有被加入过工作集合需要加入到工作集合之后处理
                bitmap_copy_not_new(p_old, (dfs_seq + j)->p_phi_var);
                bitmap_merge_not_new((dfs_seq + j)->p_phi_var, p_info->p_def_var);
                bool if_change = bitmap_if_equal(p_old, (dfs_seq + j)->p_phi_var);
                if (if_change && j <= i && !(dfs_seq + j)->if_in) {
                    p_work_list[work_tail++] = j;
                    (dfs_seq + j)->if_in = true;
                }
            }
        }
    }

    // 处理之前 phi 集合未完全处理完的块
    for (size_t work_head = 0; work_head != work_tail % work_num; work_head = (work_head + 1) % work_num) {
        p_convert_ssa p_info = dfs_seq + p_work_list[work_head];
        p_info->if_in = false;
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        for (size_t j = 0; j < block_num; j++) {
            // 当新的 phi 集合发生变化 并且没有被加入到工作集时需要加入工作集合
            if (bitmap_if_in(p_info->dom_frontier, j)) {
                bitmap_copy_not_new(p_old, (dfs_seq + j)->p_phi_var);
                bitmap_merge_not_new((dfs_seq + j)->p_phi_var, p_info->p_def_var);
                bool if_change = bitmap_if_equal(p_old, (dfs_seq + j)->p_phi_var);
                if (if_change && !(dfs_seq + j)->if_in) {
                    p_work_list[work_tail] = j;
                    work_tail = (work_tail + 1) % work_num;
                    (dfs_seq + j)->if_in = true;
                }
            }
        }
    }
    bitmap_drop(p_old);
    free(p_work_list);
}

static inline p_mir_operand get_top_operand(p_ssa_var_list_info p_var_list, size_t index) {
    p_ssa_var_info p_info = p_var_list->p_base + index;
    if (!p_info->p_current_vreg) return NULL;
    return mir_operand_vreg_gen(p_info->p_current_vreg);
}

static inline void set_branch_target_ssa_id(p_mir_basic_block_branch_target p_branch_target, p_convert_ssa dfs_seq, p_ssa_var_list_info p_var_list) {
    size_t var_num = p_var_list->vmem_num;
    p_bitmap p_phi_var = (dfs_seq + p_branch_target->p_block->dfn_id)->p_phi_var;
    for (size_t i = 0; i < var_num; i++) {
        if (bitmap_if_in(p_phi_var, i)) {
            p_mir_operand p_param = get_top_operand(p_var_list, i);
            mir_basic_block_branch_target_add_param(p_branch_target, p_param);
        }
    }
}

// 为基本块创建 phi 参数列表并命名
static inline void set_block_param_ssa_id(p_mir_basic_block p_basic_block, p_convert_ssa dfs_seq, p_ssa_var_list_info p_var_list) {
    size_t var_num = p_var_list->vmem_num;
    p_bitmap p_phi_var = (dfs_seq + p_basic_block->dfn_id)->p_phi_var;
    for (size_t i = 0; i < var_num; i++) {
        if (bitmap_if_in(p_phi_var, i)) {
            p_mir_vreg p_vreg = mir_vreg_gen((p_var_list->p_base + i)->p_vmem->b_type, (p_var_list->p_base + i)->p_vmem->ref_level);
            mir_func_vreg_add_at(p_var_list->p_func, p_vreg, p_basic_block, list_entry(&p_basic_block->instr_list, mir_instr, node));
            (p_var_list->p_base + i)->p_current_vreg = p_vreg;
            mir_basic_block_add_param(p_basic_block, p_vreg);
        }
    }
}

static inline void record_current_sym(p_ssa_var_list_info p_var_list) {
    size_t vmem_num = p_var_list->vmem_num;
    for (size_t i = 0; i < vmem_num; i++) {
        p_ssa_var_info p_info = p_var_list->p_base + i;
        p_sym_stack_node p_node = malloc(sizeof(*p_node));
        *p_node = (sym_stack_node) {
            .node = list_head_init(&p_node->node),
            .p_vreg = p_info->p_current_vreg,
        };
        list_add_prev(&p_node->node, &p_info->sym_stack);
    }
}

static inline void restore_current_sym(p_ssa_var_list_info p_var_list) {
    size_t vmem_num = p_var_list->vmem_num;
    for (size_t i = 0; i < vmem_num; i++) {
        p_ssa_var_info p_info = p_var_list->p_base + i;
        p_sym_stack_node p_node = list_entry(p_info->sym_stack.p_prev, sym_stack_node, node);
        p_info->p_current_vreg = p_node->p_vreg;
        list_del(&p_node->node);
        free(p_node);
    }
}
void convert_ssa_rename_var(p_ssa_var_list_info p_var_list, p_convert_ssa dfs_seq, p_mir_basic_block p_entry) {
    if (p_entry->if_visited) return;
    p_entry->if_visited = true;
    // 记录入块信息
    record_current_sym(p_var_list);
    set_block_param_ssa_id(p_entry, dfs_seq, p_var_list);

    p_list_head p_node;
    list_for_each(p_node, &p_entry->instr_list) {
        p_mir_instr p_instr = list_entry(p_node, mir_instr, node);

        if (p_instr->irkind == mir_load) {
            size_t var_index = get_var_index(p_instr->mir_load.p_addr, p_var_list);
            if (var_index == -1) continue;
            assert(!p_instr->mir_store.p_offset);

            p_mir_operand p_top_operand = get_top_operand(p_var_list, var_index);
            if (!p_top_operand) {
                p_var_list->p_base[var_index].p_current_vreg = p_instr->mir_load.p_des;
                continue;
            }

            mir_operand_drop(p_instr->mir_load.p_addr);
            p_instr->irkind = mir_val_assign;
            p_instr->mir_unary.p_des = p_instr->mir_load.p_des;
            p_instr->mir_unary.p_src = p_top_operand;
            continue;
        }
        if (p_instr->irkind == mir_store) {
            size_t var_index = get_var_index(p_instr->mir_store.p_addr, p_var_list);
            if (var_index == -1) continue;
            assert(!p_instr->mir_store.p_offset);

            p_mir_vreg p_vreg = mir_vreg_gen(p_var_list->p_base[var_index].p_vmem->b_type, p_var_list->p_base[var_index].p_vmem->ref_level);
            mir_vreg_set_instr_def(p_vreg, p_instr);
            mir_func_vreg_add_at(p_var_list->p_func, p_vreg, p_entry, p_instr);
            p_var_list->p_base[var_index].p_current_vreg = p_vreg;

            mir_operand_drop(p_instr->mir_store.p_addr);
            p_instr->irkind = mir_val_assign;
            p_instr->mir_unary.p_src = p_instr->mir_store.p_src;
            p_instr->mir_unary.p_des = p_vreg;
            continue;
        }
    }
    p_mir_basic_block_branch p_branch = p_entry->p_branch;
    if (p_branch->kind == mir_br_branch) {
        set_branch_target_ssa_id(p_branch->p_target_1, dfs_seq, p_var_list);
        convert_ssa_rename_var(p_var_list, dfs_seq, p_branch->p_target_1->p_block);
    }
    if (p_branch->kind == mir_cond_branch) {
        set_branch_target_ssa_id(p_branch->p_target_1, dfs_seq, p_var_list);
        convert_ssa_rename_var(p_var_list, dfs_seq, p_branch->p_target_1->p_block);
        set_branch_target_ssa_id(p_branch->p_target_2, dfs_seq, p_var_list);
        convert_ssa_rename_var(p_var_list, dfs_seq, p_branch->p_target_2->p_block);
    }
    // 恢复信息
    restore_current_sym(p_var_list);
}
#include <stdio.h>
static inline void print_dom_frontier(convert_ssa *dfs_seq, size_t block_num) {
    printf(" --- dom_frontier start---\n");
    for (size_t i = 0; i < block_num; i++) {
        p_convert_ssa p_info = dfs_seq + i;
        printf("b%ld (dfn_id: %ld): ", p_info->p_basic_block->block_id, p_info->p_basic_block->dfn_id);
        bitmap_print(p_info->dom_frontier);
        printf("\n");
    }
    printf(" --- dom_frontier end---\n");
}

void convert_ssa_func(p_mir_func p_func) {
    if (list_head_alone(&p_func->block)) return;
    size_t block_num = p_func->block_cnt;
    p_convert_ssa dfs_seq = malloc(block_num * sizeof(*dfs_seq));
    // 初始化变量集合
    p_ssa_var_list_info p_var_list = convert_ssa_init_var_list(p_func);
    // 初始化 dfs 序
    mir_basic_block_init_visited(p_func);
    p_mir_basic_block p_entry = list_entry(p_func->block.p_next, mir_basic_block, node);
    convert_ssa_init_dfs_sequence(dfs_seq, block_num, p_var_list->vmem_num, p_entry, 0);
    // 计算支配树
    mir_cfg_set_func_dom(p_func);
    // 计算支配边界
    convert_ssa_compute_dom_frontier(dfs_seq, block_num);
    print_dom_frontier(dfs_seq, block_num);
    // 插入 phi 函数
    convert_ssa_insert_phi(dfs_seq, block_num, p_var_list);
    // // 重命名
    mir_basic_block_init_visited(p_func);
    convert_ssa_rename_var(p_var_list, dfs_seq, p_entry);

    convert_ssa_dfs_seq_drop(dfs_seq, block_num);
    ssa_var_list_info_drop(p_var_list);

    mir_func_set_vreg_id(p_func);
}

void convert_ssa_program(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_mir_func p_func = list_entry(p_node, symbol_func, node)->p_m_func;
        convert_ssa_func(p_func);
    }
}

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num) {
    for (size_t i = 0; i < block_num; i++) {
        bitmap_drop((dfs_seq + i)->dom_frontier);
        bitmap_drop((dfs_seq + i)->p_def_var);
        bitmap_drop((dfs_seq + i)->p_phi_var);
    }
    free(dfs_seq);
}

void ssa_var_list_info_drop(p_ssa_var_list_info p_var_list) {
    free(p_var_list->p_base);
    free(p_var_list);
}
