#include <backend/arm/arm_struct_gen.h>
#include <stdio.h>
size_t R_NUM = 16;
size_t S_NUM = 32;

arm_label arm_label_gen(char *label) {
    arm_label la = malloc(strlen(label) + 1);
    sprintf(la, "%s", label);
    return la;
}

p_arm_func arm_func_gen(char *func_name) {
    p_arm_func p_func = malloc(sizeof(*p_func));
    p_func->func_name = func_name;
    p_func->block_list = list_head_init(&p_func->block_list);
    p_func->p_into_func_block = arm_block_gen(arm_label_gen(func_name));
    p_func->node = list_head_init(&p_func->node);
    return p_func;
}

p_arm_block arm_block_gen(arm_label label) {
    p_arm_block p_block = malloc(sizeof(*p_block));
    p_block->instr_list = list_head_init(&p_block->instr_list);
    p_block->label = label;
    p_block->node = list_head_init(&p_block->node);
    p_block->p_target1 = NULL;
    p_block->p_target2 = NULL;
    p_block->arm_block_prevs = list_head_init(&p_block->arm_block_prevs);
    return p_block;
}
void arm_func_add_block_tail(p_arm_func p_func, p_arm_block p_block) {
    assert(list_add_prev(&p_block->node, &p_func->block_list));
}
void arm_block_add_instr_tail(p_arm_block p_block, p_arm_instr p_instr) {
    assert(list_add_prev(&p_instr->node, &p_block->instr_list));
}
p_arm_operand arm_operand_reg_gen(arm_reg arm_reg) {
    p_arm_operand p_operand = malloc(sizeof(*p_operand));
    p_operand->if_imme = false;
    p_operand->reg = arm_reg;
    p_operand->lsl_imme = 0;
    return p_operand;
}

p_arm_operand arm_operand_imme_gen(arm_imme imme) {
    p_arm_operand p_operand = malloc(sizeof(*p_operand));
    p_operand->if_imme = true;
    p_operand->imme = imme;
    return p_operand;
}

p_arm_instr arm_binary_instr_gen(arm_binary_op op, arm_reg rd, arm_reg rs1, p_arm_operand op2) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_binary_type;
    p_instr->binary_instr.op = op;
    p_instr->binary_instr.cond_type = arm_al;
    p_instr->binary_instr.rd = rd;
    p_instr->binary_instr.rs1 = rs1;
    p_instr->binary_instr.op2 = op2;
    p_instr->binary_instr.s = false;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_cmp_instr_gen(arm_cmp_op op, arm_reg rs1, p_arm_operand op2) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_cmp_type;
    p_instr->cmp_instr.op = op;
    p_instr->cmp_instr.rs1 = rs1;
    p_instr->cmp_instr.op2 = op2;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_jump_instr_gen(arm_jump_op op, arm_cond_type cond_type, p_arm_block p_sour_block, p_arm_block p_target) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_jump_type;
    p_instr->jump_instr.op = op;
    p_instr->cond_type = cond_type;
    p_instr->jump_instr.p_block_target = p_target;
    p_instr->jump_instr.p_source_block = p_sour_block;
    return p_instr;
}
void arm_block_add_prev(p_arm_block p_source, p_arm_instr p_instr) {
    assert(p_source == p_instr->jump_instr.p_source_block);
    p_arm_block_edge_node p_a_block_node = malloc(sizeof(*p_a_block_node));
    p_a_block_node->node = list_head_init(&p_a_block_node->node);
    p_a_block_node->p_jump_instr = p_instr;
    list_add_prev(&p_a_block_node->node, &p_instr->jump_instr.p_block_target->arm_block_prevs);
}
void arm_block_del_prev(p_arm_instr p_jump) {
    p_list_head p_node;
    list_for_each(p_node, &p_jump->jump_instr.p_block_target->arm_block_prevs) {
        p_arm_block_edge_node p_edge_node = list_entry(p_node, arm_block_edge_node, node);
        if (p_edge_node->p_jump_instr == p_jump) {
            list_del(&p_edge_node->node);
            free(p_edge_node);
            break;
        }
    }
}

void arm_jump_instr_reset_target(p_arm_instr p_jump, p_arm_block p_target) {
    if (p_jump->jump_instr.p_block_target == p_target)
        return;
    assert(p_jump->jump_instr.p_block_target);
    arm_block_del_prev(p_jump);
    p_jump->jump_instr.p_block_target = p_target;
    arm_block_add_prev(p_jump->jump_instr.p_source_block, p_jump);
}
void arm_block_set_br1(p_arm_block p_a_block, p_arm_instr p_instr) {
    assert(p_instr->type == arm_jump_type);
    p_a_block->p_target1 = p_instr;
    arm_block_add_prev(p_a_block, p_instr);
}
void arm_block_set_br2(p_arm_block p_a_block, p_arm_instr p_instr) {
    assert(p_instr->type == arm_jump_type);
    p_a_block->p_target2 = p_instr;
    arm_block_add_prev(p_a_block, p_instr);
}
p_arm_instr arm_call_instr_gen(char *name) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->cond_type = arm_al;
    p_instr->type = arm_call_type;
    p_instr->call_instr.func_name = name;
    return p_instr;
}
p_arm_instr arm_mem_instr_gen(arm_mem_op op, arm_reg rd, arm_reg base, p_arm_operand offset) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_mem_type;
    p_instr->mem_instr.op = op;
    p_instr->mem_instr.rd = rd;
    p_instr->mem_instr.base = base;
    p_instr->mem_instr.offset = offset;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_mov_instr_gen(arm_mov_op op, arm_cond_type cond, arm_reg rd, p_arm_operand op1) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_mov_type;
    p_instr->mov_instr.op = op;
    p_instr->mov_instr.s = false;
    p_instr->mov_instr.rd = rd;
    p_instr->mov_instr.operand = op1;
    p_instr->cond_type = cond;
    return p_instr;
}

p_arm_instr arm_mov32_instr_gen(arm_reg rd, arm_label label, arm_imme imme) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_mov32_type;
    p_instr->mov32_instr.label = label;
    p_instr->mov32_instr.imme = imme;
    p_instr->mov32_instr.rd = rd;
    p_instr->cond_type = arm_al;
    return p_instr;
}

p_arm_instr arm_mul_instr_gen(arm_reg rd, arm_reg rs1, arm_reg rs2) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_mul_type;
    p_instr->mul_instr.rd = rd;
    p_instr->mul_instr.rs1 = rs1;
    p_instr->mul_instr.rs2 = rs2;
    p_instr->cond_type = arm_al;
    return p_instr;
}

p_arm_instr arm_sdiv_instr_gen(arm_reg rd, arm_reg rs1, arm_reg rs2) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->type = arm_sdiv_type;
    p_instr->sdiv_instr.rd = rd;
    p_instr->sdiv_instr.rs1 = rs1;
    p_instr->sdiv_instr.rs2 = rs2;
    p_instr->cond_type = arm_al;
    return p_instr;
}

p_arm_instr arm_push_pop_instr_gen(arm_push_pop_op op) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_push_pop_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->push_pop_instr.op = op;
    p_instr->push_pop_instr.num = 0;
    p_instr->push_pop_instr.regs = malloc(sizeof(void *) * R_NUM);
    p_instr->cond_type = arm_al;
    return p_instr;
}
void arm_push_pop_instr_add(p_arm_push_pop_instr p_instr, arm_reg reg) {
    assert(p_instr->num < 16);
    p_instr->regs[p_instr->num++] = reg;
}
void arm_push_pop_instr_init(p_arm_push_pop_instr p_instr, arm_reg *reg, size_t num) {
    for (size_t i = 0; i < num; i++)
        arm_push_pop_instr_add(p_instr, reg[i]);
}
p_arm_instr arm_vbinary_instr_gen(arm_vbinary_op op, arm_reg rd, arm_reg rs1, arm_reg rs2) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vbinary_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vbinary_instr.op = op;
    p_instr->vbinary_instr.rd = rd;
    p_instr->vbinary_instr.rs1 = rs1;
    p_instr->vbinary_instr.rs2 = rs2;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_vmov_instr_gen(arm_reg rd, arm_reg rs) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vmov_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vmov_instr.rd = rd;
    p_instr->vmov_instr.rs = rs;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_vcmp_instr_gen(arm_vcmp_op op, arm_reg rs1, arm_reg rs2) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vcmp_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vcmp_instr.op = op;
    p_instr->vcmp_instr.rs1 = rs1;
    p_instr->vcmp_instr.rs2 = rs2;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_vcvt_instr_gen(arm_vcvt_op op, arm_reg rd, arm_reg rs) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vcvt_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vcvt_instr.op = op;
    p_instr->vcvt_instr.rd = rd;
    p_instr->vcvt_instr.rs = rs;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_vmem_instr_gen(arm_vmem_op op, arm_reg rd, arm_reg base, arm_imme offset) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vmem_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vmem_instr.op = op;
    p_instr->vmem_instr.rd = rd;
    p_instr->vmem_instr.base = base;
    p_instr->vmem_instr.offset = offset;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_vneg_instr_gen(arm_reg rd, arm_reg rs) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vneg_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vneg_instr.rd = rd;
    p_instr->vneg_instr.rs = rs;
    p_instr->cond_type = arm_al;
    return p_instr;
}
p_arm_instr arm_vpush_vpop_instr_gen(arm_vpush_vpop_op op) {
    p_arm_instr p_instr = malloc(sizeof(*p_instr));
    p_instr->type = arm_vpush_vpop_type;
    p_instr->node = list_head_init(&p_instr->node);
    p_instr->vpush_vpop_instr.op = op;
    p_instr->vpush_vpop_instr.num = 0;
    p_instr->vpush_vpop_instr.regs = malloc(sizeof(void *) * S_NUM);
    p_instr->cond_type = arm_al;
    return p_instr;
}
void arm_vpush_vpop_instr_add(p_arm_vpush_vpop_instr p_instr, arm_reg reg) {
    assert(p_instr->num < 16);
    p_instr->regs[p_instr->num++] = reg;
}
void arm_vpush_vpop_instr_init(p_arm_vpush_vpop_instr p_instr, arm_reg *reg, size_t num) {
    for (size_t i = 0; i < num; i++)
        arm_vpush_vpop_instr_add(p_instr, reg[i]);
}
static inline void arm_label_drop(arm_label label) {
    free(label);
}
static inline void arm_operand_drop(p_arm_operand p_operand) {
    free(p_operand);
}

void arm_instr_drop(p_arm_instr p_instr) {
    list_del(&p_instr->node);
    switch (p_instr->type) {
    case arm_binary_type:
        arm_operand_drop(p_instr->binary_instr.op2);
        break;
    case arm_jump_type:
        break;
    case arm_call_type:
        break;
    case arm_cmp_type:
        arm_operand_drop(p_instr->cmp_instr.op2);
        break;
    case arm_mem_type:
        arm_operand_drop(p_instr->mem_instr.offset);
        break;
    case arm_mov_type:
        arm_operand_drop(p_instr->mov_instr.operand);
        break;
    case arm_mov32_type:
        arm_label_drop(p_instr->mov32_instr.label);
        break;
    case arm_mul_type:
        break;
    case arm_sdiv_type:
        break;
    case arm_push_pop_type:
        free(p_instr->push_pop_instr.regs);
        break;
    case arm_vbinary_type:
        break;
    case arm_vmov_type:
        break;
    case arm_vmem_type:
        break;
    case arm_vcvt_type:
        break;
    case arm_vneg_type:
        break;
    case arm_vcmp_type:
        break;
    case arm_vpush_vpop_type:
        free(p_instr->vpush_vpop_instr.regs);
        break;
    }
    free(p_instr);
}
void arm_block_drop(p_arm_block p_a_block) {
    list_del(&p_a_block->node);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_a_block->arm_block_prevs) {
        p_arm_block_edge_node p_edge_node = list_entry(p_node, arm_block_edge_node, node);
        free(p_edge_node);
    }
    list_for_each_safe(p_node, p_next, &p_a_block->instr_list) {
        arm_instr_drop(list_entry(p_node, arm_instr, node));
    }
    if (p_a_block->p_target1)
        arm_instr_drop(p_a_block->p_target1);
    if (p_a_block->p_target2)
        arm_instr_drop(p_a_block->p_target2);
    arm_label_drop(p_a_block->label);
    free(p_a_block);
}
void arm_func_drop(p_arm_func p_func) {
    list_del(&p_func->node);
    p_list_head p_node, p_next;
    arm_block_drop(p_func->p_into_func_block);
    list_for_each_safe(p_node, p_next, &p_func->block_list) {
        arm_block_drop(list_entry(p_node, arm_block, node));
    }
    free(p_func);
}
arm_label arm_block_label_gen(char *func, size_t id) {
    char *extra = malloc(strlen(func) + id + 5);
    sprintf(extra, ".%s_b%ld", func, id);
    return extra;
}
arm_cond_type get_opposite_type(arm_cond_type type) {
    switch (type) {
    case arm_eq:
        return arm_ne;
    case arm_ne:
        return arm_eq;
    case arm_ge:
        return arm_lt;
    case arm_gt:
        return arm_le;
    case arm_le:
        return arm_gt;
    case arm_lt:
        return arm_ge;
    case arm_al:
        return arm_al;
    }
}
bool if_float_reg(arm_reg reg) {
    return reg >= R_NUM;
}
p_arm_operand arm_operand_copy(p_arm_operand p_operand) {
    if (p_operand->if_imme) {
        return arm_operand_imme_gen(p_operand->imme);
    }
    else {
        return arm_operand_reg_gen(p_operand->reg);
    }
}
p_arm_instr arm_instr_copy(p_arm_instr p_src) {
    switch (p_src->type) {
    case arm_binary_type:
        return arm_binary_instr_gen(p_src->binary_instr.op, p_src->binary_instr.rd, p_src->binary_instr.rs1, arm_operand_copy(p_src->binary_instr.op2));
    case arm_jump_type:
        return arm_jump_instr_gen(p_src->jump_instr.op, p_src->cond_type, p_src->jump_instr.p_source_block, p_src->jump_instr.p_block_target);
    case arm_call_type:
        return arm_call_instr_gen(p_src->call_instr.func_name);
    case arm_cmp_type:
        return arm_cmp_instr_gen(p_src->cmp_instr.op, p_src->cmp_instr.rs1, arm_operand_copy(p_src->cmp_instr.op2));
    case arm_mem_type:
        return arm_mem_instr_gen(p_src->mem_instr.op, p_src->mem_instr.rd, p_src->mem_instr.base, arm_operand_copy(p_src->mem_instr.offset));
    case arm_mov_type:
        return arm_mov_instr_gen(p_src->mov_instr.op, p_src->cond_type, p_src->mov_instr.rd, arm_operand_copy(p_src->mov_instr.operand));
    case arm_mov32_type:
        return arm_mov32_instr_gen(p_src->mov32_instr.rd, p_src->mov32_instr.label, p_src->mov32_instr.imme);
    case arm_mul_type:
        return arm_mul_instr_gen(p_src->mul_instr.rd, p_src->mul_instr.rs1, p_src->mul_instr.rs2);
    case arm_sdiv_type:
        return arm_sdiv_instr_gen(p_src->sdiv_instr.rd, p_src->sdiv_instr.rs1, p_src->sdiv_instr.rs2);
    case arm_push_pop_type:
        assert(0);
        break;
    case arm_vbinary_type:
        return arm_vbinary_instr_gen(p_src->vbinary_instr.op, p_src->vbinary_instr.rd, p_src->vbinary_instr.rs1, p_src->vbinary_instr.rs2);
    case arm_vmov_type:
        return arm_vmov_instr_gen(p_src->vmov_instr.rd, p_src->vmov_instr.rs);
    case arm_vmem_type:
        return arm_vmem_instr_gen(p_src->vmem_instr.op, p_src->vmem_instr.rd, p_src->vmem_instr.base, p_src->vmem_instr.offset);
    case arm_vcvt_type:
        return arm_vcvt_instr_gen(p_src->vcvt_instr.op, p_src->vcvt_instr.rd, p_src->vcvt_instr.rs);
    case arm_vneg_type:
        return arm_vneg_instr_gen(p_src->vneg_instr.rd, p_src->vneg_instr.rs);
    case arm_vcmp_type:
        return arm_vcmp_instr_gen(p_src->vcmp_instr.op, p_src->vcmp_instr.rs1, p_src->vcmp_instr.rs2);
    case arm_vpush_vpop_type:
        assert(0);
        break;
    }
}