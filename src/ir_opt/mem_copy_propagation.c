#include <ir_gen.h>
#include <ir_manager/builddomtree.h>
#include <ir_manager/get_seqs.h>
#include <program/def.h>
#include <program/use.h>
#include <symbol_gen.h>

#define hash_MOD 109
#define hash_P 1023

typedef struct vmem_value_info_list vmem_value_info_list, *p_vmem_value_info_list;
typedef struct vmem_value_info vmem_value_info, *p_vmem_value_info;
typedef struct offset_value offset_value, *p_offset_value;
typedef struct whole_value whole_value, *p_whole_value;
struct offset_value {
    size_t offset;
    p_ir_vreg p_vreg_base;
    hlist_node h_node;
    list_head node;
    // size_t hash_tag;
    p_ir_vreg p_value;
    bool if_created_phi;
    // p_ir_varray p_varray;
};
struct whole_value {
    bool is_array;
    union {
        struct {
            p_ir_vreg p_vreg;
            bool if_created_phi;
        };
        struct {
            bool is_zero;
            p_ir_varray p_not_sure;
            list_head offset_value_list;
            hlist_hash hash;
        };
    };
};
struct vmem_value_info {
    p_whole_value p_current_value;
    size_t block_num;
};
struct vmem_value_info_list {
    p_vmem_value_info p_infos;
    size_t vmem_num;
};
static inline hlist_hash init_hash() {
    hlist_hash hash = malloc(sizeof(*hash) * hash_MOD);
    for (size_t i = 0; i < hash_MOD; ++i)
        hlist_head_init(hash + i);
    return hash;
}
static inline p_offset_value offset_value_gen(size_t offset, p_ir_vreg p_base, p_ir_vreg p_value) {
    p_offset_value p_sure = malloc(sizeof(*p_sure));
    p_sure->h_node = hlist_init_node;
    p_sure->node = list_head_init(&p_sure->node);
    p_sure->p_value = p_value;
    p_sure->offset = offset;
    p_sure->p_vreg_base = p_base;
    p_sure->if_created_phi = false;
    return p_sure;
}
#include <ir_print.h>
static inline void whole_value_gen(p_whole_value p_whole, p_ir_vmem_base p_vmem_base) {
    if (p_vmem_base->is_vmem)
        p_whole->is_array = !list_head_alone(&p_vmem_base->p_vmem_base->p_type->array);
    else
        p_whole->is_array = true;
    if (p_whole->is_array) {
        p_whole->p_not_sure = NULL;
        p_whole->is_zero = false;
        p_whole->hash = init_hash();
        p_whole->offset_value_list = list_head_init(&p_whole->offset_value_list);
    }
    else {
        p_whole->if_created_phi = false;
        p_whole->p_vreg = NULL;
    }
}
static inline void vmem_value_info_gen(p_vmem_value_info p_info, size_t block_num, p_ir_vmem_base p_vmem_base) {
    p_info->p_current_value = malloc(sizeof(*p_info->p_current_value) * block_num);
    for (size_t i = 0; i < block_num; i++) {
        whole_value_gen(p_info->p_current_value + i, p_vmem_base);
    }
    p_info->block_num = block_num;
    p_vmem_base->p_info = p_info;
}
static inline p_vmem_value_info_list vmem_value_info_list_gen(p_symbol_func p_func, p_program p_ir) {
    p_vmem_value_info_list p_list = malloc(sizeof(*p_list));
    p_list->vmem_num = p_func->var_cnt + p_ir->variable_cnt + p_func->param_vmem_base_num;
    p_list->p_infos = malloc(sizeof(*p_list->p_infos) * p_list->vmem_num);
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_ir->variable) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        vmem_value_info_gen(p_list->p_infos + id, p_func->block_cnt, p_vmem_base);
        id++;
    }
    list_for_each(p_node, &p_func->variable) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        vmem_value_info_gen(p_list->p_infos + id, p_func->block_cnt, p_vmem_base);
        id++;
    }
    list_for_each(p_node, &p_func->param) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, symbol_var, node)->p_base;
        vmem_value_info_gen(p_list->p_infos + id, p_func->block_cnt, p_vmem_base);
        id++;
    }
    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_vmem_base p_vmem_base = list_entry(p_node, ir_param_vmem_base, node)->p_param_base;
        vmem_value_info_gen(p_list->p_infos + id, p_func->block_cnt, p_vmem_base);
        id++;
    }
    assert(id == p_list->vmem_num);
    return p_list;
}
static inline p_whole_value get_whole_value_by_index(p_vmem_value_info_list p_list, size_t i, p_ir_basic_block p_basic_block) {
    return ((p_list->p_infos + i)->p_current_value + p_basic_block->block_id);
}
static inline void offset_value_drop(p_offset_value p_varray_value) {
    assert(p_varray_value->node.p_next);

    list_del(&p_varray_value->node);

    hlist_node_del(&p_varray_value->h_node);

    free(p_varray_value);
}
static inline void whole_value_drop(p_whole_value p_whole) {
    if (p_whole->is_array) {
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &p_whole->offset_value_list) {
            p_offset_value p_varray_value = list_entry(p_node, offset_value, node);
            offset_value_drop(p_varray_value);
        }
        free(p_whole->hash);
    }
}
static inline void vmem_value_info_drop(p_vmem_value_info p_info) {
    for (size_t i = 0; i < p_info->block_num; i++) {
        whole_value_drop(p_info->p_current_value + i);
    }
    free(p_info->p_current_value);
}
static inline void vmem_value_info_list_drop(p_vmem_value_info_list p_list) {
    for (size_t i = 0; i < p_list->vmem_num; i++) {
        vmem_value_info_drop(p_list->p_infos + i);
    }
    free(p_list->p_infos);
    free(p_list);
}
static inline size_t get_tag(size_t offset, p_ir_vreg p_vreg) {
    return (offset + (size_t) p_vreg) % hash_P % hash_MOD;
}
static inline p_offset_value find_offset_value(hlist_hash hash, size_t offset, p_ir_vreg p_vreg) {
    size_t tag = get_tag(offset, p_vreg);
    p_hlist_node p_node;
    hlist_for_each(p_node, hash + tag) {
        p_offset_value p_sure_value = hlist_entry(p_node, offset_value, h_node);
        if (p_sure_value->p_vreg_base == p_vreg && p_sure_value->offset == offset)
            return p_sure_value;
    }
    return NULL;
}
static inline p_whole_value get_whole_value(p_ir_varray p_varray, p_ir_basic_block p_current_block) {
    return ((p_vmem_value_info) p_varray->p_base->p_info)->p_current_value + p_current_block->block_id;
}
static inline void offset_value_add(p_offset_value p_sure_value, p_whole_value p_whole) {
    assert(p_whole->is_array);
    assert(!find_offset_value(p_whole->hash, p_sure_value->offset, p_sure_value->p_vreg_base));
    assert(hlist_node_add(p_whole->hash + get_tag(p_sure_value->offset, p_sure_value->p_vreg_base), &p_sure_value->h_node));
    assert(list_add_prev(&p_sure_value->node, &p_whole->offset_value_list));
}
static inline void set_varray_not_sure(p_whole_value p_whole, p_ir_varray p_varray) {
    p_whole->p_not_sure = p_varray;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_whole->offset_value_list) {
        p_offset_value p_offset = list_entry(p_node, offset_value, node);
        offset_value_drop(p_offset);
    }
    p_whole->is_zero = false;
}
static inline void set_whole_not_sure(p_whole_value p_whole) {
    if (p_whole->is_array)
        set_varray_not_sure(p_whole, NULL);
    else
        p_whole->p_vreg = NULL;
}
static inline p_offset_value offset_value_copy(p_offset_value p_varray_value) {
    p_offset_value p_sure_value = malloc(sizeof(*p_sure_value));
    p_sure_value->offset = p_varray_value->offset;
    p_sure_value->h_node = hlist_init_node;
    p_sure_value->p_value = p_varray_value->p_value;
    p_sure_value->p_vreg_base = p_varray_value->p_vreg_base;
    p_sure_value->node = list_head_init(&p_sure_value->node);
    return p_sure_value;
}
static inline void whole_value_copy(p_whole_value p_des, p_whole_value p_src) {
    assert(p_des->is_array == p_src->is_array);
    p_des->is_zero = p_src->is_zero;
    if (!p_des->is_array)
        p_des->p_vreg = p_src->p_vreg;
    else {
        p_list_head p_node;
        p_des->p_not_sure = p_src->p_not_sure;
        list_for_each(p_node, &p_src->offset_value_list) {
            p_offset_value p_sure_value = list_entry(p_node, offset_value, node);
            p_offset_value p_new_value = offset_value_copy(p_sure_value);
            offset_value_add(p_new_value, p_des);
        }
    }
}
static inline void vmem_value_info_list_copy(p_vmem_value_info_list p_list, p_ir_basic_block p_des, p_ir_basic_block p_src) {
    for (size_t i = 0; i < p_list->vmem_num; i++) {
        whole_value_copy(get_whole_value_by_index(p_list, i, p_des), get_whole_value_by_index(p_list, i, p_src));
    }
}
static inline void vmem_value_info_list_reset(p_vmem_value_info_list p_list, p_ir_basic_block p_basic_block) {
    for (size_t i = 0; i < p_list->vmem_num; i++) {
        p_whole_value p_old = get_whole_value_by_index(p_list, i, p_basic_block);
        p_list_head p_node;
        bool if_phi = false;
        list_for_each(p_node, &p_basic_block->varray_basic_block_phis) {
            p_ir_varray p_varray = list_entry(p_node, ir_varray_bb_phi, node)->p_varray_phi;
            if (get_whole_value(p_varray, p_basic_block) == p_old) {
                if_phi = true;
                break;
            }
        }
        if (!if_phi) {
            if (p_basic_block->p_dom_parent) {
                p_whole_value p_dom = get_whole_value_by_index(p_list, i, p_basic_block->p_dom_parent);
                if (p_old->is_array) {
                    set_varray_not_sure(p_old, NULL);
                    whole_value_copy(p_old, p_dom);
                }
                else
                    p_old->p_vreg = p_dom->p_vreg;
            }
            else {
                set_whole_not_sure(p_old);
            }
            continue;
        }
        if (!p_old->is_array) {
            if (!p_old->if_created_phi) {
                p_old->p_vreg = NULL;
            }
            continue;
        }
        p_list_head p_next;
        list_for_each_safe(p_node, p_next, &p_old->offset_value_list) {
            p_offset_value p_offset = list_entry(p_node, offset_value, node);
            if (!p_offset->if_created_phi) {
                offset_value_drop(p_offset);
            }
        }
    }
}
static inline void print_phi_param_array(p_ir_varray_bb_phi p_des_varray, p_ir_varray_bb_param p_src_varray, p_ir_vreg p_des, p_offset_value p_offset, p_ir_operand p_src) {
    printf("add <phi, param> of <");
    ir_varray_bb_phi_print(p_des_varray);
    printf(", ");
    ir_varray_bb_param_print(p_src_varray);
    printf("> in [");
    if (p_offset->p_vreg_base)
        ir_vreg_print(p_offset->p_vreg_base);
    else
        printf("%ld", p_offset->offset);
    printf("] : ");
    ir_vreg_print(p_des);
    printf(", ");
    ir_operand_print(p_src);
    printf(">\n");
}
static inline void print_phi_param_not_array(p_ir_varray_bb_phi p_des_varray, p_ir_varray_bb_param p_src_varray, p_ir_vreg p_des, p_ir_operand p_src) {
    printf("add <phi, param> of <");
    ir_varray_bb_phi_print(p_des_varray);
    printf(", ");
    ir_varray_bb_param_print(p_src_varray);
    printf("> : <");
    ir_vreg_print(p_des);
    printf(", ");
    ir_operand_print(p_src);
    printf(">\n");
}
static inline void deal_store(p_ir_instr p_instr, bool if_init) {
    p_ir_store_instr p_store = &p_instr->ir_store;
    p_ir_varray p_src = p_store->p_array_src->p_varray_use;
    p_ir_varray p_des = p_store->p_array_des;
    p_whole_value src_info = get_whole_value(p_src, p_instr->p_basic_block);
    p_whole_value des_info = get_whole_value(p_des, p_instr->p_basic_block);
    assert(des_info == src_info);
    p_ir_vreg p_vreg = NULL;
    if (if_init) {
        p_vreg = ir_vreg_gen(symbol_type_copy(p_store->p_src->p_type));
        symbol_func_vreg_add(p_instr->p_basic_block->p_func, p_vreg);
        p_ir_instr p_new_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_copy(p_store->p_src), p_vreg);
        ir_instr_add_prev(p_new_assign, p_instr);
    }
    else {
        p_ir_instr p_prev = list_entry(p_instr->node.p_prev, ir_instr, node);
        assert(p_prev->irkind == ir_unary && p_prev->ir_unary.op == ir_val_assign);
        p_vreg = p_prev->ir_unary.p_des;
    }
    if (!src_info->is_array) {
        des_info->p_vreg = p_vreg;
        return;
    }
    size_t offset = 0;
    p_ir_vreg p_vreg_base = NULL;
    if (p_store->p_addr->kind == reg) {
        p_vreg_base = p_store->p_addr->p_vreg;
        set_varray_not_sure(des_info, p_des);
        p_offset_value p_new = offset_value_gen(offset, p_vreg_base, p_vreg);
        offset_value_add(p_new, des_info);
    }
    else {
        if (p_src->varray_def_type == varray_func_def || p_src->varray_def_type == varray_global_def) {
            assert(!src_info->p_not_sure);
            des_info->p_not_sure = p_src;
        }
        offset = p_store->p_addr->offset;
        p_offset_value p_varray_value = find_offset_value(des_info->hash, offset, p_vreg_base);
        if (p_varray_value) {
            p_varray_value->p_value = p_vreg;
        }
        else {
            p_list_head p_node, p_next;
            list_for_each_safe(p_node, p_next, &des_info->offset_value_list) {
                p_offset_value p_varray_sure_value = list_entry(p_node, offset_value, node);
                if (p_varray_sure_value->p_vreg_base)
                    offset_value_drop(p_varray_sure_value);
            }
            p_offset_value p_new = offset_value_gen(offset, p_vreg_base, p_vreg);
            offset_value_add(p_new, des_info);
        }
    }
}
static inline void deal_load(p_ir_instr p_instr) {
    p_ir_load_instr p_load = &p_instr->ir_load;
    p_ir_varray_use p_src = p_load->p_array_src;
    p_whole_value src_info = get_whole_value(p_src->p_varray_use, p_instr->p_basic_block);
    size_t offset = 0;
    p_ir_vreg p_vreg_base = NULL;
    if (!src_info->is_array) {
        if (src_info->p_vreg) {
            ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_vreg_gen(src_info->p_vreg), p_load->p_des);
        }
        src_info->p_vreg = p_load->p_des;
        return;
    }
    if (p_load->p_addr->kind == reg) {
        p_vreg_base = p_load->p_addr->p_vreg;
        p_offset_value p_varray_value = find_offset_value(src_info->hash, offset, p_vreg_base);

        if (p_varray_value) {
            ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_vreg_gen(p_varray_value->p_value), p_load->p_des);
            p_varray_value->p_value = p_load->p_des;
        }
        else {
            if (src_info->is_zero && list_head_alone(&src_info->offset_value_list)) { // 没有任何确定的值
                ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_int_gen(0), p_load->p_des);
            }
            else {
                // can't reset
            }
            p_offset_value p_new = offset_value_gen(offset, p_vreg_base, p_load->p_des);
            offset_value_add(p_new, src_info);
        }
    }
    else {
        offset = p_load->p_addr->offset;
        p_offset_value p_varray_value = find_offset_value(src_info->hash, offset, p_vreg_base);
        if (p_varray_value) {
            ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_vreg_gen(p_varray_value->p_value), p_load->p_des);
            p_varray_value->p_value = p_load->p_des;
        }
        else {
            if (src_info->is_zero) {
                ir_instr_reset_unary(p_instr, ir_val_assign, ir_operand_int_gen(0), p_load->p_des);
            }
            else {
                if (src_info->p_not_sure)
                    ir_varray_use_reset_varray(p_src, src_info->p_not_sure);
            }
            p_offset_value p_new = offset_value_gen(offset, p_vreg_base, p_load->p_des);
            offset_value_add(p_new, src_info);
        }
    }
}
static inline void deal_call(p_ir_instr p_instr) {
    p_ir_call_instr p_call_instr = &p_instr->ir_call;
    if (!strcmp(p_call_instr->p_func->name, "memset")) {
        // set_zero
        assert(!list_head_alone(&p_instr->ir_call.varray_defs));
        assert(p_instr->ir_call.varray_defs.p_next == p_instr->ir_call.varray_defs.p_prev);
        p_ir_varray_def_pair p_pair = list_entry(p_instr->ir_call.varray_defs.p_next, ir_varray_def_pair, node);
        if (!p_pair->p_des) return;
        p_whole_value des_info = get_whole_value(p_pair->p_des, p_instr->p_basic_block);
        p_whole_value src_info = get_whole_value(p_pair->p_src->p_varray_use, p_instr->p_basic_block);
        assert(des_info == src_info);
        assert(des_info->is_array);
        set_varray_not_sure(des_info, NULL);
        des_info->is_zero = true;
        return;
    }
    p_list_head p_node;
    list_for_each(p_node, &p_call_instr->varray_defs) {
        p_ir_varray_def_pair p_pair = list_entry(p_node, ir_varray_def_pair, node);
        if (!p_pair->p_des) continue;
        p_whole_value des_info = get_whole_value(p_pair->p_des, p_instr->p_basic_block);
        p_whole_value src_info = get_whole_value(p_pair->p_src->p_varray_use, p_instr->p_basic_block);
        assert(des_info == src_info);
        // whole not sure
        if (des_info->is_array)
            set_varray_not_sure(des_info, p_pair->p_des);
        else
            des_info->p_vreg = NULL;
    }
}
static inline void init_block_info(p_vmem_value_info_list p_list, p_ir_basic_block p_basic_block) {
    if (p_basic_block->p_dom_parent) {
        vmem_value_info_list_copy(p_list, p_basic_block, p_basic_block->p_dom_parent);
        p_list_head p_node;
        list_for_each(p_node, &p_basic_block->varray_basic_block_phis) {
            p_ir_varray p_phi = list_entry(p_node, ir_varray_bb_phi, node)->p_varray_phi;
            p_whole_value p_whole = get_whole_value(p_phi, p_basic_block);
            if (p_whole->is_array)
                set_varray_not_sure(p_whole, p_phi);
            else
                p_whole->p_vreg = NULL;
        }
    }
}
typedef struct whole_value_tmp whole_value_tmp, *p_whole_value_tmp;
typedef struct offset_value_tmp offset_value_tmp, *p_offset_value_tmp;
typedef struct param_value_pair param_value_pair, *p_param_value_pair;
typedef struct whole_zero whole_zero, *p_whole_zero;
struct whole_zero {
    p_ir_varray_bb_param p_varray_param;
    list_head node;
};
struct param_value_pair {
    p_ir_varray_bb_param p_varray_param;
    p_ir_vreg p_value;
    list_head node;
};
struct offset_value_tmp {
    size_t offset;
    p_ir_vreg p_vreg;
    list_head node;
    list_head param_value_list;
};
struct whole_value_tmp {
    p_symbol_type p_type;
    bool is_stack_ptr;
    list_head offset_value_list;
};
static inline p_whole_value_tmp whole_info_tmp_gen(p_whole_value p_whole_phi) {
    p_whole_value_tmp p_info_tmp = malloc(sizeof(*p_info_tmp));
    p_info_tmp->offset_value_list = list_head_init(&p_info_tmp->offset_value_list);
    p_info_tmp->p_type = NULL;
    return p_info_tmp;
}
static inline void offset_value_tmp_add(p_whole_value_tmp p_tmp, p_offset_value_tmp p_value_tmp) {
    // hlist_node_add(p_tmp->hash + get_tag(p_value_tmp->offset, p_value_tmp->p_vreg), &p_value_tmp->h_node);
    list_add_prev(&p_value_tmp->node, &p_tmp->offset_value_list);
}
static inline p_offset_value_tmp offset_value_tmp_gen(size_t offset, p_ir_vreg p_base) {
    p_offset_value_tmp p_tmp = malloc(sizeof(*p_tmp));
    p_tmp->node = list_head_init(&p_tmp->node);
    p_tmp->offset = offset;
    p_tmp->p_vreg = p_base;
    p_tmp->param_value_list = list_head_init(&p_tmp->param_value_list);
    return p_tmp;
}
static inline p_param_value_pair param_value_pair_vreg_gen(p_ir_varray_bb_param p_param, p_ir_vreg p_vreg) {
    p_param_value_pair p_pair = malloc(sizeof(*p_pair));
    p_pair->node = list_head_init(&p_pair->node);
    p_pair->p_varray_param = p_param;
    p_pair->p_value = p_vreg;
    return p_pair;
}
static inline void offset_value_tmp_drop(p_offset_value_tmp p_value_tmp) {
    list_del(&p_value_tmp->node);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_value_tmp->param_value_list) {
        p_param_value_pair p_pair = list_entry(p_node, param_value_pair, node);
        free(p_pair);
    }
    free(p_value_tmp);
}
static inline void whole_info_tmp_drop(p_whole_value_tmp p_info) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_info->offset_value_list) {
        p_offset_value_tmp p_value_tmp = list_entry(p_node, offset_value_tmp, node);
        offset_value_tmp_drop(p_value_tmp);
    }
    free(p_info);
}
static inline bool if_have_in_tmp(p_whole_value_tmp p_whole_tmp, size_t offset, p_ir_vreg p_base) {
    p_list_head p_node;
    list_for_each(p_node, &p_whole_tmp->offset_value_list) {
        p_offset_value_tmp p_value = list_entry(p_node, offset_value_tmp, node);
        if (p_value->offset == offset && p_value->p_vreg == p_base)
            return true;
    }
    return false;
}
static inline void whole_value_tmp_init(p_whole_value_tmp p_tmp, p_ir_varray_bb_phi p_bb_phi) {
    p_whole_value p_whole = get_whole_value(p_bb_phi->p_varray_phi, p_bb_phi->p_basic_block);
    assert(p_whole->is_array);
    p_list_head p_node;
    list_for_each(p_node, &p_bb_phi->p_varray_phi->use_list) {
        p_ir_varray_use p_use = list_entry(p_node, ir_varray_use, node);
        if (p_use->varray_use_type == varray_instr_use) {
            if (p_use->p_instr->irkind == ir_load) {
                p_ir_vreg p_base = NULL;
                size_t offset = 0;
                if (p_use->p_instr->ir_load.p_addr->kind == reg)
                    p_base = p_use->p_instr->ir_load.p_addr->p_vreg;
                else {
                    assert(p_bb_phi->p_varray_phi->p_base->is_vmem);
                    assert(p_use->p_instr->ir_load.p_addr->p_vmem == p_bb_phi->p_varray_phi->p_base->p_vmem_base);
                    offset = p_use->p_instr->ir_load.p_addr->offset;
                }
                p_offset_value p_find = find_offset_value(p_whole->hash, offset, p_base);
                if (p_find && p_find->if_created_phi) continue;
                if (if_have_in_tmp(p_tmp, offset, p_base)) continue;
                p_tmp->p_type = p_use->p_instr->ir_load.p_des->p_type;
                p_tmp->is_stack_ptr = p_use->p_instr->ir_load.is_stack_ptr;
                p_offset_value_tmp p_value_tmp = offset_value_tmp_gen(offset, p_base);
                offset_value_tmp_add(p_tmp, p_value_tmp);
            }
        }
    }
}
static inline bool if_stack(p_ir_varray p_varray) {

    assert(p_varray->p_base->is_vmem);
    return !p_varray->p_base->p_vmem_base->is_global;
}
static inline bool if_dom_tail(p_ir_vreg p_vreg, p_ir_basic_block p_block) {
    p_ir_basic_block p_def_block = NULL;
    switch (p_vreg->def_type) {
    case func_param_def:
        return true;
    case bb_phi_def:
        p_def_block = p_vreg->p_bb_phi->p_basic_block;
        if (p_block == p_def_block) {
            return true;
        }
        return ir_basic_block_dom_check(p_block, p_def_block);
    case instr_def:
        p_def_block = p_vreg->p_instr_def->p_basic_block;
        if (p_block == p_def_block) {
            return true;
        }
        return ir_basic_block_dom_check(p_block, p_def_block);
    }
}
static inline void whole_value_tmp_merge_bb_param(p_whole_value_tmp p_tmp, p_ir_varray_bb_param p_param) {
    p_whole_value p_whole = get_whole_value(p_param->p_varray_bb_param->p_varray_use, p_param->p_target->p_source_block);
    p_ir_basic_block p_source_block = p_param->p_target->p_source_block;
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_tmp->offset_value_list) {
        assert(p_tmp->p_type);
        p_offset_value_tmp p_offset = list_entry(p_node, offset_value_tmp, node);
        p_ir_vreg p_value = NULL;
        bool is_zero = false;
        bool is_success;
        p_offset_value p_find = find_offset_value(p_whole->hash, p_offset->offset, p_offset->p_vreg);
        if (p_find) {
            p_value = p_find->p_value;
            is_success = true;
        }
        else {

            // 尝试能否 load
            if (p_offset->p_vreg) {
                if (p_whole->is_zero && list_head_alone(&p_whole->offset_value_list)) { // 值全为0
                    is_zero = true;
                    is_success = true;
                }
                // 必须被地址定义支配
                else if (if_dom_tail(p_offset->p_vreg, p_source_block)) {
                    p_ir_vreg p_new = ir_vreg_gen(symbol_type_copy(p_tmp->p_type));
                    symbol_func_vreg_add(p_source_block->p_func, p_new);
                    p_ir_instr p_new_load = ir_load_instr_gen(ir_operand_vreg_gen(p_offset->p_vreg), p_new, p_tmp->is_stack_ptr);
                    ir_load_instr_set_varray_src(p_new_load, ir_varray_use_gen(p_param->p_varray_bb_param->p_varray_use));
                    ir_basic_block_addinstr_tail(p_source_block, p_new_load);
                    p_offset_value p_new_offset = offset_value_gen(p_offset->offset, p_offset->p_vreg, p_new);
                    offset_value_add(p_new_offset, p_whole);
                    p_value = p_new;
                    is_success = true;
                }
                else {
                    is_success = false;
                }
            }
            else {
                if (p_whole->is_zero) { // 值全为0
                    is_zero = true;
                    is_success = true;
                }
                else {
                    assert(p_param->p_varray_bb_param->p_varray_use->p_base->is_vmem);
                    p_symbol_var p_vmem_base = p_param->p_varray_bb_param->p_varray_use->p_base->p_vmem_base;
                    p_ir_vreg p_new = ir_vreg_gen(symbol_type_copy(p_tmp->p_type));
                    symbol_func_vreg_add(p_source_block->p_func, p_new);
                    p_ir_instr p_new_load = ir_load_instr_gen(ir_operand_addr_gen(p_vmem_base, NULL, p_offset->offset), p_new, p_tmp->is_stack_ptr);
                    ir_load_instr_set_varray_src(p_new_load, ir_varray_use_gen(p_param->p_varray_bb_param->p_varray_use));
                    ir_basic_block_addinstr_tail(p_source_block, p_new_load);
                    p_offset_value p_new_offset = offset_value_gen(p_offset->offset, p_offset->p_vreg, p_new);
                    offset_value_add(p_new_offset, p_whole);
                    p_value = p_new;
                    is_success = true;
                }
            }
        }
        if (!is_success) {
            assert(!is_zero);
            assert(!p_value);
            offset_value_tmp_drop(p_offset);
        }
        else {
            p_param_value_pair p_pair = NULL;
            if (is_zero) {
                assert(!p_value);
                p_ir_vreg p_value = ir_vreg_gen(symbol_type_var_gen(type_i32));
                p_ir_instr p_assign = ir_unary_instr_gen(ir_val_assign, ir_operand_int_gen(0), p_value);
                ir_basic_block_addinstr_tail(p_source_block, p_assign);
                symbol_func_vreg_add(p_source_block->p_func, p_value);
                p_pair = param_value_pair_vreg_gen(p_param, p_value);
            }
            else {
                assert(p_value);
                p_pair = param_value_pair_vreg_gen(p_param, p_value);
            }
            list_add_prev(&p_pair->node, &p_offset->param_value_list);
        }
    }
}
static inline void whole_value_tmp_whole_value(p_whole_value p_des, p_whole_value_tmp p_tmp, p_ir_varray_bb_phi p_bb_phi) {
    p_list_head p_node;
    list_for_each(p_node, &p_tmp->offset_value_list) {
        assert(p_tmp->p_type);
        p_offset_value_tmp p_value_tmp = list_entry(p_node, offset_value_tmp, node);
        assert(!list_head_alone(&p_value_tmp->param_value_list));
        p_ir_vreg p_phi = ir_vreg_gen(symbol_type_copy(p_tmp->p_type));
        symbol_func_vreg_add(p_bb_phi->p_basic_block->p_func, p_phi);
        ir_basic_block_add_phi(p_bb_phi->p_basic_block, p_phi);
        p_offset_value p_offset = find_offset_value(p_des->hash, p_value_tmp->offset, p_value_tmp->p_vreg);
        if (p_offset) {
            assert(!p_offset->if_created_phi);
            p_offset->p_value = p_phi;
        }
        else {
            p_offset = offset_value_gen(p_value_tmp->offset, p_value_tmp->p_vreg, p_phi);
            offset_value_add(p_offset, p_des);
        }
        p_list_head p_pair_node;
        list_for_each(p_pair_node, &p_value_tmp->param_value_list) {
            p_param_value_pair p_pair = list_entry(p_pair_node, param_value_pair, node);
            p_ir_operand p_operand = ir_operand_vreg_gen(p_pair->p_value);
            print_phi_param_array(p_bb_phi, p_pair->p_varray_param, p_phi, p_offset, p_operand);
            ir_basic_block_branch_target_add_param(p_pair->p_varray_param->p_target, p_operand);
        }
        p_offset->if_created_phi = true;
    }
}
static inline bool deal_phi_not_array(p_ir_varray_bb_phi p_varray_phi, p_whole_value p_whole) {
    assert(!p_whole->is_array);
    if (p_whole->if_created_phi) return false; // 已经生成过 phi
    p_ir_varray p_varray = p_varray_phi->p_varray_phi;
    if (list_head_alone(&p_varray->use_list)) return false;
    p_ir_basic_block p_basic_block = p_varray_phi->p_basic_block;
    list_head param_pair = list_head_init(&param_pair);
    p_list_head p_param_node;
    list_for_each(p_param_node, &p_varray_phi->varray_param_list) {
        p_ir_varray_bb_param p_param = list_entry(p_param_node, ir_varray_bb_param, phi_node);
        p_whole_value p_param_info = get_whole_value(p_param->p_varray_bb_param->p_varray_use, p_param->p_target->p_source_block);
        assert(!p_param_info->is_array);
        if (!p_param_info->p_vreg) {
            assert(p_varray->p_base->is_vmem);
            p_ir_vreg p_new = ir_vreg_gen(symbol_type_copy(p_varray->p_base->p_vmem_base->p_type));
            symbol_func_vreg_add(p_basic_block->p_func, p_new);
            p_param_info->p_vreg = p_new;
            p_ir_instr p_new_load = ir_load_instr_gen(ir_operand_addr_gen(p_varray->p_base->p_vmem_base, NULL, 0), p_new, if_stack(p_varray));
            ir_load_instr_set_varray_src(p_new_load, ir_varray_use_gen(p_param->p_varray_bb_param->p_varray_use));
            ir_basic_block_addinstr_tail(p_param->p_target->p_source_block, p_new_load);
        }
        p_param_value_pair p_pair = param_value_pair_vreg_gen(p_param, p_param_info->p_vreg);
        list_add_prev(&p_pair->node, &param_pair);
    }
    p_ir_vreg p_phi = NULL;
    assert(!list_head_alone(&param_pair));
    p_phi = ir_vreg_gen(symbol_type_copy(p_varray->p_base->p_vmem_base->p_type));
    symbol_func_vreg_add(p_basic_block->p_func, p_phi);
    ir_basic_block_add_phi(p_basic_block, p_phi);
    p_whole->p_vreg = p_phi;
    p_whole->if_created_phi = true;

    p_list_head p_tmp_node, p_tmp_node_next;
    list_for_each_safe(p_tmp_node, p_tmp_node_next, &param_pair) {
        p_param_value_pair p_pair = list_entry(p_tmp_node, param_value_pair, node);
        p_ir_operand p_param = ir_operand_vreg_gen(p_pair->p_value);
        print_phi_param_not_array(p_varray_phi, p_pair->p_varray_param, p_phi, p_param);
        ir_basic_block_branch_target_add_param(p_pair->p_varray_param->p_target, p_param);
        free(p_pair);
    }
    return true;
}
static inline bool deal_phi_array(p_ir_varray_bb_phi p_varray_phi, p_whole_value p_whole) {
    assert(p_whole->is_array);
    p_whole_value_tmp p_tmp = whole_info_tmp_gen(p_whole);
    whole_value_tmp_init(p_tmp, p_varray_phi);
    p_list_head p_param_node = p_varray_phi->varray_param_list.p_next;
    while (p_param_node != &p_varray_phi->varray_param_list) {
        p_ir_varray_bb_param p_param = list_entry(p_param_node, ir_varray_bb_param, phi_node);
        whole_value_tmp_merge_bb_param(p_tmp, p_param);
        p_param_node = p_param_node->p_next;
    }
    whole_value_tmp_whole_value(p_whole, p_tmp, p_varray_phi);
    bool if_success = !list_head_alone(&p_tmp->offset_value_list);

    whole_info_tmp_drop(p_tmp);
    return if_success;
}
static inline void deal_basic_block_instr(p_ir_basic_block p_basic_block, bool if_init) {
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->instr_list) {
        p_ir_instr p_instr = list_entry(p_node, ir_instr, node);
        switch (p_instr->irkind) {
        case ir_store:
            deal_store(p_instr, if_init);
            break;
        case ir_load:
            deal_load(p_instr);
            break;
        case ir_call:
            deal_call(p_instr);
            break;
        default:
            break;
        }
    }
}
static inline bool deal_basic_block(p_vmem_value_info_list p_list, p_ir_basic_block p_basic_block) {
    bool if_success = false;
    p_list_head p_node;
    list_for_each(p_node, &p_basic_block->varray_basic_block_phis) {
        p_ir_varray_bb_phi p_varray_phi = list_entry(p_node, ir_varray_bb_phi, node);
        p_whole_value p_whole = get_whole_value(p_varray_phi->p_varray_phi, p_basic_block);
        if (!p_whole->is_array) {
            if_success |= deal_phi_not_array(p_varray_phi, p_whole);
        }
        else
            if_success |= deal_phi_array(p_varray_phi, p_whole);
    }
    vmem_value_info_list_reset(p_list, p_basic_block);
    deal_basic_block_instr(p_basic_block, false);
    return if_success;
}
static inline void mem_copy_func(p_symbol_func p_func, p_program p_program) {
    if (list_head_alone(&p_func->block)) return;
    p_vmem_value_info_list p_list = vmem_value_info_list_gen(p_func, p_program);
    p_list_head rpo_seqs = get_rpo_seqs(p_func);
    p_list_head p_node;
    list_for_each(p_node, rpo_seqs) {
        p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, seqs_node);
        init_block_info(p_list, p_basic_block);
        deal_basic_block_instr(p_basic_block, true);
    }
    bool if_success = true;
    while (if_success) {
        if_success = false;
        list_for_each(p_node, rpo_seqs) {
            p_ir_basic_block p_basic_block = list_entry(p_node, ir_basic_block, seqs_node);
            if_success |= deal_basic_block(p_list, p_basic_block);
        }
    }
    symbol_func_set_block_id(p_func);
    vmem_value_info_list_drop(p_list);
    free(rpo_seqs);
}
void ir_opt_mem_copy_propagation(p_program p_program) {
    p_list_head p_node;
    ir_cfg_set_program_dom(p_program);
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        mem_copy_func(p_func, p_program);
    }
}
