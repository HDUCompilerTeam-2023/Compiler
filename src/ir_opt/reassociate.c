#include <program/use.h>
#include <symbol_gen/func.h>
#include <symbol_gen/type.h>
#include <ir_print/operand.h>
#include <ir_gen/operand.h>
#include <ir_gen/instr.h>
#include <ir_gen/vreg.h>
#include <ir_gen/basic_block.h>
#include <ir_opt/const_fold.h>

typedef struct {
    p_ir_operand p_operand;
    I32CONST_t cnt;
    list_head node;
} expr_mem, *p_expr_mem;

static inline p_expr_mem _expr_mem_gen(p_ir_operand p_operand) {
    p_expr_mem p_mem = malloc(sizeof(*p_mem));
    *p_mem = (expr_mem) {
        .p_operand = ir_operand_copy(p_operand),
        .cnt = 0,
        .node = list_init_head(&p_mem->node),
    };
    return p_mem;
}

typedef enum {
    add = 0,
    mul,
} expr_kind;

typedef struct {
    expr_kind kind;
    p_ir_vreg p_root;
    p_ir_operand p_const;
    list_head members;
} expr_info, *p_expr_info;

typedef struct {
    enum {
        root,
        temp,
    } kind;
    p_ir_vreg p_vreg;
    p_expr_info p_expr;
    p_ir_instr p_instr;
} vreg_info, *p_vreg_info;

static inline void _expr_del_mem(p_expr_mem p_mem) {
    list_del(&p_mem->node);
    ir_operand_drop(p_mem->p_operand);
    free(p_mem);
}

static inline void _expr_add_mem(p_expr_info p_expr, p_ir_operand p_operand, I32CONST_t cnt) {
    if (p_operand->kind == imme) {
        assert(p_operand->p_type->basic == type_i32 || p_operand->p_type->ref_level);
        if (p_expr->kind == add) {
            if (p_operand->p_type->ref_level) {
                assert(cnt == 1);
                assert(!p_expr->p_const->p_type->ref_level);
                p_operand = ir_operand_copy(p_operand);
                p_operand->offset += p_expr->p_const->i32const;
                p_ir_operand p_tmp = p_operand; p_operand = p_expr->p_const; p_expr->p_const = p_tmp;
                ir_operand_drop(p_operand);
                return;
            }

            if (p_expr->p_const->p_type->ref_level)
                p_expr->p_const->offset += p_operand->i32const * cnt;
            else
                p_expr->p_const->i32const += p_operand->i32const * cnt;
            return;
        }
        for (I32CONST_t i = 0; i < cnt; ++i) {
            p_expr->p_const->i32const *= p_operand->i32const;
        }
        return;
    }
    p_expr_mem p_mem = NULL;
    p_list_head p_node;
    list_for_each(p_node, &p_expr->members) {
        assert(p_operand->kind == reg);
        p_expr_mem p_f_mem = list_entry(p_node, expr_mem, node);
        if (p_f_mem->p_operand->p_vreg == p_operand->p_vreg) {
            p_mem = p_f_mem;
            break;
        }
    }
    if (!p_mem) {
        p_mem = _expr_mem_gen(p_operand);
    }
    else {
        list_del(&p_mem->node);
    }
    list_add_prev(&p_mem->node, &p_expr->members);
    p_mem->cnt += cnt;

    if (!p_mem->cnt)
        _expr_del_mem(p_mem);
}

static inline void _expr_print(p_expr_info p_expr) {
    printf("%%%ld = ", p_expr->p_root->id);
    char op[2][2] = {{'*', '+'}, {'^', '*'}};
    p_list_head p_node;
    list_for_each(p_node, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_node, expr_mem, node);
        assert(p_expr->p_root->p_type->basic == p_mem->p_operand->p_type->basic);
        printf("(");
        ir_operand_print(p_mem->p_operand);
        printf(" %c %d) %c ", op[p_expr->kind][0], p_mem->cnt, op[p_expr->kind][1]);
    }
    printf("(");
    ir_operand_print(p_expr->p_const);
    printf(" %c 1)\n", op[p_expr->kind][0]);
}

static inline void _expr_gen(p_vreg_info p_info) {
    assert(p_info->kind == root);
    assert(!p_info->p_expr);

    if (p_info->p_vreg->p_type->basic == type_f32 && !p_info->p_vreg->p_type->ref_level)
        return;

    if (p_info->p_instr->irkind != ir_binary)
        return;

    expr_kind kind;
    if (p_info->p_instr->ir_binary.op == ir_add_op ||
            p_info->p_instr->ir_binary.op == ir_sub_op)
        kind = add;
    else if (p_info->p_instr->ir_binary.op == ir_mul_op)
        kind = mul;
    else
        return;

    I32CONST_t i32const = kind == add ? 0 : 1;
    p_expr_info p_expr = malloc(sizeof(*p_expr));
    *p_expr = (expr_info) {
        .kind = kind,
        .p_root = p_info->p_vreg,
        .p_const = ir_operand_int_gen(i32const),
        .members = list_init_head(&p_expr->members),
    };
    p_info->p_expr = p_expr;
}

static inline void _expr_drop(p_vreg_info p_info) {
    assert(p_info->kind == root);
    assert(p_info->p_expr);
    assert(!p_info->p_instr);

    p_expr_info p_expr = p_info->p_expr;

    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_node, expr_mem, node);
        _expr_del_mem(p_mem);
    }
    ir_operand_drop(p_expr->p_const);
    free(p_info->p_expr);
}

static inline void _collect_root(p_symbol_func p_func, p_vreg_info vreg_info_map) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        vreg_info_map[p_vreg->id].kind = root;
        vreg_info_map[p_vreg->id].p_vreg = p_vreg;
        vreg_info_map[p_vreg->id].p_expr = NULL;
        vreg_info_map[p_vreg->id].p_instr = NULL;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        vreg_info_map[p_vreg->id].kind = root;
        vreg_info_map[p_vreg->id].p_vreg = p_vreg;
        vreg_info_map[p_vreg->id].p_expr = NULL;
        vreg_info_map[p_vreg->id].p_instr = NULL;

        if (p_vreg->p_type->basic == type_f32 && !p_vreg->p_type->ref_level)
            continue;

        if (p_vreg->def_type != instr_def)
            continue;
        p_ir_instr p_instr_def = p_vreg->p_instr_def;
        vreg_info_map[p_vreg->id].p_instr = p_instr_def;
        if (p_instr_def->irkind != ir_binary)
            continue;

        if (p_vreg->use_list.p_next != p_vreg->use_list.p_prev)
            continue;
        if (list_head_alone(&p_vreg->use_list)) {
            ir_instr_drop(p_instr_def);
            vreg_info_map[p_vreg->id].p_instr = NULL;
            vreg_info_map[p_vreg->id].kind = temp;
            continue;
        }

        p_ir_operand p_use = list_entry(p_vreg->use_list.p_next, ir_operand, use_node);
        if (p_use->used_type != instr_ptr)
            continue;
        p_ir_instr p_instr_use = p_use->p_instr;
        if (p_instr_use->irkind != ir_binary)
            continue;

        if (p_instr_def->ir_binary.op == ir_add_op ||
                p_instr_def->ir_binary.op == ir_sub_op) {
            if (p_instr_use->ir_binary.op == ir_add_op ||
                    p_instr_use->ir_binary.op == ir_sub_op) {
                vreg_info_map[p_vreg->id].kind = temp;
                continue;
            }
        }
        if (p_instr_def->ir_binary.op == ir_mul_op) {
            if (p_instr_use->ir_binary.op == ir_mul_op) {
                vreg_info_map[p_vreg->id].kind = temp;
                continue;
            }
        }
    }
}

static inline void _create_expr_for_temp(p_vreg_info p_temp, p_expr_info p_expr, I32CONST_t cnt, p_vreg_info vreg_info_map) {
    assert(p_temp->p_instr);
    assert(p_temp->p_instr->irkind == ir_binary);
    assert(p_temp->p_instr->ir_binary.p_des == p_temp->p_vreg);

    if (p_expr->kind == add)
        assert(p_temp->p_instr->ir_binary.op == ir_add_op ||
                p_temp->p_instr->ir_binary.op == ir_sub_op);
    else
        assert(p_temp->p_instr->ir_binary.op == ir_mul_op);

    I32CONST_t second_cnt = cnt;

    if (p_temp->p_instr->ir_binary.op == ir_sub_op)
        second_cnt *= -1;

    p_ir_operand p_src1 = p_temp->p_instr->ir_binary.p_src1;
    if (p_src1->kind == reg && vreg_info_map[p_src1->p_vreg->id].kind == temp)
        _create_expr_for_temp(vreg_info_map + p_src1->p_vreg->id, p_expr, cnt, vreg_info_map);
    else
        _expr_add_mem(p_expr, p_src1, cnt);

    p_ir_operand p_src2 = p_temp->p_instr->ir_binary.p_src2;
    if (p_src2->kind == reg && vreg_info_map[p_src2->p_vreg->id].kind == temp)
        _create_expr_for_temp(vreg_info_map + p_src2->p_vreg->id, p_expr, second_cnt, vreg_info_map);
    else
        _expr_add_mem(p_expr, p_src2, second_cnt);

    ir_instr_drop(p_temp->p_instr);
    p_temp->p_instr = NULL;
}

static inline void _collect_expr_for_root(p_vreg_info p_root, p_vreg_info vreg_info_map) {
    assert(p_root->kind == root);
    if (!p_root->p_instr)
        return;

    _expr_gen(p_root);
    if (!p_root->p_expr)
        return;

    _create_expr_for_temp(p_root, p_root->p_expr, 1, vreg_info_map);
}

static inline void _substitute_into_expr(p_expr_info p_expr, p_vreg_info vreg_info_map) {
    p_list_head p_expr_mem_node, p_expr_mem_next;
    list_for_each_safe(p_expr_mem_node, p_expr_mem_next, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_expr_mem_node, expr_mem, node);
        if (p_mem->p_operand->kind == imme)
            continue;

        p_ir_vreg p_vreg = p_mem->p_operand->p_vreg;
        p_vreg_info p_info = vreg_info_map + p_vreg->id;
        assert(p_info->kind == root);
        assert(!p_info->p_expr || !p_info->p_instr);

        if (p_mem->cnt == 0) {
            _expr_del_mem(p_mem);

            if (list_head_alone(&p_vreg->use_list)) {
                if (p_info->p_expr)
                    _expr_drop(p_info);
                if (p_info->p_instr)
                    ir_instr_drop(p_info->p_instr);
                p_info->kind = temp;
                p_info->p_expr = NULL;
                p_info->p_instr = NULL;
            }
            continue;
        }

        if (p_info->p_instr) {
            p_ir_instr p_instr = p_info->p_instr;
            switch (p_instr->irkind) {
            case ir_binary:
                assert(p_instr->ir_binary.op != ir_add_op &&
                        p_instr->ir_binary.op != ir_sub_op &&
                        p_instr->ir_binary.op != ir_mul_op);
                break;
            case ir_unary:
                switch (p_instr->ir_unary.op) {
                case ir_val_assign:
                    _expr_add_mem(p_expr, p_instr->ir_unary.p_src, p_mem->cnt);
                    if (p_expr_mem_next != p_expr_mem_node->p_next) p_expr_mem_next = p_expr_mem_node->p_next;
                    _expr_del_mem(p_mem);
                    break;
                case ir_minus_op:
                    if (p_expr->kind == add) {
                        _expr_add_mem(p_expr, p_instr->ir_unary.p_src, -p_mem->cnt);
                        if (p_expr_mem_next != p_expr_mem_node->p_next) p_expr_mem_next = p_expr_mem_node->p_next;
                        _expr_del_mem(p_mem);
                        break;
                    }
                    if (p_mem->cnt & 1)
                        p_expr->p_const->i32const *= -1;
                    _expr_add_mem(p_expr, p_instr->ir_unary.p_src, p_mem->cnt);
                    if (p_expr_mem_next != p_expr_mem_node->p_next) p_expr_mem_next = p_expr_mem_node->p_next;
                    _expr_del_mem(p_mem);
                    break;
                case ir_f2i_op:
                case ir_i2f_op:
                case ir_ptr_add_sp:
                    break;
                }
            case ir_gep:
            case ir_call:
            case ir_load:
            case ir_store:
                break;
            }
            if (list_head_alone(&p_vreg->use_list)) {
                if (p_info->p_instr)
                    ir_instr_drop(p_info->p_instr);
                p_info->kind = temp;
                p_info->p_instr = NULL;
            }
            continue;
        }

        if (p_info->p_expr) {
            if (p_vreg->use_list.p_next != p_vreg->use_list.p_prev)
                continue;

            if (p_expr->kind == add && p_info->p_expr->kind == mul) {
                if (p_info->p_expr->p_const->i32const != 1) {
                    p_mem->cnt *= p_info->p_expr->p_const->i32const;
                    p_info->p_expr->p_const->i32const = 1;
                }
                if (list_head_alone(&p_info->p_expr->members)) {
                    _expr_add_mem(p_expr, p_info->p_expr->p_const, p_mem->cnt);
                }
                else if (p_info->p_expr->members.p_next == p_info->p_expr->members.p_prev) {
                    p_expr_mem p_mem_sub = list_entry(p_info->p_expr->members.p_next, expr_mem, node);
                    if (p_mem_sub->cnt == 1) {
                        printf("%%%ld %%%ld %d\n", p_expr->p_root->id, p_mem->p_operand->p_vreg->id, p_mem->cnt);
                        _expr_add_mem(p_expr, p_mem_sub->p_operand, p_mem->cnt);
                    }
                    else
                        continue;
                }
                else {
                    continue;
                }
                if (p_expr_mem_next != p_expr_mem_node->p_next) p_expr_mem_next = p_expr_mem_node->p_next;
                _expr_del_mem(p_mem);
                assert(list_head_alone(&p_vreg->use_list));
                _expr_drop(p_info);
                p_info->kind = temp;
                p_info->p_expr = NULL;
                continue;
            }
            if (p_expr->kind == mul && p_info->p_expr->kind == add) {
                assert(!p_info->p_vreg->p_type->ref_level);
                if (list_head_alone(&p_info->p_expr->members)) {
                    _expr_add_mem(p_expr, p_info->p_expr->p_const, p_mem->cnt);
                }
                else if (p_info->p_expr->p_const->i32const == 0 && p_info->p_expr->members.p_next == p_info->p_expr->members.p_prev) {
                    p_expr_mem p_mem_sub = list_entry(p_info->p_expr->members.p_next, expr_mem, node);
                    _expr_add_mem(p_expr, p_mem_sub->p_operand, p_mem->cnt);
                    p_expr->p_const->i32const *= p_mem_sub->cnt;
                }
                else {
                    continue;
                }
                if (p_expr_mem_next != p_expr_mem_node->p_next) p_expr_mem_next = p_expr_mem_node->p_next;
                _expr_del_mem(p_mem);
                assert(list_head_alone(&p_vreg->use_list));
                _expr_drop(p_info);
                p_info->kind = temp;
                p_info->p_expr = NULL;
                continue;
            }

            if (p_expr->kind != p_info->p_expr->kind)
                continue;
            _expr_add_mem(p_expr, p_info->p_expr->p_const, p_mem->cnt);
            p_list_head p_node;
            list_for_each(p_node, &p_info->p_expr->members) {
                p_expr_mem p_mem_sub = list_entry(p_node, expr_mem, node);
                _expr_add_mem(p_expr, p_mem_sub->p_operand, p_mem->cnt * p_mem_sub->cnt);
            }
            if (p_expr_mem_next != p_expr_mem_node->p_next) p_expr_mem_next = p_expr_mem_node->p_next;
            _expr_del_mem(p_mem);
            assert(list_head_alone(&p_vreg->use_list));
            _expr_drop(p_info);
            p_info->kind = temp;
            p_info->p_expr = NULL;
        }
    }
}

static inline void _reg_info_print(p_vreg_info vreg_info_map, size_t vreg_cnt) {
    for (size_t i = 0; i < vreg_cnt; ++i) {
        if (vreg_info_map[i].kind != root) {
            assert(!vreg_info_map[i].p_instr);
            assert(!vreg_info_map[i].p_expr);
            assert(list_head_alone(&vreg_info_map[i].p_vreg->use_list));
            continue;
        }
        printf("%%%ld is root\n", i);
        if (vreg_info_map[i].p_expr) {
            assert(!vreg_info_map[i].p_instr);
            _expr_print(vreg_info_map[i].p_expr);
        }
    }
}

static inline I32CONST_t _i32_abs(I32CONST_t i) {
    if (i < 0)
        return -i;
    return i;
}

static inline void _sort_add_expr(p_expr_info p_expr) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_node, expr_mem, node);
        list_del(&p_mem->node);
        p_list_head p_node;
        list_for_each(p_node, &p_expr->members) {
            if (p_node == p_next)
                break;
            p_expr_mem p_sorted = list_entry(p_node, expr_mem, node);
            if (_i32_abs(p_sorted->cnt) < _i32_abs(p_mem->cnt))
                continue;
            if (_i32_abs(p_sorted->cnt) > _i32_abs(p_mem->cnt))
                break;
            if (p_mem->cnt > 0)
                break;
            if (p_sorted->cnt < 0)
                break;
        }
        list_add_prev(&p_mem->node, p_node);
    }
}

static inline void _sort_expr(p_expr_info p_expr) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_node, expr_mem, node);
        list_del(&p_mem->node);
        p_list_head p_node;
        list_for_each(p_node, &p_expr->members) {
            if (p_node == p_next)
                break;
            p_expr_mem p_sorted = list_entry(p_node, expr_mem, node);
            if (p_sorted->cnt < p_mem->cnt)
                continue;
            break;
        }
        list_add_prev(&p_mem->node, p_node);
    }
}

static inline void _sort_all_expr(p_vreg_info vreg_info_map, size_t vreg_cnt) {
    for (size_t i = 0; i < vreg_cnt; ++i) {
        if (vreg_info_map[i].kind != root) {
            assert(!vreg_info_map[i].p_instr);
            assert(!vreg_info_map[i].p_expr);
            assert(list_head_alone(&vreg_info_map[i].p_vreg->use_list));
            continue;
        }
        if (vreg_info_map[i].p_expr) {
            assert(!vreg_info_map[i].p_instr);
            if (vreg_info_map[i].p_expr->kind == add)
                _sort_add_expr(vreg_info_map[i].p_expr);
            else
                _sort_expr(vreg_info_map[i].p_expr);
        }
    }
}

static inline p_ir_operand _create_binary(ir_binary_op b_op, p_ir_operand p_src1, p_ir_operand p_src2, p_symbol_func p_func) {
    assert(p_src1);
    assert(p_src2);
    if (p_src2->p_type->ref_level) {
        p_ir_operand p_tmp = p_src1; p_src1 = p_src2; p_src2 = p_tmp;
    }
    if (p_src1->p_type->ref_level) {
        assert(!p_src2->p_type->ref_level);
        assert(b_op == ir_add_op || b_op == ir_sub_op);
    }

    p_ir_vreg p_des = ir_vreg_gen(symbol_type_copy(p_src1->p_type));
    symbol_func_vreg_add(p_func, p_des);
    p_ir_instr p_instr = ir_binary_instr_gen(b_op, p_src1, p_src2, p_des);
    ir_basic_block_addinstr_tail(p_func->p_entry_block, p_instr);
    return ir_operand_vreg_gen(p_des);
}

static inline void _rewrite_add_expr(p_expr_info p_expr, p_symbol_func p_func) {
    p_ir_operand p_val = NULL;

    p_ir_operand p_sim = NULL;
    I32CONST_t sim_cnt = 0;

    p_list_head p_node;
    list_for_each(p_node, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_node, expr_mem, node);
        assert(p_mem->p_operand->kind == reg);

        I32CONST_t cnt = p_mem->cnt;
        assert(cnt != 0);

        if (p_sim == NULL) {
            p_sim = ir_operand_copy(p_mem->p_operand);
            sim_cnt = cnt;
            continue;
        }

        if (_i32_abs(cnt) == _i32_abs(sim_cnt) && _i32_abs(sim_cnt) != 1) {
            ir_binary_op b_op;
            if (cnt != sim_cnt)
                b_op = ir_sub_op;
            else
                b_op = ir_add_op;
            p_sim = _create_binary(b_op, p_sim, ir_operand_copy(p_mem->p_operand), p_func);
            continue;
        }


        if (!p_val) {
            p_val = p_sim;
            if (sim_cnt != 1)
                p_val = _create_binary(ir_mul_op, p_val, ir_operand_int_gen(sim_cnt), p_func);
        }
        else {
            if (_i32_abs(sim_cnt) != 1)
                p_sim = _create_binary(ir_mul_op, p_sim, ir_operand_int_gen(_i32_abs(sim_cnt)), p_func);
            ir_binary_op b_op;
            if (sim_cnt < 0)
                b_op = ir_sub_op;
            else
                b_op = ir_add_op;
            p_val = _create_binary(b_op, p_val, p_sim, p_func);
        }

        p_sim = ir_operand_copy(p_mem->p_operand);
        sim_cnt = cnt;
    }

    if (p_sim) {
        if (!p_val) {
            if (sim_cnt != 1)
                p_sim = _create_binary(ir_mul_op, p_sim, ir_operand_int_gen(sim_cnt), p_func);
            p_val = p_sim;
        }
        else {
            if (_i32_abs(sim_cnt) != 1)
                p_sim = _create_binary(ir_mul_op, p_sim, ir_operand_int_gen(_i32_abs(sim_cnt)), p_func);
            ir_binary_op b_op;
            if (sim_cnt < 0)
                b_op = ir_sub_op;
            else
                b_op = ir_add_op;
            p_val = _create_binary(b_op, p_val, p_sim, p_func);
        }
    }

    if (!p_val)
        p_val = ir_operand_copy(p_expr->p_const);
    else
        p_val = _create_binary(ir_add_op, p_val, ir_operand_copy(p_expr->p_const), p_func);

    p_ir_instr p_instr = ir_unary_instr_gen(ir_val_assign, p_val, p_expr->p_root);
    ir_basic_block_addinstr_tail(p_func->p_entry_block, p_instr);
}

static inline void _rewrite_mul_expr(p_expr_info p_expr, p_symbol_func p_func) {
    p_ir_operand p_val = NULL;

    p_ir_operand p_sim = NULL;
    I32CONST_t sim_cnt = 0;

    p_list_head p_node;
    list_for_each(p_node, &p_expr->members) {
        p_expr_mem p_mem = list_entry(p_node, expr_mem, node);
        assert(p_mem->p_operand->kind == reg);

        I32CONST_t cnt = p_mem->cnt;
        assert(cnt > 0);

        if (p_sim == NULL) {
            p_sim = ir_operand_copy(p_mem->p_operand);
            sim_cnt = cnt;
            continue;
        }

        if (cnt == sim_cnt && sim_cnt != 1) {
            p_sim = _create_binary(ir_mul_op, p_sim, ir_operand_copy(p_mem->p_operand), p_func);
            continue;
        }

        for (I32CONST_t i = 0; i < sim_cnt; ++i) {
            if (i) p_sim = ir_operand_copy(p_sim);
            if (!p_val) {
                p_val = p_sim;
                continue;
            }
            p_val = _create_binary(ir_mul_op, p_val, p_sim, p_func);
        }

        p_sim = ir_operand_copy(p_mem->p_operand);
        sim_cnt = cnt;
    }

    if (p_sim) {
        for (I32CONST_t i = 0; i < sim_cnt; ++i) {
            if (i) p_sim = ir_operand_copy(p_sim);
            if (!p_val) {
                p_val = p_sim;
                continue;
            }
            p_val = _create_binary(ir_mul_op, p_val, p_sim, p_func);
        }
    }

    if (!p_val)
        p_val = ir_operand_copy(p_expr->p_const);
    else
        p_val = _create_binary(ir_mul_op, p_val, ir_operand_copy(p_expr->p_const), p_func);

    p_ir_instr p_instr = ir_unary_instr_gen(ir_val_assign, p_val, p_expr->p_root);
    ir_basic_block_addinstr_tail(p_func->p_entry_block, p_instr);
}

static inline void _rewrite(p_vreg_info vreg_info_map, size_t vreg_cnt, p_symbol_func p_func) {
    for (size_t i = 0; i < vreg_cnt; ++i) {
        p_ir_vreg p_vreg = vreg_info_map[i].p_vreg;
        if (vreg_info_map[i].kind != root) {
            assert(!vreg_info_map[i].p_instr);
            assert(!vreg_info_map[i].p_expr);
            assert(list_head_alone(&vreg_info_map[i].p_vreg->use_list));
            symbol_func_vreg_del(p_func, p_vreg);
            continue;
        }
        if (vreg_info_map[i].p_expr) {
            assert(!vreg_info_map[i].p_instr);
            if (vreg_info_map[i].p_expr->kind == add)
                _rewrite_add_expr(vreg_info_map[i].p_expr, p_func);
            else
                _rewrite_mul_expr(vreg_info_map[i].p_expr, p_func);
            _expr_drop(vreg_info_map + i);
        }
    }
}

static inline void _reassociate_func(p_symbol_func p_func) {
    symbol_func_set_block_id(p_func);
    size_t vreg_cnt = p_func->param_reg_cnt + p_func->vreg_cnt;
    p_vreg_info vreg_info_map = malloc(sizeof(*vreg_info_map) * vreg_cnt);

    _collect_root(p_func, vreg_info_map);
    for (size_t i = 0; i < vreg_cnt; ++i) {
        if (vreg_info_map[i].kind == root)
            _collect_expr_for_root(vreg_info_map + i, vreg_info_map);
    }
    _reg_info_print(vreg_info_map, vreg_cnt);

    for (size_t i = 0; i < vreg_cnt; ++i) {
        if (vreg_info_map[i].kind != root)
            continue;
        if (vreg_info_map[i].p_expr) {
            _substitute_into_expr(vreg_info_map[i].p_expr, vreg_info_map);
        }
    }

    _sort_all_expr(vreg_info_map, vreg_cnt);
    _reg_info_print(vreg_info_map, vreg_cnt);

    _rewrite(vreg_info_map, vreg_cnt, p_func);

    free(vreg_info_map);
}

void ir_reassociate(p_program p_ir) {
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        _reassociate_func(p_func);
        symbol_func_set_block_id(p_func);
    }
}
