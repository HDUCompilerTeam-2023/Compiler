#include <util.h>

#include <ir.h>
#include <ir/basic_block.h>
#include <stdio.h>

#include <ir/bb_param.h>
#include <ir/instr.h>
#include <ir/operand.h>
#include <ir/param.h>
#include <ir/vreg.h>
#include <symbol/func.h>
#include <symbol/var.h>

#include <ir_print.h>

#include <symbol/type.h>

#include <ir_opt/loop_opt/var_analysis.h>

void program_var_analysis(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        if (list_head_alone(&p_func->block)) continue;
        loop_var_analysis(p_func->p_nestedtree_root);
    }
}

bool check_operand(p_ir_operand p_operand) {
    switch (p_operand->kind) {
    case imme:
        if (p_operand->p_type->ref_level > 0) {
            return 0;
        }
        return 1;
    case reg:
        if (p_operand->p_vreg->if_loop_inv)
            return 1;
        return 0;
    }
}

void invariant_analysis(p_nestedtree_node root) {
    p_list_head p_node;
    if (!root->head) return;
    printf("head: %ld\n", root->head->block_id);
    list_for_each(p_node, &root->head->basic_block_phis) {
        list_entry(p_node, ir_bb_phi, node)->p_bb_phi->if_loop_inv = false;
    }
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_binary:
            if (!check_operand(p_instr->ir_binary.p_src1) || !check_operand(p_instr->ir_binary.p_src2))
                p_instr->ir_binary.p_des->if_loop_inv = false;
            break;
        case ir_unary:
            if (!check_operand(p_instr->ir_unary.p_src))
                p_instr->ir_unary.p_des->if_loop_inv = false;
            break;
        case ir_load:
            p_instr->ir_load.p_des->if_loop_inv = false;
            break;
        case ir_gep:
            p_instr->ir_gep.p_des->if_loop_inv = false;
            break;
        case ir_call:
            if (p_instr->ir_call.p_func->ret_type != type_void)
                p_instr->ir_call.p_des->if_loop_inv = false;
            break;
        default:
            break;
        }
    }
    list_for_each(p_node, &root->tail_list) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
        p_list_head p_list_node;
        list_for_each(p_list_node, &p_basic_block->basic_block_phis) {
            list_entry(p_list_node, ir_bb_phi, node)->p_bb_phi->if_loop_inv = false;
        }
        list_for_each(p_list_node, &p_basic_block->instr_list) {
            p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
            switch (p_instr->irkind) {
            case ir_binary:
                if (!check_operand(p_instr->ir_binary.p_src1) || !check_operand(p_instr->ir_binary.p_src2))
                    p_instr->ir_binary.p_des->if_loop_inv = false;
                break;
            case ir_unary:
                if (!check_operand(p_instr->ir_unary.p_src))
                    p_instr->ir_unary.p_des->if_loop_inv = false;
                break;
            case ir_load:
                p_instr->ir_load.p_des->if_loop_inv = false;
                break;
            case ir_gep:
                p_instr->ir_gep.p_des->if_loop_inv = false;
                break;
            case ir_call:
                if (p_instr->ir_call.p_func->ret_type != type_void)
                    p_instr->ir_call.p_des->if_loop_inv = false;
                break;
            default:
                break;
            }
        }
    }
    list_for_each(p_node, &root->head->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_binary:
            if (p_instr->ir_binary.p_des->if_loop_inv)
                printf("%ld\n", p_instr->ir_binary.p_des->id);
            break;
        case ir_unary:
            if (p_instr->ir_unary.p_des->if_loop_inv)
                printf("%ld\n", p_instr->ir_unary.p_des->id);
        default:
            break;
        }
    }
    if (!list_head_alone(&root->tail_list)) {
        list_for_each(p_node, &root->tail_list) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block_list_node, node)->p_basic_block;
            printf("tail: %ld\n", p_basic_block->block_id);
            p_list_head p_list_node;
            list_for_each(p_list_node, &p_basic_block->instr_list) {
                p_ir_instr p_instr = list_entry(p_list_node, ir_instr, node);
                switch (p_instr->irkind) {
                case ir_binary:
                    if (p_instr->ir_binary.p_des->if_loop_inv)
                        printf("%ld\n", p_instr->ir_binary.p_des->id);
                    break;
                case ir_unary:
                    if (p_instr->ir_unary.p_des->if_loop_inv)
                        printf("%ld\n", p_instr->ir_unary.p_des->id);
                default:
                    break;
                }
            }
        }
    }
}

void loop_var_analysis(p_nestedtree_node root) {
    p_list_head p_node;
    list_for_each(p_node, &root->son_list) {
        p_nestedtree_node p_list_node = list_entry(p_node, nested_list_node, node)->p_nested_node;
        loop_var_analysis(p_list_node);
    }
    // invariant analysis
    invariant_analysis(root);
}