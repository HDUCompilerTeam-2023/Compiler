#include <optimizer/convert_ssa.h>
#include <mir_port/basic_block.h>
#include <mir_manager.h>
#include <symbol/sym.h>
#include <symbol/type.h>
#include <symbol/store.h>
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

p_ssa_var_list_info convert_ssa_init_var_list(p_mir_func p_func, p_mir_program p_program)
{
    p_ssa_var_list_info p_var_list = malloc(sizeof(*p_var_list));
    *p_var_list = (ssa_var_list_info){
        .global_num = list_head_alone(&p_program->p_store->variable)? 0 : list_entry(p_program->p_store->variable.p_prev, symbol_sym, node)->id + 1,
        .local_num = list_head_alone(&p_func->p_func_sym->variable)? 0 : list_entry(p_func->p_func_sym->variable.p_prev, symbol_sym, node)->id + 1,
        .temp_num = 1,
        .p_ret_sym = list_entry(p_func->temp_sym_head.p_prev, mir_temp_sym, node),
        .p_func = p_func,
    };

    size_t whole_num = p_var_list->temp_num + p_var_list->global_num + p_var_list->local_num;
    p_var_list->p_base = malloc(whole_num * sizeof(*p_var_list->p_base));
    p_list_head p_node;
    // 全局变量, 已初始化
    list_for_each(p_node, &p_program->p_store->variable){
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        *(p_var_list->p_base + p_sym->id) = (ssa_var_info) {
            .p_mem_sym = p_sym,
            .p_current_sym = NULL,
            .sym_stack = list_head_init(&(p_var_list->p_base + p_sym->id)->sym_stack),
        };
    }

    p_symbol_type p_param_type = p_func->p_func_sym->p_type->p_params;
    // 函数参数定值已经初始化
    list_for_each(p_node, &p_func->p_func_sym->variable) {
        if (!p_param_type) break;
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        *(p_var_list->p_base + p_sym->id + p_var_list->global_num) = (ssa_var_info) {
            .p_mem_sym = p_sym,
            .p_current_sym = NULL,
            .sym_stack = list_head_init(&(p_var_list->p_base + p_sym->id)->sym_stack),
        };
        p_param_type = p_param_type->p_params;
    }
    // 局部变量
    while(p_node != &p_func->p_func_sym->variable) {
        p_symbol_sym p_sym = list_entry(p_node, symbol_sym, node);
        *(p_var_list->p_base + p_sym->id + p_var_list->global_num) = (ssa_var_info) {
            .p_mem_sym = p_sym,
            .p_current_sym = NULL,
            .sym_stack = list_head_init(&(p_var_list->p_base + p_sym->id)->sym_stack),
        };
        p_node = p_node->p_next;
    }
    // 返回值
    *(p_var_list->p_base + whole_num - 1) = (ssa_var_info) {
        .p_mem_sym = NULL,
        .p_current_sym = NULL,
        .sym_stack = list_head_init(&(p_var_list->p_base + whole_num - 1)->sym_stack),
    };
    return p_var_list;
}

// 将变量转换到对应的标号，若不存在标号返回 -1
static inline size_t get_var_index(p_mir_operand p_operand, p_ssa_var_list_info p_var_list) {
    if(!p_operand) return -1;
    if (p_operand->kind == mem)
        if (p_operand->p_sym->is_global)
            return p_operand->p_sym->id;
        else
            return p_operand->p_sym->id + p_var_list->global_num;
    else if (p_operand->kind == reg && p_operand->p_temp_sym == p_var_list->p_ret_sym)
        return p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num - 1;
    else
        return -1;
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

void convert_ssa_insert_phi(p_convert_ssa dfs_seq, size_t block_num, p_ssa_var_list_info p_var_list)
{
    // 入口块对所有变量已经定值
    bitmap_set_full(dfs_seq->p_def_var);
    size_t work_num = block_num + 1;
    // 记录原来的集合
    p_bitmap p_old = bitmap_gen(p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num);
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
            size_t id = get_var_index(p_operand, p_var_list);
            if(id != -1)
                bitmap_add_element(p_info->p_def_var, id);
        }
        // 遍历支配边界将定值集合并入到 边界的 phi 集合
        for(size_t j = 0; j < block_num; j ++){
            if(bitmap_if_in(p_info->dom_frontier, j))
            {   // 若 phi 集合发生变化且已经遍历过且没有被加入过工作集合需要加入到工作集合之后处理
                bitmap_copy_not_new(p_old, (dfs_seq + j)->p_phi_var);
                bitmap_merge_not_new((dfs_seq + j)->p_phi_var, p_info->p_def_var);
                bool if_change = bitmap_if_equal(p_old, (dfs_seq + j)->p_phi_var);
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

static inline p_mir_temp_sym new_temp_var(p_ssa_var_list_info p_var_list, size_t index, p_mir_basic_block p_current_block, p_mir_instr p_instr)
{
    p_ssa_var_info p_info = p_var_list->p_base + index;    
    p_mir_temp_sym p_new_sym ;
    if(p_info->p_mem_sym)
        p_new_sym = mir_temp_sym_basic_gen(p_info->p_mem_sym->p_type->basic);
    else
        p_new_sym = mir_temp_sym_basic_gen(p_var_list->p_ret_sym->b_type);
    mir_func_temp_sym_add_at(p_var_list->p_func, p_new_sym, p_current_block, p_instr);
    p_info->p_current_sym = p_new_sym;
    return p_new_sym;
}

static inline p_mir_temp_sym get_top_sym(p_ssa_var_list_info p_var_list, size_t index, p_mir_instr p_instr, p_mir_basic_block p_basic_block)
{
    p_ssa_var_info p_info = p_var_list ->p_base + index;
    if(!p_info->p_current_sym)
        p_info->p_current_sym = new_temp_var(p_var_list, index, p_basic_block, p_instr);
    return p_info->p_current_sym;
}

static inline void set_src_ssa_id(p_mir_instr p_instr, p_ssa_var_list_info p_var_list, p_mir_basic_block p_basic_block)
{
    p_mir_operand p_src1 = mir_instr_get_src1(p_instr);
    p_mir_operand p_src2 = mir_instr_get_src2(p_instr);

    size_t index1 = get_var_index(p_src1, p_var_list);
    size_t index2 = get_var_index(p_src2, p_var_list);
    if(index1 != -1){
        *p_src1 = (mir_operand){
            .kind = reg,
            .p_temp_sym = get_top_sym(p_var_list, index1, p_instr, p_basic_block),
        };
    }

    if (index2 != -1) {
        *p_src2 = (mir_operand) {
            .kind = reg,
            .p_temp_sym = get_top_sym(p_var_list, index2, p_instr, p_basic_block),
        };
    }
}

static inline void set_des_ssa_id(p_mir_instr p_instr, p_ssa_var_list_info p_var_list, p_mir_basic_block p_basic_block) {
    p_mir_operand p_operand = mir_instr_get_des(p_instr);
    size_t index = get_var_index(p_operand, p_var_list);
    if (index != -1){
        *p_operand = (mir_operand){
            .kind = reg,
            .p_temp_sym = new_temp_var(p_var_list, index, p_basic_block, p_instr),
        };
    }
}

static inline void set_block_call_ssa_id(p_mir_basic_block_call p_block_call, p_convert_ssa dfs_seq, p_ssa_var_list_info p_var_list, p_mir_instr p_instr, p_mir_basic_block p_basic_block) 
{
    size_t var_num = p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num;
    p_bitmap p_phi_var = (dfs_seq + p_block_call->p_block->dfn_id)->p_phi_var;
    for(size_t i = 0; i < var_num; i ++){
        if(bitmap_if_in(p_phi_var, i))
        {
            p_mir_operand p_param = mir_operand_temp_sym_gen(get_top_sym(p_var_list, i, p_instr, p_basic_block));
            mir_basic_block_call_add_param(p_block_call, p_param);
        }
    }
}

// 为基本块创建 phi 参数列表并命名
static inline void set_block_param_ssa_id(p_mir_basic_block p_basic_block, p_convert_ssa dfs_seq, p_ssa_var_list_info p_var_list)
{
    size_t var_num = p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num;
    p_bitmap p_phi_var = (dfs_seq + p_basic_block->dfn_id)->p_phi_var;
    for (size_t i = 0; i < var_num; i++) {
        if (bitmap_if_in(p_phi_var, i)) {
            p_mir_operand p_param = mir_operand_temp_sym_gen(new_temp_var(p_var_list, i, p_basic_block, list_entry(&p_basic_block->instr_list, mir_instr, node)));
            mir_basic_block_add_param(p_basic_block, p_param);
        }
    }
}

static inline void record_current_sym(p_ssa_var_list_info p_var_list)
{
    size_t whole_num = p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num;
    for (size_t i = 0; i < whole_num; i ++) {
        p_ssa_var_info p_info = p_var_list->p_base + i;
        p_sym_stack_node p_node = malloc(sizeof(*p_node));
        *p_node = (sym_stack_node){
            .node = list_head_init(&p_node->node),
            .p_temp_sym = p_info->p_current_sym,
        };
        list_add_prev(&p_node->node, &p_info->sym_stack);
    }
}

static inline void restore_current_sym(p_ssa_var_list_info p_var_list)
{
    size_t whole_num = p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num;
    for (size_t i = 0; i < whole_num; i++) {
        p_ssa_var_info p_info = p_var_list->p_base + i;
        p_sym_stack_node p_node = list_entry(p_info->sym_stack.p_prev, sym_stack_node, node);
        p_info->p_current_sym = p_node->p_temp_sym;
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
    list_for_each(p_node, &p_entry->instr_list){
        p_mir_instr p_instr = list_entry(p_node, mir_instr, node);

        if (p_instr->irkind == mir_br)
        {
            set_block_call_ssa_id(p_instr->mir_br.p_target, dfs_seq, p_var_list, p_instr, p_entry);
            convert_ssa_rename_var(p_var_list, dfs_seq, p_instr->mir_br.p_target->p_block);
            continue;
        }
        if (p_instr->irkind == mir_condbr)
        {
            set_block_call_ssa_id(p_instr->mir_condbr.p_target_true, dfs_seq, p_var_list, p_instr, p_entry);
            convert_ssa_rename_var(p_var_list, dfs_seq, p_instr->mir_condbr.p_target_true->p_block);
            set_block_call_ssa_id(p_instr->mir_condbr.p_target_false, dfs_seq, p_var_list, p_instr, p_entry);
            convert_ssa_rename_var(p_var_list, dfs_seq, p_instr->mir_condbr.p_target_false->p_block);
            continue;
        }
        

        set_src_ssa_id(p_instr, p_var_list, p_entry);

        set_des_ssa_id(p_instr, p_var_list, p_entry);
    }
    // 恢复信息 
    restore_current_sym(p_var_list);
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


void convert_ssa_func(p_mir_func p_func, p_mir_program p_program){
    if (list_head_alone(&p_func->entry_block)) return;
    size_t block_num = list_entry(p_func->entry_block.p_prev, mir_basic_block, node)->block_id + 1;
    p_convert_ssa dfs_seq = malloc(block_num * sizeof(*dfs_seq));
    // 初始化变量集合
    p_ssa_var_list_info p_var_list = convert_ssa_init_var_list(p_func, p_program);
    size_t whole_num = p_var_list->global_num + p_var_list->local_num + p_var_list->temp_num;
    // 初始化 dfs 序
    mir_basic_block_init_visited(p_func);
    p_mir_basic_block p_entry = list_entry(p_func->entry_block.p_next, mir_basic_block, node);
    convert_ssa_init_dfs_sequence(dfs_seq, block_num, whole_num, p_entry, 0);
    // 计算支配树
    mir_cfg_set_func_dom(p_func);
    // 计算支配边界
    convert_ssa_compute_dom_frontier(dfs_seq, block_num);
    print_dom_frontier(dfs_seq, block_num);
    // 插入 phi 函数
    convert_ssa_insert_phi(dfs_seq, block_num, p_var_list);
    // 重命名
    mir_basic_block_init_visited(p_func);
    convert_ssa_rename_var(p_var_list, dfs_seq, p_entry);

    convert_ssa_dfs_seq_drop(dfs_seq, block_num);
    ssa_var_list_info_drop(p_var_list);

    mir_func_set_temp_id(p_func);
}

void convert_ssa_program(p_mir_program p_program){
    for (size_t i = 0; i < p_program->func_cnt; i++)
        convert_ssa_func(p_program->func_table + i, p_program);
}

void convert_ssa_dfs_seq_drop(convert_ssa *dfs_seq, size_t block_num) {
    for (size_t i = 0; i < block_num; i++){
        bitmap_drop((dfs_seq + i)->dom_frontier);
        bitmap_drop((dfs_seq + i)->p_def_var);
        bitmap_drop((dfs_seq + i)->p_phi_var);
    }
    free(dfs_seq);
}

void ssa_var_list_info_drop(p_ssa_var_list_info p_var_list)
{
    free(p_var_list->p_base);
    free(p_var_list);
}