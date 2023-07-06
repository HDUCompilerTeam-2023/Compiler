#include <ir_manager.h>
#include <ir_opt/mem2reg.h>
#include <program/def.h>
#include <symbol/type.h>
#include <symbol/var.h>
#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
p_convert_ssa_list mem2reg_info_gen(p_symbol_func p_func) {
    p_convert_ssa_list p_convert_list = malloc(sizeof(*p_convert_list));
    p_convert_list->p_func = p_func;
    p_convert_list->p_base = malloc(sizeof(*p_convert_list->p_base) * p_func->block_cnt);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_convert_list->p_base[p_basic_block->block_id] = (convert_ssa) {
            .dom_frontier = bitmap_gen(p_func->block_cnt),
            .p_def_var = bitmap_gen(p_func->var_cnt),
            .p_phi_var = bitmap_gen(p_func->var_cnt),
            .if_in = false,
        };
        bitmap_set_empty((p_convert_list->p_base + p_basic_block->block_id)->dom_frontier);
        bitmap_set_empty((p_convert_list->p_base + p_basic_block->block_id)->p_def_var);
        bitmap_set_empty((p_convert_list->p_base + p_basic_block->block_id)->p_phi_var);
    }
    return p_convert_list;
}

p_ssa_var_list_info mem2reg_init_var_list(p_symbol_func p_func) {
    p_ssa_var_list_info p_var_list = malloc(sizeof(*p_var_list));
    *p_var_list = (ssa_var_list_info) {
        .vmem_num = p_func->var_cnt,
        .p_func = p_func,
    };

    p_var_list->p_base = malloc(p_var_list->vmem_num * sizeof(*p_var_list->p_base));
    p_list_head p_node;
    // 全局变量, 已初始化
    list_for_each(p_node, &p_func->constant) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        *(p_var_list->p_base + p_vmem->id) = (ssa_var_info) {
            .p_vmem = p_vmem,
            .p_current_vreg = NULL,
            .sym_stack = list_init_head(&(p_var_list->p_base + p_vmem->id)->sym_stack),
        };
    }
    list_for_each(p_node, &p_func->param) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        *(p_var_list->p_base + p_vmem->id) = (ssa_var_info) {
            .p_vmem = p_vmem,
            .p_current_vreg = NULL,
            .sym_stack = list_init_head(&(p_var_list->p_base + p_vmem->id)->sym_stack),
        };
    }
    list_for_each(p_node, &p_func->variable) {
        p_symbol_var p_vmem = list_entry(p_node, symbol_var, node);
        *(p_var_list->p_base + p_vmem->id) = (ssa_var_info) {
            .p_vmem = p_vmem,
            .p_current_vreg = NULL,
            .sym_stack = list_init_head(&(p_var_list->p_base + p_vmem->id)->sym_stack),
        };
    }

    return p_var_list;
}

// 将变量转换到对应的标号，若不存在标号返回 -1
static inline size_t get_var_index(p_ir_operand p_operand, p_ssa_var_list_info p_var_list) {
    if (!p_operand) return -1;
    if (p_operand->kind == reg) return -1;
    if (p_operand->p_type->ref_level == 0) return -1;

    p_symbol_var p_vmem = p_operand->p_vmem;

    if (!list_head_alone(&p_vmem->p_type->array) && p_vmem->p_type->ref_level == 0) return -1;
    if (p_vmem->is_global) return -1;
    if (p_vmem->is_const) return -1;
    return p_vmem->id;
}

void mem2reg_compute_dom_frontier(p_convert_ssa_list p_convert_list) {
    p_list_head p_node;
    list_for_each_tail(p_node, &p_convert_list->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_convert_ssa p_info = p_convert_list->p_base + p_basic_block->block_id;
        p_ir_basic_block_branch_target p_true_block = p_basic_block->p_branch->p_target_1;
        p_ir_basic_block_branch_target p_false_block = p_basic_block->p_branch->p_target_2;

        // 将直接后继做为 DF_up 的候选
        if (p_true_block)
            bitmap_add_element(p_info->dom_frontier, p_true_block->p_block->block_id);
        if (p_false_block)
            bitmap_add_element(p_info->dom_frontier, p_false_block->p_block->block_id);

        p_list_head p_node;
        // 记录 直接支配点
        p_bitmap p_son_list = bitmap_gen(p_convert_list->p_func->block_cnt);
        bitmap_set_empty(p_son_list);
        // 将支配树上的直接儿子的支配边界作为候选
        list_for_each(p_node, &p_basic_block->dom_son_list) {
            p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            p_convert_ssa p_son_info = p_convert_list->p_base + p_son->block_id;
            bitmap_add_element(p_son_list, p_son->block_id);
            bitmap_merge_not_new(p_info->dom_frontier, p_son_info->dom_frontier);
        }
        // 所有候选中不受当前节点直接支配的节点为支配边界
        bitmap_neg_not_new(p_son_list);
        bitmap_and_not_new(p_info->dom_frontier, p_son_list);
        bitmap_drop(p_son_list);
    }
}

void mem2reg_insert_phi(p_convert_ssa_list p_convert_list, p_ssa_var_list_info p_var_list) {
    size_t work_num = p_convert_list->p_func->block_cnt + 1;
    // 记录原来的集合
    p_bitmap p_old = bitmap_gen(p_var_list->vmem_num);
    // 工作队列
    size_t *p_work_list = malloc(work_num * sizeof(*p_work_list));
    size_t work_tail = 0;
    // 第一遍得到块的指令定值集合，同时得到部分 phi 集合
    p_list_head p_node;
    list_for_each(p_node, &p_convert_list->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_convert_ssa p_info = p_convert_list->p_base + p_basic_block->block_id;
        // 将 phi 集合加入到定值集合
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            p_ir_operand p_operand = ir_instr_get_store_addr(p_instr);
            //  将已经声明的变量和ret变量加入到该块定值集合
            size_t id = get_var_index(p_operand, p_var_list);
            if (id != -1)
                bitmap_add_element(p_info->p_def_var, id);
        }
        // 遍历支配边界将定值集合并入到 边界的 phi 集合
        p_list_head p_node_;
        list_for_each(p_node_, &p_convert_list->p_func->block) {
            size_t j = list_entry(p_node_, ir_basic_block, node)->block_id;
            if (bitmap_if_in(p_info->dom_frontier, j)) { // 若 phi 集合发生变化且已经遍历过且没有被加入过工作集合需要加入到工作集合之后处理
                bitmap_copy_not_new(p_old, (p_convert_list->p_base + j)->p_phi_var);
                bitmap_merge_not_new((p_convert_list->p_base + j)->p_phi_var, p_info->p_def_var);
                bool if_change = !bitmap_if_equal(p_old, (p_convert_list->p_base + j)->p_phi_var);
                if (if_change && j <= p_basic_block->block_id && !(p_convert_list->p_base + j)->if_in) {
                    p_work_list[work_tail++] = j;
                    (p_convert_list->p_base + j)->if_in = true;
                }
            }
        }
    }

    // 处理之前 phi 集合未完全处理完的块
    for (size_t work_head = 0; work_head != work_tail % work_num; work_head = (work_head + 1) % work_num) {
        p_convert_ssa p_info = p_convert_list->p_base + p_work_list[work_head];
        p_info->if_in = false;
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        list_for_each(p_node, &p_convert_list->p_func->block){
            size_t j = list_entry(p_node, ir_basic_block, node)->block_id;
            // 当新的 phi 集合发生变化 并且没有被加入到工作集时需要加入工作集合
            if (bitmap_if_in(p_info->dom_frontier, j)) {
                bitmap_copy_not_new(p_old, (p_convert_list->p_base + j)->p_phi_var);
                bitmap_merge_not_new((p_convert_list->p_base + j)->p_phi_var, p_info->p_def_var);
                bool if_change = !bitmap_if_equal(p_old, (p_convert_list->p_base + j)->p_phi_var);
                if (if_change && !(p_convert_list->p_base + j)->if_in) {
                    p_work_list[work_tail] = j;
                    work_tail = (work_tail + 1) % work_num;
                    (p_convert_list->p_base + j)->if_in = true;
                }
            }
        }
    }
    bitmap_drop(p_old);
    free(p_work_list);
}

static inline p_ir_operand get_top_operand(p_ssa_var_list_info p_var_list, size_t index) {
    p_ssa_var_info p_info = p_var_list->p_base + index;
    if (!p_info->p_current_vreg) return NULL;
    return ir_operand_vreg_gen(p_info->p_current_vreg);
}

static inline void set_branch_target_ssa_id(p_ir_basic_block_branch_target p_branch_target, p_convert_ssa_list p_convert_list, p_ssa_var_list_info p_var_list) {
    size_t var_num = p_var_list->vmem_num;
    p_bitmap p_phi_var = (p_convert_list->p_base + p_branch_target->p_block->block_id)->p_phi_var;
    for (size_t i = 0; i < var_num; i++) {
        if (bitmap_if_in(p_phi_var, i)) {
            p_ir_operand p_param = get_top_operand(p_var_list, i);
            ir_basic_block_branch_target_add_param(p_branch_target, p_param);
        }
    }
}

// 为基本块创建 phi 参数列表并命名
static inline void set_block_param_ssa_id(p_ir_basic_block p_basic_block, p_convert_ssa_list p_convert_list, p_ssa_var_list_info p_var_list) {
    size_t var_num = p_var_list->vmem_num;
    p_bitmap p_phi_var = (p_convert_list->p_base + p_basic_block->block_id)->p_phi_var;
    for (size_t i = 0; i < var_num; i++) {
        if (bitmap_if_in(p_phi_var, i)) {
            p_ir_vreg p_vreg = ir_vreg_gen(symbol_type_copy((p_var_list->p_base + i)->p_vmem->p_type));
            symbol_func_vreg_add(p_var_list->p_func, p_vreg);
            (p_var_list->p_base + i)->p_current_vreg = p_vreg;
            ir_basic_block_add_param(p_basic_block, p_vreg);
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
void mem2reg_rename_var(p_ssa_var_list_info p_var_list, p_convert_ssa_list p_convert_list, p_ir_basic_block p_entry) {
    if (p_entry->if_visited) return;
    p_entry->if_visited = true;
    // 记录入块信息
    record_current_sym(p_var_list);
    set_block_param_ssa_id(p_entry, p_convert_list, p_var_list);

    p_list_head p_node;
    list_for_each(p_node, &p_entry->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);

        if (p_instr->irkind == ir_load) {
            size_t var_index = get_var_index(p_instr->ir_load.p_addr, p_var_list);
            if (var_index == -1) continue;
            assert(!p_instr->ir_store.p_offset);

            p_ir_operand p_top_operand = get_top_operand(p_var_list, var_index);
            if (!p_top_operand) {
                p_var_list->p_base[var_index].p_current_vreg = p_instr->ir_load.p_des;
                continue;
            }

            ir_operand_drop(p_instr->ir_load.p_addr);
            p_instr->irkind = ir_unary;
            p_instr->ir_unary.op = ir_val_assign;
            p_instr->ir_unary.p_des = p_instr->ir_load.p_des;
            p_instr->ir_unary.p_src = p_top_operand;
            continue;
        }
        if (p_instr->irkind == ir_store) {
            size_t var_index = get_var_index(p_instr->ir_store.p_addr, p_var_list);
            if (var_index == -1) continue;
            assert(!p_instr->ir_store.p_offset);

            p_ir_vreg p_vreg = ir_vreg_gen(symbol_type_copy(p_var_list->p_base[var_index].p_vmem->p_type));
            ir_vreg_set_instr_def(p_vreg, p_instr);
            symbol_func_vreg_add(p_var_list->p_func, p_vreg);
            p_var_list->p_base[var_index].p_current_vreg = p_vreg;

            ir_operand_drop(p_instr->ir_store.p_addr);
            p_instr->irkind = ir_unary;
            p_instr->ir_unary.op = ir_val_assign;
            p_instr->ir_unary.p_src = p_instr->ir_store.p_src;
            p_instr->ir_unary.p_des = p_vreg;
            continue;
        }
    }
    p_ir_basic_block_branch p_branch = p_entry->p_branch;
    if (p_branch->kind == ir_br_branch) {
        set_branch_target_ssa_id(p_branch->p_target_1, p_convert_list, p_var_list);
        mem2reg_rename_var(p_var_list, p_convert_list, p_branch->p_target_1->p_block);
    }
    if (p_branch->kind == ir_cond_branch) {
        set_branch_target_ssa_id(p_branch->p_target_1, p_convert_list, p_var_list);
        mem2reg_rename_var(p_var_list, p_convert_list, p_branch->p_target_1->p_block);
        set_branch_target_ssa_id(p_branch->p_target_2, p_convert_list, p_var_list);
        mem2reg_rename_var(p_var_list, p_convert_list, p_branch->p_target_2->p_block);
    }
    // 恢复信息
    restore_current_sym(p_var_list);
}
#include <stdio.h>
static inline void print_dom_frontier(p_convert_ssa_list p_convert_list) {
    p_list_head p_node;
    printf(" --- dom_frontier start---\n");
    list_for_each(p_node, &p_convert_list->p_func->block){
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_convert_ssa p_info = p_convert_list->p_base + p_basic_block->block_id;
        printf("b%ld ", p_basic_block->block_id);
        bitmap_print(p_info->dom_frontier);
        printf("\n");
    }
    printf("--- dom_frontier end---\n");
}

static inline void delete_vmem(p_symbol_func p_func) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_func->variable) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        if (!list_head_alone(&p_var->p_type->array) && p_var->p_type->ref_level == 0) continue;
        if (p_var->is_const) continue;
        if (p_var->is_global) continue;
        symbol_func_delete_varible(p_func, p_var);
    }
    symbol_func_set_varible_id(p_func);
}

void mem2reg_func_pass(p_symbol_func p_func) {
    if (list_head_alone(&p_func->block)) return;
    p_convert_ssa_list p_convert_list = mem2reg_info_gen(p_func);
    // 初始化变量集合
    p_ssa_var_list_info p_var_list = mem2reg_init_var_list(p_func);
    // 初始化 dfs 序
    symbol_func_basic_block_init_visited(p_func);
    p_ir_basic_block p_entry = list_entry(p_func->block.p_next, ir_basic_block, node);
    // 计算支配树
    ir_cfg_set_func_dom(p_func);
    // 计算支配边界
    mem2reg_compute_dom_frontier(p_convert_list);
    print_dom_frontier(p_convert_list);
    // 插入 phi 函数
    mem2reg_insert_phi(p_convert_list, p_var_list);
    // // 重命名
    symbol_func_basic_block_init_visited(p_func);
    mem2reg_rename_var(p_var_list, p_convert_list, p_entry);

    convert_ssa_dfs_seq_drop(p_convert_list);
    ssa_var_list_info_drop(p_var_list);

    delete_vmem(p_func);
    symbol_func_set_block_id(p_func);
}

void mem2reg_program_pass(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        mem2reg_func_pass(p_func);
    }
}

void convert_ssa_dfs_seq_drop(p_convert_ssa_list p_convert_list) {
    for (size_t i = 0; i < p_convert_list->p_func->block_cnt; i++) {
        bitmap_drop((p_convert_list->p_base + i)->dom_frontier);
        bitmap_drop((p_convert_list->p_base + i)->p_def_var);
        bitmap_drop((p_convert_list->p_base + i)->p_phi_var);
    }
    free(p_convert_list->p_base);
    free(p_convert_list);
}

void ssa_var_list_info_drop(p_ssa_var_list_info p_var_list) {
    free(p_var_list->p_base);
    free(p_var_list);
}
