#include <ir_gen.h>
#include <program/def.h>
#include <symbol/type.h>
#include <symbol/var.h>
#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
typedef struct mem_ssa mem_ssa, *p_mem_ssa;
typedef struct operand_stack_node operand_stack_node, *p_operand_stack_node;
typedef struct ssa_mem_info ssa_mem_info, *p_ssa_mem_info;
typedef struct ssa_mem_list_info ssa_mem_list_info, *p_ssa_mem_list_info;
typedef struct sym_stack_node sym_stack_node, *p_sym_stack_node;
typedef struct mem_ssa_list mem_ssa_list, *p_mem_ssa_list;

struct mem_ssa {
    p_bitmap dom_frontier; // 支配边界

    p_bitmap p_phi_var; // phi 函数变量集合
    p_bitmap p_def_var; // 定值集合

    bool if_create_phi;
    bool if_in; // 是否在工作表中
};

struct ssa_mem_info {
    p_ir_vmem_base p_vmem_base;
    p_ir_varray p_current_varray;
    list_head sym_stack; // 保存进入基本块时的信息
    bool has_store;
    size_t id;
};

struct sym_stack_node {
    list_head node;
    p_ir_varray p_varray;
};

struct mem_ssa_list {
    p_mem_ssa p_base;
    p_symbol_func p_func;
};

struct ssa_mem_list_info {
    p_ssa_mem_info p_base;
    size_t vmem_base_num;
    p_symbol_func p_func;
    p_program p_program;
};

static inline p_ir_vmem_base get_vmem_base(p_ir_operand p_operand) {
    assert(p_operand);
    while (1) {
        assert(p_operand->p_type->ref_level > 0);
        if (p_operand->kind == reg) {
            if (p_operand->p_vreg->def_type == func_param_def)
                return symbol_func_get_param_vmem_base(p_operand->p_vreg->p_func, p_operand->p_vreg)->p_param_base;
            assert(p_operand->p_vreg->def_type == instr_def);
            p_ir_instr p_def_instr = p_operand->p_vreg->p_instr_def;
            switch (p_def_instr->irkind) {
            case ir_load:
                // now just deal ptr in param, may be have some problems
                assert(p_def_instr->ir_load.p_addr->kind == imme);
                p_list_head p_instr_node;
                p_ir_operand p_record = p_operand;
                p_instr_node = &p_def_instr->node;
                p_ir_basic_block p_basic_block = p_def_instr->p_basic_block;
                p_symbol_func p_func = p_basic_block->p_func;
                while (1) {
                    while (p_instr_node != &p_basic_block->instr_list) {
                        p_ir_instr p_instr = list_entry(p_instr_node, ir_instr, node);
                        if (p_instr->irkind == ir_store) {
                            if (p_instr->ir_store.p_addr->kind == imme
                                && p_instr->ir_store.p_addr->p_vmem == p_def_instr->ir_load.p_addr->p_vmem) {
                                p_operand = p_instr->ir_store.p_src;
                                break;
                            }
                        }
                        p_instr_node = p_instr_node->p_prev;
                    }
                    if (p_operand != p_record) break;
                    p_basic_block = list_entry(p_basic_block->node.p_prev, ir_basic_block, node);
                    if (&p_basic_block->node == &p_func->block) break;

                    while (list_head_alone(&p_basic_block->instr_list)) {
                        p_basic_block = list_entry(p_basic_block->node.p_prev, ir_basic_block, node);
                        if (&p_basic_block->node == &p_func->block) break;
                    }
                    if (&p_basic_block->node == &p_func->block) break;

                    p_instr_node = p_basic_block->instr_list.p_prev;
                }
                assert(p_record != p_operand);
                break;
            case ir_gep:
                p_operand = p_def_instr->ir_gep.p_addr;
                break;
            case ir_binary:
                assert(p_def_instr->ir_binary.op == ir_add_op);
                if (p_def_instr->ir_binary.p_src1->p_type->ref_level > 0)
                    p_operand = p_def_instr->ir_binary.p_src1;
                else
                    p_operand = p_def_instr->ir_binary.p_src2;
                break;
            case ir_unary:
                switch (p_def_instr->ir_unary.op) {
                case ir_val_assign:
                    p_operand = p_def_instr->ir_unary.p_src;
                    break;
                case ir_ptr_add_sp:
                    p_operand = p_def_instr->ir_unary.p_src;
                    break;
                default:
                    assert(0);
                }
                break;
            default:
                assert(0);
            }
        }
        else
            return p_operand->p_vmem->p_base;
    }
}

p_mem_ssa_list memssa_info_gen(p_symbol_func p_func, size_t vmem_base_num) {
    p_mem_ssa_list p_convert_list = malloc(sizeof(*p_convert_list));
    p_convert_list->p_func = p_func;
    p_convert_list->p_base = malloc(sizeof(*p_convert_list->p_base) * p_func->block_cnt);
    p_list_head p_node;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_convert_list->p_base[p_basic_block->block_id] = (mem_ssa) {
            .dom_frontier = bitmap_gen(p_func->block_cnt),
            .p_def_var = bitmap_gen(vmem_base_num),
            .p_phi_var = bitmap_gen(vmem_base_num),
            .if_create_phi = false,
            .if_in = false,
        };
        bitmap_set_empty((p_convert_list->p_base + p_basic_block->block_id)->dom_frontier);
        bitmap_set_empty((p_convert_list->p_base + p_basic_block->block_id)->p_def_var);
        bitmap_set_empty((p_convert_list->p_base + p_basic_block->block_id)->p_phi_var);
    }
    return p_convert_list;
}
static inline void memssa_init_var(p_ssa_mem_list_info p_list, size_t index, p_ir_vmem_base p_base) {
    assert(p_base);
    (p_list->p_base + index)->p_vmem_base = p_base;
    (p_list->p_base + index)->p_current_varray = ir_varray_gen(p_base);
    ir_varray_set_func_def((p_list->p_base + index)->p_current_varray, p_list->p_func);
    (p_list->p_base + index)->sym_stack = list_head_init(&(p_list->p_base + index)->sym_stack);
    (p_list->p_base + index)->id = index;
    (p_list->p_base + index)->has_store = false;
    p_base->p_info = (p_list->p_base + index);
}
p_ssa_mem_list_info memssa_init_var_list(p_symbol_func p_func, p_program p_ir) {
    p_ssa_mem_list_info p_var_list = malloc(sizeof(*p_var_list));
    *p_var_list = (ssa_mem_list_info) {
        .vmem_base_num = p_func->var_cnt + p_ir->variable_cnt + p_func->param_vmem_base_num,
        .p_func = p_func,
        .p_program = p_ir,
    };

    p_var_list->p_base = malloc(p_var_list->vmem_base_num * sizeof(*p_var_list->p_base));
    p_list_head p_node;
    size_t index = 0;
    // 全局变量, 已初始化
    list_for_each(p_node, &p_ir->variable) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        memssa_init_var(p_var_list, index, p_vmem_base);
        index++;
    }
    list_for_each(p_node, &p_func->variable) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        memssa_init_var(p_var_list, index, p_vmem_base);
        index++;
    }
    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, ir_param_vmem_base, node)->p_param_base;
        memssa_init_var(p_var_list, index, p_vmem_base);
        index++;
    }
    assert(index == p_var_list->vmem_base_num);
    return p_var_list;
}

void memssa_compute_dom_frontier(p_mem_ssa_list p_convert_list) {
    p_list_head p_node;
    list_for_each_tail(p_node, &p_convert_list->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_mem_ssa p_info = p_convert_list->p_base + p_basic_block->block_id;
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
        list_for_each(p_node, &p_basic_block->dom_son_list->block_list) {
            p_ir_basic_block p_son = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            p_mem_ssa p_son_info = p_convert_list->p_base + p_son->block_id;
            bitmap_add_element(p_son_list, p_son->block_id);
            bitmap_merge_not_new(p_info->dom_frontier, p_son_info->dom_frontier);
        }
        // 所有候选中不受当前节点直接支配的节点为支配边界
        bitmap_neg_not_new(p_son_list);
        bitmap_and_not_new(p_info->dom_frontier, p_son_list);
        bitmap_drop(p_son_list);
    }
}

void memssa_insert_phi(p_mem_ssa_list p_convert_list, p_ssa_mem_list_info p_var_list) {
    size_t work_num = p_convert_list->p_func->block_cnt + 1;
    // 记录原来的集合
    p_bitmap p_old = bitmap_gen(p_var_list->vmem_base_num);
    // 工作队列
    size_t *p_work_list = malloc(work_num * sizeof(*p_work_list));
    size_t work_tail = 0;
    // 第一遍得到块的指令定值集合，同时得到部分 phi 集合
    p_list_head p_node;
    list_for_each(p_node, &p_convert_list->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_mem_ssa p_info = p_convert_list->p_base + p_basic_block->block_id;
        // 将 phi 集合加入到定值集合
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
            if (p_instr->irkind == ir_store) {
                p_ssa_mem_info p_vmem_base_info = get_vmem_base(p_instr->ir_store.p_addr)->p_info;
                bitmap_add_element(p_info->p_def_var, p_vmem_base_info->id);
            }
            else if (p_instr->irkind == ir_call) {
                p_list_head p_node;
                list_for_each(p_node, &p_var_list->p_program->variable) {
                    p_ssa_mem_info p_vmem_base_info = list_entry(p_node, symbol_var, node)->p_base->p_info;
                    bitmap_add_element(p_info->p_def_var, p_vmem_base_info->id);
                }
                list_for_each(p_node, &p_instr->ir_call.param_list) {
                    p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                    if (p_param->p_type->ref_level) {
                        p_ssa_mem_info p_vmem_base_info = get_vmem_base(p_param)->p_info;
                        bitmap_add_element(p_info->p_def_var, p_vmem_base_info->id);
                    }
                }
            }
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
        p_mem_ssa p_info = p_convert_list->p_base + p_work_list[work_head];
        p_info->if_in = false;
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        list_for_each(p_node, &p_convert_list->p_func->block) {
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

static inline p_ir_varray_use get_top_varray_use(p_ssa_mem_info p_info) {
    assert(p_info);
    assert(p_info->p_current_varray);
    return ir_varray_use_gen(p_info->p_current_varray);
}

static inline void set_branch_target_ssa_id(p_ir_basic_block_branch_target p_branch_target, p_mem_ssa_list p_convert_list, p_ssa_mem_list_info p_var_list) {
    size_t var_num = p_var_list->vmem_base_num;
    p_bitmap p_phi_var = (p_convert_list->p_base + p_branch_target->p_block->block_id)->p_phi_var;
    if (!(p_convert_list->p_base + p_branch_target->p_block->block_id)->if_create_phi) {
        (p_convert_list->p_base + p_branch_target->p_block->block_id)->if_create_phi = true;
        for (size_t i = 0; i < var_num; i++) {
            if (bitmap_if_in(p_phi_var, i)) {
                p_ir_varray_use p_varray_param = get_top_varray_use(p_var_list->p_base + i);
                p_ir_varray p_varray = ir_varray_gen((p_var_list->p_base + i)->p_vmem_base);
                p_ir_varray_bb_phi p_phi = ir_basic_block_add_varray_phi(p_branch_target->p_block, p_varray);
                p_ir_varray_bb_param p_param = ir_basic_block_branch_target_add_varray_param(p_branch_target, p_phi, p_varray_param);
                assert(p_param->p_des_phi == p_phi);
            }
        }
    }
    else {
        p_list_head p_node;
        list_for_each(p_node, &p_branch_target->p_block->varray_basic_block_phis) {
            p_ir_varray_bb_phi p_phi = list_entry(p_node, ir_varray_bb_phi, node);
            p_ir_varray_use p_varray_param = get_top_varray_use((p_ssa_mem_info) p_phi->p_varray_phi->p_base->p_info);
            p_ir_varray_bb_param p_param = ir_basic_block_branch_target_add_varray_param(p_branch_target, p_phi, p_varray_param);
            assert(p_param->p_des_phi == p_phi);
        }
    }
}

// 为基本块创建 phi 参数列表并命名
static inline void set_block_param_ssa_id(p_ir_basic_block p_basic_block, p_mem_ssa_list p_convert_list, p_ssa_mem_list_info p_var_list) {
    if (p_basic_block != p_convert_list->p_func->p_entry_block) {
        assert((p_convert_list->p_base + p_basic_block->block_id)->if_create_phi);
    }
    else
        return;
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->varray_basic_block_phis) {
        p_ir_varray_bb_phi p_phi = list_entry(p_node, ir_varray_bb_phi, node);
        ((p_ssa_mem_info) p_phi->p_varray_phi->p_base->p_info)->p_current_varray = p_phi->p_varray_phi;
    }
}

static inline void record_current_sym(p_ssa_mem_list_info p_var_list) {
    size_t vmem_baes_num = p_var_list->vmem_base_num;
    for (size_t i = 0; i < vmem_baes_num; i++) {
        p_ssa_mem_info p_info = p_var_list->p_base + i;
        p_sym_stack_node p_node = malloc(sizeof(*p_node));
        *p_node = (sym_stack_node) {
            .node = list_head_init(&p_node->node),
            .p_varray = p_info->p_current_varray,
        };
        list_add_prev(&p_node->node, &p_info->sym_stack);
    }
}

static inline void restore_current_sym(p_ssa_mem_list_info p_var_list) {
    size_t vmem_num = p_var_list->vmem_base_num;
    for (size_t i = 0; i < vmem_num; i++) {
        p_ssa_mem_info p_info = p_var_list->p_base + i;
        p_sym_stack_node p_node = list_entry(p_info->sym_stack.p_prev, sym_stack_node, node);
        p_info->p_current_varray = p_node->p_varray;
        list_del(&p_node->node);
        free(p_node);
    }
}
#include <ir_print.h>
void memssa_rename_var(p_ssa_mem_list_info p_var_list, p_mem_ssa_list p_convert_list, p_ir_basic_block p_entry) {
    if (p_entry->if_visited) return;
    p_entry->if_visited = true;
    // 记录入块信息
    record_current_sym(p_var_list);
    set_block_param_ssa_id(p_entry, p_convert_list, p_var_list);

    p_list_head p_node;
    list_for_each(p_node, &p_entry->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        if (p_instr->irkind == ir_load) {
            p_ssa_mem_info p_vmem_base_info = get_vmem_base(p_instr->ir_load.p_addr)->p_info;

            p_ir_varray_use p_top_varray_use = get_top_varray_use(p_vmem_base_info);

            ir_load_instr_set_varray_src(p_instr, p_top_varray_use);
            continue;
        }
        if (p_instr->irkind == ir_store) {
            p_ssa_mem_info p_vmem_base_info = get_vmem_base(p_instr->ir_store.p_addr)->p_info;

            ir_store_instr_set_varray_src(p_instr, get_top_varray_use(p_vmem_base_info));

            p_ir_varray p_varray = ir_varray_gen(p_vmem_base_info->p_vmem_base);
            ir_store_instr_set_varray_des(p_instr, p_varray);
            p_vmem_base_info->p_current_varray = p_varray;
            continue;
        }
        if (p_instr->irkind == ir_call) {
            p_list_head p_node;
            list_for_each(p_node, &p_instr->ir_call.p_func->p_side_effects->stored_global) {
                p_ssa_mem_info p_vmem_base_info = list_entry(p_node, mem_visit_node, node)->p_global->p_base->p_info;
                p_vmem_base_info->has_store = true;
                p_ir_varray_use p_use = get_top_varray_use(p_vmem_base_info);
                p_ir_varray p_des = ir_varray_gen(p_vmem_base_info->p_vmem_base);
                p_ir_varray_def_pair p_pair = ir_varray_def_pair_gen(p_des, p_use);
                ir_call_instr_add_varray_def_pair(p_instr, p_pair);
                p_vmem_base_info->p_current_varray = p_des;
            }
            list_for_each(p_node, &p_instr->ir_call.p_func->p_side_effects->loaded_global) {
                p_ssa_mem_info p_vmem_base_info = list_entry(p_node, mem_visit_node, node)->p_global->p_base->p_info;
                if (p_vmem_base_info->has_store) {
                    p_vmem_base_info->has_store = false;
                    continue;
                }
                p_ir_varray_use p_use = get_top_varray_use(p_vmem_base_info);
                p_ir_varray_def_pair p_pair = ir_varray_def_pair_gen(NULL, p_use);
                ir_call_instr_add_varray_def_pair(p_instr, p_pair);
            }
            size_t i = 0;
            ir_instr_print(p_instr);
            list_for_each(p_node, &p_instr->ir_call.param_list) {
                p_ir_operand p_param = list_entry(p_node, ir_param, node)->p_param;
                if (p_instr->ir_call.p_func->p_side_effects->stored_param[i]) {
                    p_ssa_mem_info p_vmem_base_info = get_vmem_base(p_param)->p_info;
                    if (p_vmem_base_info->has_store) {
                        p_vmem_base_info->has_store = false;
                        continue;
                    }
                    p_ir_varray_use p_use = get_top_varray_use(p_vmem_base_info);
                    p_ir_varray p_des = ir_varray_gen(p_vmem_base_info->p_vmem_base);
                    p_ir_varray_def_pair p_pair = ir_varray_def_pair_gen(p_des, p_use);
                    ir_call_instr_add_varray_def_pair(p_instr, p_pair);
                    ir_varray_def_pair_print(p_pair);
                    p_vmem_base_info->p_current_varray = p_des;
                    i++;
                    continue;
                }
                if (p_instr->ir_call.p_func->p_side_effects->loaded_param[i]) {
                    p_ssa_mem_info p_vmem_base_info = get_vmem_base(p_param)->p_info;
                    p_ir_varray_use p_use = get_top_varray_use(p_vmem_base_info);
                    p_ir_varray_def_pair p_pair = ir_varray_def_pair_gen(NULL, p_use);
                    ir_call_instr_add_varray_def_pair(p_instr, p_pair);
                }
                i++;
            }
            assert(i == p_instr->ir_call.p_func->param_reg_cnt);
            continue;
        }
    }
    p_ir_basic_block_branch p_branch = p_entry->p_branch;
    if (p_branch->kind == ir_br_branch) {
        set_branch_target_ssa_id(p_branch->p_target_1, p_convert_list, p_var_list);
        memssa_rename_var(p_var_list, p_convert_list, p_branch->p_target_1->p_block);
    }
    if (p_branch->kind == ir_cond_branch) {
        set_branch_target_ssa_id(p_branch->p_target_1, p_convert_list, p_var_list);
        memssa_rename_var(p_var_list, p_convert_list, p_branch->p_target_1->p_block);
        set_branch_target_ssa_id(p_branch->p_target_2, p_convert_list, p_var_list);
        memssa_rename_var(p_var_list, p_convert_list, p_branch->p_target_2->p_block);
    }
    // 恢复信息
    restore_current_sym(p_var_list);
}
#include <stdio.h>
static inline void print_dom_frontier(p_mem_ssa_list p_convert_list) {
    p_list_head p_node;
    printf(" --- dom_frontier start---\n");
    list_for_each(p_node, &p_convert_list->p_func->block) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, node);
        p_mem_ssa p_info = p_convert_list->p_base + p_basic_block->block_id;
        printf("b%ld ", p_basic_block->block_id);
        bitmap_print(p_info->dom_frontier);
        printf("\n");
    }
    printf("--- dom_frontier end---\n");
}
static inline void clear_var_varray(p_list_head p_head) {
    p_list_head p_node;
    list_for_each(p_node, p_head) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        assert(p_var->p_base);
        ir_vmem_base_clear_all(p_var->p_base);
    }
}
static inline void clear_param_varray(p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_param_vmem_base p_param = list_entry(p_node, ir_param_vmem_base, node);
        assert(p_param->p_param_base);
        ir_vmem_base_clear_all(p_param->p_param_base);
    }
}
static inline void mem_ssa_dfs_seq_drop(p_mem_ssa_list p_convert_list) {
    for (size_t i = 0; i < p_convert_list->p_func->block_cnt; i++) {
        bitmap_drop((p_convert_list->p_base + i)->dom_frontier);
        bitmap_drop((p_convert_list->p_base + i)->p_def_var);
        bitmap_drop((p_convert_list->p_base + i)->p_phi_var);
    }
    free(p_convert_list->p_base);
    free(p_convert_list);
}

static inline void ssa_mem_list_info_drop(p_ssa_mem_list_info p_var_list) {
    free(p_var_list->p_base);
    free(p_var_list);
}
void memssa_func_pass(p_symbol_func p_func, p_program p_program) {
    clear_var_varray(&p_func->variable);
    clear_param_varray(p_func);
    // 初始化变量集合
    p_ssa_mem_list_info p_var_list = memssa_init_var_list(p_func, p_program);
    p_mem_ssa_list p_convert_list = memssa_info_gen(p_func, p_var_list->vmem_base_num);
    // 初始化 dfs 序
    symbol_func_basic_block_init_visited(p_func);
    p_ir_basic_block p_entry = p_func->p_entry_block; //(p_func->block.p_next, ir_basic_block, node);
    // 计算支配树
    ir_cfg_set_func_dom(p_func);
    // 计算支配边界
    memssa_compute_dom_frontier(p_convert_list);
    print_dom_frontier(p_convert_list);
    // 插入 phi 函数
    memssa_insert_phi(p_convert_list, p_var_list);
    // // 重命名
    symbol_func_basic_block_init_visited(p_func);
    memssa_rename_var(p_var_list, p_convert_list, p_entry);

    mem_ssa_dfs_seq_drop(p_convert_list);
    ssa_mem_list_info_drop(p_var_list);

    symbol_func_set_block_id(p_func);
}

void memssa_program_pass(p_program p_program) {
    clear_var_varray(&p_program->variable);
    ir_side_effects(p_program);
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        memssa_func_pass(p_func, p_program);
    }
}