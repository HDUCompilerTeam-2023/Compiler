#include <ir/vreg.h>
#include <ir_gen.h>
#include <symbol_gen.h>

p_ir_vreg ir_vreg_gen(p_symbol_type p_type) {
    p_ir_vreg p_vreg = malloc(sizeof(*p_vreg));
    *p_vreg = (ir_vreg) {
        .p_type = p_type,
        .id = 0,
        .use_list = list_init_head(&p_vreg->use_list),
        .node = list_head_init(&p_vreg->node),
        .reg_id = -1,
        .if_float = false,
        .if_cond = false,
        .is_loop_inv = false,
        .scev_kind = scev_unknown,
        .p_scevexp = NULL,
    };
    return p_vreg;
}

p_ir_vreg ir_vreg_copy(p_ir_vreg p_vreg) {
    p_ir_vreg p_copy = malloc(sizeof(*p_copy));
    *p_copy = *p_vreg;
    p_copy->node = list_head_init(&p_copy->node);
    p_copy->use_list = list_head_init(&p_copy->use_list);
    p_copy->p_type = symbol_type_copy(p_vreg->p_type);
    p_copy->scev_kind = scev_unknown;
    p_copy->p_scevexp = NULL;
    p_copy->is_loop_inv = p_vreg->is_loop_inv;
    return p_copy;
}

void ir_vreg_drop(p_ir_vreg p_vreg) {
    list_del(&p_vreg->node);
    symbol_type_drop(p_vreg->p_type);
    if (p_vreg->p_scevexp) free(p_vreg->p_scevexp), p_vreg->p_scevexp = NULL;
    assert(list_head_alone(&p_vreg->use_list));
    free(p_vreg);
}
p_ir_vreg_list ir_vreg_list_init(void) {
    p_ir_vreg_list p_vreg_list = malloc(sizeof(*p_vreg_list));
    *p_vreg_list = (ir_vreg_list) {
        .vreg_list = list_head_init(&p_vreg_list->vreg_list),
    };
    return p_vreg_list;
}
p_ir_vreg_list ir_vreg_list_add(p_ir_vreg_list p_bb_phi_list, p_ir_vreg p_vreg) {
    p_ir_vreg_list_node p_vreg_list_node = ir_vreg_list_node_gen(p_vreg);
    list_add_prev(&p_vreg_list_node->node, &p_bb_phi_list->vreg_list);
    return p_bb_phi_list;
}
void ir_vreg_list_drop(p_ir_vreg_list p_vreg_list) {
    assert(p_vreg_list);
    while (!list_head_alone(&p_vreg_list->vreg_list)) {
        p_ir_vreg_list_node p_vreg_list_node = list_entry(p_vreg_list->vreg_list.p_next, ir_vreg_list_node, node);
        ir_vreg_list_node_drop(p_vreg_list_node);
    }
    free(p_vreg_list);
}
p_ir_vreg_list_node ir_vreg_list_node_gen(p_ir_vreg p_vreg) {
    p_ir_vreg_list_node p_vreg_list_node = malloc(sizeof(*p_vreg_list_node));
    *p_vreg_list_node = (ir_vreg_list_node) {
        .p_vreg = p_vreg,
        .node = list_head_init(&p_vreg_list_node->node),
    };
    return p_vreg_list_node;
}
void ir_vreg_list_node_drop(p_ir_vreg_list_node p_vreg_list_node) {
    list_del(&p_vreg_list_node->node);
    free(p_vreg_list_node);
}
void ir_vreg_list_node_set_vreg(p_ir_vreg_list_node p_vreg_list_node, p_ir_vreg p_vreg) {
    p_vreg_list_node->p_vreg = p_vreg;
}

void copy_vreg_list(p_ir_vreg_list p_des, p_ir_vreg_list p_src) {
    p_list_head p_node;
    list_for_each(p_node, &p_src->vreg_list) {
        p_ir_vreg p_param = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        ir_vreg_list_add(p_des, p_param);
    }
}

bool if_in_vreg_list(p_ir_vreg_list p_list, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->vreg_list) {
        p_ir_vreg p_live = list_entry(p_node, ir_vreg_list_node, node)->p_vreg;
        if (p_live == p_vreg)
            return true;
    }
    return false;
}

void ir_vreg_list_del(p_ir_vreg_list p_list, p_ir_vreg p_vreg) {
    p_list_head p_node;
    list_for_each(p_node, &p_list->vreg_list) {
        p_ir_vreg_list_node p_vreg_list_node = list_entry(p_node, ir_vreg_list_node, node);
        if (p_vreg_list_node->p_vreg == p_vreg) {
            ir_vreg_list_node_drop(p_vreg_list_node);
            break;
        }
    }
}
void ir_vreg_list_clear(p_ir_vreg_list p_list){
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_list->vreg_list){
        p_ir_vreg_list_node p_list_node = list_entry(p_node, ir_vreg_list_node, node);
        list_del(&p_list_node->node);
        free(p_list_node);
    }
}
