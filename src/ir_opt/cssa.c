#include <program/use.h>
#include <program/def.h>
#include <ir_manager/liveness.h>
#include <ir_opt/cssa.h>
#include <ir_gen/basic_block.h>
#include <ir_gen/vreg.h>
#include <ir_gen/instr.h>
#include <ir_gen/operand.h>
#include <ir_gen/bb_param.h>
#include <symbol_gen/type.h>
#include <symbol_gen/func.h>

static inline p_pcc_node _pcc_node_gen(p_ir_vreg p_vreg) {
    p_pcc_node p_pn = malloc(sizeof(*p_pn));
    *p_pn = (pcc_node) {
        .p_vreg = p_vreg,
        .node = list_init_head(&p_pn->node),
    };
    return p_pn;
}
static inline void _pcc_node_drop(p_pcc_node p_pn) {
    list_del(&p_pn->node);
    free(p_pn);
}
static inline p_phi_congruence_class _phi_congruence_class_gen(void) {
    p_phi_congruence_class pcc = malloc(sizeof(*pcc));
    *pcc = (phi_congruence_class) {
        .p_mem_type = NULL,
        .set = list_init_head(&pcc->set),
        .node = list_init_head(&pcc->node),
    };
    return pcc;
}
static inline void _phi_congruence_class_add(p_phi_congruence_class pcc, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &pcc->set) {
        p_pcc_node p_pn = list_entry(p_node, pcc_node, node);
        if (p_pn->p_vreg == p_vreg)
            return;
    }
    p_pcc_node p_pn = _pcc_node_gen(p_vreg);
    list_add_prev(&p_pn->node, &pcc->set);
    if (pcc->p_mem_type) {
        assert(p_vreg->p_type->ref_level == pcc->p_mem_type->ref_level);
        assert(p_vreg->p_type->basic == pcc->p_mem_type->basic);
        return;
    }
    pcc->p_mem_type = symbol_type_copy(p_vreg->p_type);
}
void phi_congruence_class_drop(p_phi_congruence_class pcc) {
    symbol_type_drop(pcc->p_mem_type);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &pcc->set) {
        p_pcc_node p_pn = list_entry(p_node, pcc_node, node);
        _pcc_node_drop(p_pn);
    }
    list_del(&pcc->node);
    free(pcc);
}
void phi_congruence_class_print(p_phi_congruence_class pcc) {
    printf("{");
    p_list_head p_node;
    list_for_each(p_node, &pcc->set) {
        p_pcc_node p_pn = list_entry(p_node, pcc_node, node);
        printf("%%%ld, ", p_pn->p_vreg->id);
    }
    printf("}\n");
}

typedef struct {
    p_ir_vreg p_vreg;
    list_head node;
} vreg_node, *p_vreg_node;
static inline p_vreg_node _vreg_node_gen(p_ir_vreg p_vreg) {
    p_vreg_node p_vn = malloc(sizeof(*p_vn));
    *p_vn = (vreg_node) {
        .p_vreg = p_vreg,
        .node = list_init_head(&p_vn->node),
    };
    return p_vn;
}
static inline void _vreg_node_drop(p_vreg_node p_vn) {
    list_del(&p_vn->node);
    free(p_vn);
}
static inline void _vreg_set_add(p_list_head p_set, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, p_set) {
        p_vreg_node p_vn = list_entry(p_node, vreg_node, node);
        if (p_vn->p_vreg == p_vreg)
            return;
    }
    p_vreg_node p_vn = _vreg_node_gen(p_vreg);
    list_add_prev(&p_vn->node, p_set);
}

typedef struct {
    p_ir_bb_param p_param;
    list_head node;
} bb_param_node, *p_bb_param_node;
static inline void _bb_param_node_add(p_ir_bb_param p_param, p_list_head p_head) {
    p_bb_param_node p_bpn = malloc(sizeof(*p_bpn));
    *p_bpn = (bb_param_node) {
        .p_param = p_param,
        .node = list_init_head(&p_bpn->node),
    };
    list_add_prev(&p_bpn->node, p_head);
}
static inline void _bb_param_list_drop(p_list_head p_head) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, p_head) {
        p_bb_param_node p_bpn = list_entry(p_node, bb_param_node, node);
        list_del(&p_bpn->node);
        free(p_bpn);
    }
}

static inline p_ir_vreg_list _get_bb_live_in(p_ir_basic_block p_bb) {
    if (list_head_alone(&p_bb->instr_list))
        return p_bb->p_branch->p_live_in;
    p_ir_instr p_instr = list_entry(p_bb->instr_list.p_next, ir_instr, node);
    return p_instr->p_live_in;
}
static inline p_ir_vreg_list _get_bb_live_out(p_ir_basic_block p_bb) {
    return p_bb->p_branch->p_live_in;
}

static inline p_ir_vreg_list _get_live_info(p_ir_vreg p_vreg) {
    assert(p_vreg);
    switch (p_vreg->def_type) {
    case instr_def:
        return p_vreg->p_instr_def->p_live_out;
    case bb_phi_def:
        return _get_bb_live_in(p_vreg->p_bb_phi->p_basic_block);
    case func_param_def:
        return _get_bb_live_in(p_vreg->p_func->p_entry_block);
    }
}

static inline bool _check_vreg_is_interfenrence(p_ir_vreg p_vreg_i, p_ir_vreg p_vreg_j) {
    assert(p_vreg_i);
    assert(p_vreg_j);
    if (p_vreg_i == p_vreg_j)
        return false;
    p_ir_vreg_list p_live_info_i = _get_live_info(p_vreg_i), p_live_info_j = _get_live_info(p_vreg_j);

    p_list_head p_node;
    list_for_each(p_node, &p_live_info_i->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        if (p_vreg == p_vreg_j)
            return true;
    }
    list_for_each(p_node, &p_live_info_j->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        if (p_vreg == p_vreg_i)
            return true;
    }
    return false;
}
static inline bool _check_pcc_is_interference(p_phi_congruence_class p_pcc_i, p_phi_congruence_class p_pcc_j) {
    p_list_head p_node_i, p_node_j;
    list_for_each(p_node_i, &p_pcc_i->set) {
        p_pcc_node p_pn_i = list_entry(p_node_i, pcc_node, node);
        p_ir_vreg p_vreg_i = p_pn_i->p_vreg;
        list_for_each(p_node_j, &p_pcc_j->set) {
            p_pcc_node p_pn_j = list_entry(p_node_j, pcc_node, node);
            p_ir_vreg p_vreg_j = p_pn_j->p_vreg;

            if (_check_vreg_is_interfenrence(p_vreg_i, p_vreg_j))
                return true;
        }
    }
    return false;
}
static inline bool _check_exist_in_pcc_and_live(p_phi_congruence_class p_pcc, p_ir_vreg_list p_live) {
    p_list_head p_node_pcc, p_node_live;
    list_for_each(p_node_pcc, &p_pcc->set) {
        p_ir_vreg p_vreg_pcc = list_entry(p_node_pcc, pcc_node, node)->p_vreg;
        list_for_each(p_node_live, &p_live->vreg_list) {
            p_ir_vreg p_vreg_live = list_entry(p_node_live, ir_vreg_list_node, node)->p_vreg;
            if (p_vreg_pcc == p_vreg_live)
                return true;
        }
    }
    return false;
}
typedef struct {
    size_t n;
    bool *gragh;
    size_t *cnt;
} graph, *p_graph;
static inline p_graph _graph_gen(size_t n) {
    p_graph p_g = malloc(sizeof(*p_g));
    *p_g = (graph) {
        .n = n,
        .gragh = malloc(sizeof(*p_g->gragh) * n * n),
        .cnt = malloc(sizeof(*p_g->cnt) * n),
    };
    memset(p_g->gragh, 0, sizeof(*p_g->gragh) * n * n);
    memset(p_g->cnt, 0, sizeof(*p_g->cnt) * n);
    return p_g;
}
static inline void _graph_drop(p_graph p_g) {
    free(p_g->gragh);
    free(p_g->cnt);
    free(p_g);
}
static inline void _graph_set(p_graph p_g, size_t i, size_t j) {
    assert(i != j);
    if (p_g->gragh[i * p_g->n + j]) {
        assert(p_g->gragh[j * p_g->n + i]);
        return;
    }
    assert(!p_g->gragh[j * p_g->n + i]);
    p_g->gragh[i * p_g->n + j] = true;
    p_g->gragh[j * p_g->n + i] = true;
    ++p_g->cnt[i];
    ++p_g->cnt[j];
}
static inline void _graph_unset(p_graph p_g, size_t i) {
    for (size_t j = 0; j < p_g->n; ++j) {
        if (!p_g->gragh[i * p_g->n + j]) {
            assert(!p_g->gragh[j * p_g->n + i]);
            continue;
        }
        assert(p_g->gragh[j * p_g->n + i]);
        p_g->gragh[i * p_g->n + j] = false;
        p_g->gragh[j * p_g->n + i] = false;
        --p_g->cnt[i];
        --p_g->cnt[j];
    }
    assert(!p_g->cnt[i]);
}
static inline size_t _eliminate_phi_interference_for_each_phi(p_ir_bb_phi p_phi, p_list_head p_param_list, size_t vreg_cnt, p_list_head pcc_head) {
    size_t copy_cnt = 0;

    // Set candidateResourceSet
    list_head candidate_resource_set = list_init_head(&candidate_resource_set);
    // for each xi (0<=i<=n) in phiInst unresolvedNeighborMap[xi] = {};
    p_graph unresolved_neighbor_map = _graph_gen(vreg_cnt);
    list_head unresolved_sort_head = list_init_head(&unresolved_sort_head);
    list_head unresolved_choose_head = list_init_head(&unresolved_choose_head);

    p_phi_congruence_class phi_congruence_class_phi = p_phi->p_bb_phi->p_info;

    p_list_head p_node_i;
    list_for_each(p_node_i, p_param_list) {
        p_ir_bb_param p_param_i = list_entry(p_node_i, bb_param_node, node)->p_param;
        p_phi_congruence_class phi_congruence_class_i = p_param_i->p_bb_param->p_vreg->p_info;
        if (_check_pcc_is_interference(phi_congruence_class_i, phi_congruence_class_phi)) {
            bool deal = false;
            if (_check_exist_in_pcc_and_live(phi_congruence_class_phi, _get_bb_live_out(p_param_i->p_target->p_source_block))) {
                _vreg_set_add(&candidate_resource_set, p_phi->p_bb_phi);
                deal = true;
            }
            if (_check_exist_in_pcc_and_live(phi_congruence_class_i, _get_bb_live_in(p_phi->p_basic_block))) {
                _vreg_set_add(&candidate_resource_set, p_param_i->p_bb_param->p_vreg);
                deal = true;
            }
            if (!deal) {
                _vreg_set_add(&unresolved_sort_head, p_param_i->p_bb_param->p_vreg);
                _vreg_set_add(&unresolved_sort_head, p_phi->p_bb_phi);
                _graph_set(unresolved_neighbor_map, p_param_i->p_bb_param->p_vreg->id, p_phi->p_bb_phi->id);
            }
        }

        p_list_head p_node_j;
        list_for_each(p_node_j, p_param_list) {
            if (p_node_j == p_node_i)
                break;
            p_ir_bb_param p_param_j = list_entry(p_node_j, bb_param_node, node)->p_param;
            p_phi_congruence_class phi_congruence_class_j = p_param_j->p_bb_param->p_vreg->p_info;
            if (_check_pcc_is_interference(phi_congruence_class_i, phi_congruence_class_j)) {
                bool deal = false;
                if (_check_exist_in_pcc_and_live(phi_congruence_class_i, _get_bb_live_out(p_param_j->p_target->p_source_block))) {
                    _vreg_set_add(&candidate_resource_set, p_param_i->p_bb_param->p_vreg);
                    deal = true;
                }
                if (_check_exist_in_pcc_and_live(phi_congruence_class_j, _get_bb_live_out(p_param_i->p_target->p_source_block))) {
                    _vreg_set_add(&candidate_resource_set, p_param_j->p_bb_param->p_vreg);
                    deal = true;
                }
                if (!deal) {
                    _vreg_set_add(&unresolved_sort_head, p_param_i->p_bb_param->p_vreg);
                    _vreg_set_add(&unresolved_sort_head, p_param_j->p_bb_param->p_vreg);
                    _graph_set(unresolved_neighbor_map, p_param_i->p_bb_param->p_vreg->id, p_param_j->p_bb_param->p_vreg->id);
                }
            }
        }
    }

    // Process the unresolved resources (Case 4) as described in Section 3.1.3.
    p_list_head p_node;
    list_for_each(p_node, &candidate_resource_set) {
        p_vreg_node p_vn = list_entry(p_node, vreg_node, node);
        _graph_unset(unresolved_neighbor_map, p_vn->p_vreg->id);
    }

    size_t *remain_neighbor = malloc(sizeof(*remain_neighbor) * vreg_cnt);
    for (size_t i = 0; i < vreg_cnt; ++i)
        remain_neighbor[i] = unresolved_neighbor_map->cnt[i];

    while(!list_head_alone(&unresolved_sort_head)) {
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &unresolved_sort_head) {
            p_vreg_node p_vn = list_entry(p_node, vreg_node, node);
            if (!remain_neighbor[p_vn->p_vreg->id]) {
                _vreg_node_drop(p_vn);
                continue;
            }
            list_del(&p_vn->node);
            p_list_head p_node_prev;
            list_for_each(p_node_prev, &unresolved_sort_head) {
                if (p_node_prev == p_next)
                    break;
                p_vreg_node p_vn_prev = list_entry(p_node_prev, vreg_node, node);
                if (remain_neighbor[p_vn_prev->p_vreg->id] > remain_neighbor[p_vn->p_vreg->id])
                    break;
            }
            list_add_prev(&p_vn->node, p_node_prev);
        }

        if (list_head_alone(&unresolved_sort_head))
            break;

        p_vreg_node p_vn = list_entry(unresolved_sort_head.p_prev, vreg_node, node);
        list_del(&p_vn->node);
        list_add_prev(&p_vn->node, &unresolved_choose_head);

        for (size_t i = 0; i < vreg_cnt; ++i) {
            if (remain_neighbor[i] && unresolved_neighbor_map->gragh[p_vn->p_vreg->id * unresolved_neighbor_map->n + i])
                --remain_neighbor[i];
        }
    }

    p_list_head p_next;
    list_for_each_safe(p_node, p_next, &unresolved_choose_head) {
        p_vreg_node p_vn = list_entry(p_node, vreg_node, node);
        if (!remain_neighbor[p_vn->p_vreg->id]) {
            _vreg_node_drop(p_vn);
            continue;
        }
        list_del(&p_vn->node);
        list_add_prev(&p_vn->node, &candidate_resource_set);
    }
    free(remain_neighbor);
    assert(list_head_alone(&unresolved_choose_head));
    assert(list_head_alone(&unresolved_sort_head));
    _graph_drop(unresolved_neighbor_map);

    // for each xi in candidateResourceSet insertCopy(xi,phiInst)
    bool *candidate_resource_set_book = malloc(sizeof(*candidate_resource_set_book) * vreg_cnt);
    memset(candidate_resource_set_book, 0, sizeof(*candidate_resource_set_book) * vreg_cnt);
    list_for_each_safe(p_node, p_next, &candidate_resource_set) {
        p_vreg_node p_vn = list_entry(p_node, vreg_node, node);
        p_ir_vreg p_vreg = p_vn->p_vreg;
        list_del(&p_vn->node);
        free(p_vn);

        candidate_resource_set_book[p_vreg->id] = true;
    }
    assert(list_head_alone(&candidate_resource_set));

    if (candidate_resource_set_book[p_phi->p_bb_phi->id]) {
        p_ir_vreg p_vreg = p_phi->p_bb_phi;

        p_ir_vreg p_copy = ir_vreg_copy(p_vreg);
        symbol_func_vreg_add(p_phi->p_basic_block->p_func, p_copy);

        p_phi_congruence_class pcc = _phi_congruence_class_gen();
        p_copy->p_info = pcc;
        _phi_congruence_class_add(pcc, p_copy);
        list_add_prev(&pcc->node, pcc_head);

        ir_bb_phi_set_vreg(p_phi, p_copy);
        p_ir_instr p_instr = ir_unary_instr_gen(ir_val_assign, ir_operand_vreg_gen(p_copy), p_vreg);
        copy_vreg_list(p_instr->p_live_out, _get_bb_live_in(p_phi->p_basic_block));
        copy_vreg_list(p_instr->p_live_in, p_instr->p_live_out);
        ir_basic_block_addinstr_head(p_phi->p_basic_block, p_instr);

        printf("copy phi %%%ld\n", p_vreg->id);
        ++copy_cnt;

        ir_vreg_list_add(p_instr->p_live_in, p_copy, 0);
        ir_vreg_list_del(p_instr->p_live_in, p_vreg);
    }

    list_for_each(p_node, p_param_list) {
        p_ir_bb_param p_param = list_entry(p_node, bb_param_node, node)->p_param;
        p_ir_vreg p_vreg = p_param->p_bb_param->p_vreg;
        p_ir_basic_block p_bb = p_param->p_target->p_source_block;
        if (!candidate_resource_set_book[p_vreg->id])
            continue;

        p_ir_vreg p_copy = ir_vreg_copy(p_vreg);
        symbol_func_vreg_add(p_phi->p_basic_block->p_func, p_copy);

        p_phi_congruence_class pcc = _phi_congruence_class_gen();
        p_copy->p_info = pcc;
        _phi_congruence_class_add(pcc, p_copy);
        list_add_prev(&pcc->node, pcc_head);

        ir_operand_reset_vreg(p_param->p_bb_param, p_copy);
        p_ir_instr p_instr = ir_unary_instr_gen(ir_val_assign, ir_operand_vreg_gen(p_vreg), p_copy);
        copy_vreg_list(p_instr->p_live_in, _get_bb_live_out(p_bb));
        copy_vreg_list(p_instr->p_live_out, p_instr->p_live_in);
        ir_basic_block_addinstr_tail(p_bb, p_instr);

        printf("copy param %%%ld\n", p_vreg->id);
        ++copy_cnt;

        ir_vreg_list_add(p_instr->p_live_out, p_copy, 0);
        ir_vreg_list_add(_get_bb_live_out(p_bb), p_copy, 0);
        if (if_in_vreg_list(p_bb->p_live_out, p_vreg))
            continue;
        if (p_bb->p_branch->p_target_1) {
            p_list_head p_node;
            list_for_each(p_node, &p_bb->p_branch->p_target_1->block_param) {
                p_ir_bb_param p_bp = list_entry(p_node, ir_bb_param, node);
                if (p_bp->p_bb_param->p_vreg == p_vreg)
                    break;
            }
            if (p_node != &p_bb->p_branch->p_target_1->block_param)
                continue;
        }
        if (p_bb->p_branch->p_target_2) {
            p_list_head p_node;
            list_for_each(p_node, &p_bb->p_branch->p_target_2->block_param) {
                p_ir_bb_param p_bp = list_entry(p_node, ir_bb_param, node);
                if (p_bp->p_bb_param->p_vreg == p_vreg)
                    break;
            }
            if (p_node != &p_bb->p_branch->p_target_2->block_param)
                continue;
        }
        ir_vreg_list_del(p_instr->p_live_out, p_vreg);
        ir_vreg_list_del(_get_bb_live_out(p_bb), p_vreg);
    }
    free(candidate_resource_set_book);

    // Merge phiCongruenceClass’s for all resources in phiInst.
    p_phi_congruence_class currentphi_congruence_class = p_phi->p_bb_phi->p_info;

    list_for_each(p_node, p_param_list) {
        p_ir_bb_param p_param = list_entry(p_node, bb_param_node, node)->p_param;
        p_ir_vreg p_vreg = p_param->p_bb_param->p_vreg;
        p_phi_congruence_class phi_congruence_class_param = p_vreg->p_info;
        if (phi_congruence_class_param == currentphi_congruence_class)
            continue;

        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &phi_congruence_class_param->set) {
            p_pcc_node p_pn = list_entry(p_node, pcc_node, node);
            p_ir_vreg p_congruence_vreg = p_pn->p_vreg;
            _pcc_node_drop(p_pn);
            _phi_congruence_class_add(currentphi_congruence_class, p_congruence_vreg);
            p_congruence_vreg->p_info = currentphi_congruence_class;
        }
        phi_congruence_class_drop(phi_congruence_class_param);
    }

    return copy_cnt;
}

static inline size_t _eliminate_phi_interference(p_symbol_func p_func, p_list_head pcc_head) {
    symbol_func_set_block_id(p_func);
    p_liveness_info p_live_info = liveness_info_gen(p_func);
    liveness_analysis(p_live_info);
    liveness_info_drop(p_live_info);

    size_t vreg_cnt = p_func->param_reg_cnt + p_func->vreg_cnt;

    size_t *bb_phi_cnt_map = malloc(sizeof(*bb_phi_cnt_map) * p_func->block_cnt);
    memset(bb_phi_cnt_map, 0, sizeof(*bb_phi_cnt_map) * p_func->block_cnt);

    // for each resource x participated in a phi, phiCongruenceClass[x] = {x}
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_phi_congruence_class pcc = _phi_congruence_class_gen();
        p_vreg->p_info = pcc;
        _phi_congruence_class_add(pcc, p_vreg);
        list_add_prev(&pcc->node, pcc_head);
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_phi_congruence_class pcc = _phi_congruence_class_gen();
        p_vreg->p_info = pcc;
        _phi_congruence_class_add(pcc, p_vreg);
        list_add_prev(&pcc->node, pcc_head);
    }
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        p_list_head p_node;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            ++bb_phi_cnt_map[p_bb->block_id];
        }
    }

    size_t copy_cnt = 0;
    list_for_each(p_node, &p_func->block) {
        p_ir_basic_block p_bb = list_entry(p_node, ir_basic_block, node);
        if (list_head_alone(&p_bb->basic_block_phis))
            continue;

        size_t bb_phi_cnt = bb_phi_cnt_map[p_bb->block_id];
        p_ir_bb_phi *phi_map = malloc(sizeof(p_ir_bb_phi) * bb_phi_cnt);
        p_list_head head_map = malloc(sizeof(*head_map) * bb_phi_cnt);
        for (size_t i = 0; i < bb_phi_cnt; ++i) {
            head_map[i] = list_init_head(head_map + i);
        }

        size_t phi_id = 0;
        p_list_head p_node;
        list_for_each(p_node, &p_bb->basic_block_phis) {
            p_ir_bb_phi p_phi = list_entry(p_node, ir_bb_phi, node);
            phi_map[phi_id++] = p_phi;
        }
        list_for_each(p_node, &p_bb->prev_branch_target_list) {
            p_ir_basic_block_branch_target p_target = list_entry(p_node, ir_branch_target_node, node)->p_target;
            assert(p_target->p_block == p_bb);
            size_t phi_id = 0;
            p_list_head p_node;
            list_for_each(p_node, &p_target->block_param) {
                p_ir_bb_param p_param = list_entry(p_node, ir_bb_param, node);
                assert(p_param->p_bb_param->kind == reg);
                _bb_param_node_add(p_param, head_map + phi_id++);
            }
        }

        // for each phi instruction in CFG
        for (size_t i = 0; i < bb_phi_cnt; ++i) {
            copy_cnt += _eliminate_phi_interference_for_each_phi(phi_map[i], head_map + i, vreg_cnt, pcc_head);
            _bb_param_list_drop(head_map + i);
        }
        free(phi_map);
        free(head_map);
    }
    list_for_each(p_node, &p_func->param_reg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_vreg->p_info = NULL;
    }
    list_for_each(p_node, &p_func->vreg_list) {
        p_ir_vreg p_vreg = list_entry(p_node, ir_vreg, node);
        p_vreg->p_info = NULL;
    }

    printf("TSSA to CSSA add %ld copy instr\n", copy_cnt);

    free(bb_phi_cnt_map);
    symbol_func_set_block_id(p_func);
    return copy_cnt;
}

size_t ir_opt_cssa_no_pcc(p_program p_ir) {
    size_t copy_cnt = 0;
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        list_head pcc_head = list_init_head(&pcc_head);
        copy_cnt += _eliminate_phi_interference(p_func, &pcc_head);
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &pcc_head) {
            p_phi_congruence_class pcc = list_entry(p_node, phi_congruence_class, node);
            phi_congruence_class_print(pcc);
            phi_congruence_class_drop(pcc);
        }
    }
    return copy_cnt;
}

size_t ir_opt_cssa(p_symbol_func p_func, p_list_head pcc_head) {
    return _eliminate_phi_interference(p_func, pcc_head);
}
