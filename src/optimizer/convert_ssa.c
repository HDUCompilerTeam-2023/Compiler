#include <optimizer/convert_ssa.h>
#include <mir_port/basic_block.h>
#include <symbol/sym.h>
#include <symbol/type.h>
void convert_ssa_gen(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_mir_basic_block p_basic_block, size_t current_num)
{
    dfs_seq[current_num] = (convert_ssa){
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

size_t convert_ssa_init_dfs_sequence(convert_ssa *dfs_seq, size_t block_num, size_t var_num, p_mir_basic_block p_entry, size_t current_num)
{
    if(p_entry->if_visited)return current_num;
    p_entry->if_visited = true;
    convert_ssa_gen(dfs_seq, block_num, var_num, p_entry, current_num);
    current_num++;

    p_mir_basic_block p_true_block = mir_basic_block_get_true(p_entry);
    p_mir_basic_block p_false_block = mir_basic_block_get_false(p_entry);

    if (p_true_block)
        current_num = convert_ssa_init_dfs_sequence(dfs_seq, block_num, var_num, p_true_block, current_num);
    if (p_false_block)
        current_num = convert_ssa_init_dfs_sequence(dfs_seq, block_num, var_num, p_false_block, current_num);
    return current_num;
}

void convert_ssa_init_var_list(p_ssa_var_info p_var_list, size_t var_num, p_mir_func p_func, p_mir_temp_sym p_ret)
{
    p_list_head p_node;
    p_symbol_type p_param_type = p_func->p_func_sym->p_type->p_params;
    // 函数参数定值已经初始化
    list_for_each(p_node, &p_func->p_func_sym->variable) {
        if (!p_param_type) break;
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        *(p_var_list + p_sym->id) = (ssa_var_info) {
            .p_operand = mir_operand_declared_sym_gen(p_sym),
            .count = 1,
            .current_count = 0,
        };
        p_param_type = p_param_type->p_params;
    }
    // 局部变量
    while(p_node != &p_func->p_func_sym->variable) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        *(p_var_list + p_sym->id) = (ssa_var_info) {
            .p_operand = mir_operand_declared_sym_gen(p_sym),
            .count = 0,
            .current_count = 0,
        };
        p_node = p_node->p_next;
    }
    // 返回值
    *(p_var_list + var_num - 1) = (ssa_var_info) {
        .p_operand = mir_operand_temp_sym_gen(p_ret),
        .count = 0,
        .current_count = 0,
    };
}


void convert_ssa_compute_dom_frontier(convert_ssa *dfs_seq, size_t block_num)
{
    for(size_t i = block_num - 1; i < block_num; i --){
        p_convert_ssa p_info = dfs_seq + i;

        p_mir_basic_block p_true_block = mir_basic_block_get_true(p_info->p_basic_block);
        p_mir_basic_block p_false_block = mir_basic_block_get_false(p_info->p_basic_block);

            // 将直接后继做为 DF_up 的候选
            if (p_true_block)
                bitmap_add_element(p_info->dom_frontier, p_true_block->dfn_id);
            if (p_false_block)
                bitmap_add_element(p_info->dom_frontier, p_false_block->dfn_id);
            
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
// 合并集合并返回合并前后集合是否发生变化
static inline bool merge_if_change(p_bitmap p_b1, p_bitmap p_b2){
    p_bitmap p_old = bitmap_copy(p_b1);
    bitmap_merge_not_new(p_b1, p_b2);
    if (bitmap_if_equal(p_old, p_b1))
    {
        bitmap_drop(p_old);
        return true;
    }
    bitmap_drop(p_old);
    return false;
}

void convert_ssa_insert_phi(p_convert_ssa dfs_seq, size_t block_num, p_mir_temp_sym p_ret, size_t var_num)
{
    // 入口块对所有变量已经定值
    bitmap_set_full(dfs_seq->p_def_var);
    size_t work_num = block_num + 1;
    // 工作队列
    size_t* p_work_list = malloc(work_num * sizeof(*p_work_list));
    size_t work_tail = 0;
    // 第一遍得到块的指令定值集合，同时得到部分 phi 集合 
    for(size_t i = 0; i < block_num; i ++){
        p_convert_ssa p_info = dfs_seq + i;
        // 将 phi 集合加入到定值集合
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        p_list_head p_node;
        list_for_each(p_node, &p_info->p_basic_block->instr_list){
            p_mir_instr p_instr = list_entry(p_node, mir_instr, node);
            p_mir_operand p_operand = mir_instr_get_des(p_instr);
            //  将已经声明的变量和ret变量加入到该块定值集合
            if (p_operand) {
                if(p_operand->kind == declared_var)
                    bitmap_add_element(p_info->p_def_var, p_operand->p_sym->id);
                else if ((p_operand->kind == temp_var && p_operand->p_temp_sym == p_ret)) {
                    bitmap_add_element(p_info->p_def_var, var_num - 1);
                }
            }
        }
        // 遍历支配边界将定值集合并入到 边界的 phi 集合
        for(size_t j = 0; j < block_num; j ++){
            if(bitmap_if_in(p_info->dom_frontier, j))
            {   // 若 phi 集合发生变化且已经遍历过且没有被加入过工作集合需要加入到工作集合之后处理
                bool if_change = merge_if_change((dfs_seq + j)->p_phi_var, p_info->p_def_var);
                if (if_change && j <= i && !(dfs_seq + j)->if_in) {
                    p_work_list[work_tail ++] = j;
                    (dfs_seq + j)->if_in = true;
                }
            }
        }
    }

    // 处理之前 phi 集合未完全处理完的块
    for(size_t work_head = 0; work_head != work_tail % work_num; work_head = (work_head + 1) % work_num){
        p_convert_ssa p_info = dfs_seq + p_work_list[work_head];
        p_info->if_in = false;
        bitmap_merge_not_new(p_info->p_def_var, p_info->p_phi_var);
        for (size_t j = 0; j < block_num; j ++) {
            // 当新的 phi 集合发生变化 并且没有被加入到工作集时需要加入工作集合 
            if (bitmap_if_in(p_info->dom_frontier, j)) {
                bool if_change = merge_if_change((dfs_seq + j)->p_phi_var, p_info->p_def_var);
                if (if_change && !(dfs_seq + j)->if_in) {
                    p_work_list[work_tail] = j;
                    work_tail = (work_tail + 1) % work_num;
                    (dfs_seq + j)->if_in = true;
                }
            }
        }
    }
    free(p_work_list);
}

// 将集合重写为基本块参数
void convert_ssa_rewrite_phi(p_convert_ssa dfs_seq, size_t block_num, p_ssa_var_info p_var_list, size_t var_num)
{
    for (size_t i = 0; i < block_num; i ++) {
        p_convert_ssa p_info = dfs_seq + i;
        for(size_t j = 0; j < var_num; j ++)
        {
            if(bitmap_if_in(p_info->p_phi_var, j))
            {
                mir_basic_block_add_param(p_info->p_basic_block, mir_operand_copy((p_var_list + j)->p_operand));
                p_list_head p_node;
                list_for_each(p_node, &p_info->p_basic_block->prev_basic_block_list){
                    p_mir_basic_block p_prev_block = list_entry(p_node, mir_basic_block_list_node, node)->p_basic_block;
                    p_mir_instr p_last_instr = list_entry(p_prev_block->instr_list.p_prev, mir_instr, node);
                    if (p_last_instr->irkind == mir_br)
                        mir_basic_block_call_add_param(p_last_instr->mir_br.p_target, mir_operand_copy((p_var_list + j)->p_operand));
                    else if(p_last_instr->irkind == mir_condbr)
                    {
                        if(p_last_instr->mir_condbr.p_target_true->p_block == p_info->p_basic_block)
                            mir_basic_block_call_add_param(p_last_instr->mir_condbr.p_target_true, mir_operand_copy((p_var_list + j)->p_operand));
                        if (p_last_instr->mir_condbr.p_target_false->p_block == p_info->p_basic_block)
                            mir_basic_block_call_add_param(p_last_instr->mir_condbr.p_target_false, mir_operand_copy((p_var_list + j)->p_operand));
                    }
                }
            }
        }
    }
}



#include <stdio.h>
    static inline void print_dom_frontier(convert_ssa *dfs_seq, size_t block_num) {
    printf(" --- dom_frontier start---\n");
    for(size_t i = 0; i < block_num; i ++)
    {
        p_convert_ssa p_info = dfs_seq + i;
        printf("b%ld (dfn_id: %ld): ", p_info->p_basic_block->block_id, p_info->p_basic_block->dfn_id);
        bitmap_print(p_info->dom_frontier);
        printf("\n");
    }
    printf(" --- dom_frontier end---\n");
}


void convert_ssa_func(p_mir_func p_func){
    if (list_head_alone(&p_func->entry_block)) return;
    size_t block_num = list_entry(p_func->entry_block.p_prev, mir_basic_block, node)->block_id + 1;
    p_convert_ssa dfs_seq = malloc(block_num * sizeof(*dfs_seq));
    // 初始化变量集合
    size_t var_num = 1;
    if (!list_head_alone(&p_func->p_func_sym->variable))
        var_num += list_entry(p_func->p_func_sym->variable.p_prev, symbol_sym, node)->id + 1;
    p_ssa_var_info p_var_list = malloc(sizeof(*p_var_list) * var_num);
    p_mir_temp_sym p_ret = list_entry(p_func->temp_sym_head.p_prev, mir_temp_sym, node);
    convert_ssa_init_var_list(p_var_list, var_num, p_func, p_ret);
    // 初始化 dfs 序
    mir_basic_block_init_visited(p_func);
    p_mir_basic_block p_entry = list_entry(p_func->entry_block.p_next, mir_basic_block, node);
    convert_ssa_init_dfs_sequence(dfs_seq, block_num, var_num, p_entry, 0);
    // 计算支配边界
    convert_ssa_compute_dom_frontier(dfs_seq, block_num);
    print_dom_frontier(dfs_seq, block_num);
    // 插入 phi 函数
    convert_ssa_insert_phi(dfs_seq, block_num, p_ret, var_num);
    convert_ssa_rewrite_phi(dfs_seq, block_num, p_var_list, var_num);

    convert_ssa_dfs_seq_drop(dfs_seq, block_num);
    ssa_var_info_drop(p_var_list, var_num);
}

void convert_ssa_program(p_mir_program p_program){
    for (size_t i = 0; i < p_program->func_cnt; i++)
        convert_ssa_func(p_program->func_table + i);
}

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num) {
    for (size_t i = 0; i < block_num; i++){
        bitmap_drop((dfs_seq + i)->dom_frontier);
        bitmap_drop((dfs_seq + i)->p_def_var);
        bitmap_drop((dfs_seq + i)->p_phi_var);
    }
    free(dfs_seq);
}

void ssa_var_info_drop(p_ssa_var_info p_info, size_t var_num)
{
    for(size_t i = 0; i < var_num; i ++)
    {
        mir_operand_drop((p_info + i)->p_operand);
    }
    free(p_info);
}