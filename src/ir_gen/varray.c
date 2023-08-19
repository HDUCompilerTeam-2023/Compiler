#include <ir_gen.h>
#include <ir_gen/varray.h>
#include <symbol_gen.h>
p_ir_vmem_base ir_vmem_base_vmem_gen(p_symbol_var p_var) {
    p_ir_vmem_base p_base = malloc(sizeof(*p_base));
    p_base->is_vmem = true;
    p_base->num = 0;
    p_base->p_vmem_base = p_var;
    p_var->p_base = p_base;
    p_base->varray_list = list_head_init(&p_base->varray_list);
    return p_base;
}
p_ir_vmem_base ir_vmem_base_param_gen(p_ir_param_vmem_base p_param_base) {
    p_ir_vmem_base p_base = malloc(sizeof(*p_base));
    p_base->is_vmem = false;
    p_base->num = 0;
    p_base->p_param_base = p_param_base;
    p_base->varray_list = list_head_init(&p_base->varray_list);
    return p_base;
}
p_ir_param_vmem_base ir_param_vmem_base_gen(p_ir_vreg p_vreg, p_symbol_func p_func) {
    p_ir_param_vmem_base p_param_base = malloc(sizeof(*p_param_base));
    p_param_base->p_param_base = ir_vmem_base_param_gen(p_param_base);
    p_param_base->node = list_head_init(&p_param_base->node);
    p_param_base->p_func = p_func;
    p_param_base->id = p_func->param_vmem_base_num++;
    p_param_base->p_vreg = p_vreg;
    return p_param_base;
}
static inline void ir_vmem_base_add_varray(p_ir_vmem_base p_base, p_ir_varray p_varray) {
    p_varray->p_base = p_base;
    if (p_base->is_vmem) {
        if (p_base->p_vmem_base->is_global)
            p_base->p_vmem_base->p_program->varray_num++;
        else
            p_base->p_vmem_base->p_func->varray_num++;
    }
    else {
        assert(p_base->p_param_base->p_vreg->def_type == func_param_def);
        p_base->p_param_base->p_func->varray_num++;
    }
    assert(list_add_prev(&p_varray->node, &p_base->varray_list));
}
p_ir_varray ir_varray_gen(p_ir_vmem_base p_base) {
    p_ir_varray p_varray = malloc(sizeof(*p_varray));
    p_varray->id = p_base->num++;
    p_varray->node = list_head_init(&p_varray->node);
    p_varray->use_list = list_head_init(&p_varray->use_list);
    p_varray->p_instr_def = NULL;
    ir_vmem_base_add_varray(p_base, p_varray);
    return p_varray;
}
p_ir_varray ir_varray_copy(p_ir_varray p_src) {
    p_ir_varray p_varray = malloc(sizeof(*p_varray));
    return ir_varray_gen(p_src->p_base);
}
p_ir_varray_use ir_varray_use_gen(p_ir_varray p_varray) {
    p_ir_varray_use p_use = malloc(sizeof(*p_use));
    p_use->node = list_head_init(&p_use->node);
    p_use->p_varray_use = p_varray;
    list_add_prev(&p_use->node, &p_varray->use_list);
    return p_use;
}
void ir_varray_use_reset_varray(p_ir_varray_use p_use, p_ir_varray p_varray) {
    if (p_use->p_varray_use == p_varray)
        return;
    list_del(&p_use->node);
    p_use->p_varray_use = p_varray;
    list_add_prev(&p_use->node, &p_varray->use_list);
}
p_ir_varray_def_pair ir_varray_def_pair_gen(p_ir_varray p_des, p_ir_varray_use p_src) {
    p_ir_varray_def_pair p_pair = malloc(sizeof(*p_pair));
    p_pair->node = list_head_init(&p_pair->node);
    p_pair->p_des = NULL;
    p_pair->p_src = NULL;
    if (p_des)
        ir_varray_def_pair_set_des(p_pair, p_des);
    assert(p_src);
    ir_varray_set_def_pair_use(p_src, p_pair);
    p_pair->p_src = p_src;
    return p_pair;
}
void ir_varray_set_instr_def(p_ir_varray p_varray, p_ir_instr p_instr) {
    p_varray->varray_def_type = varray_instr_def;
    p_varray->p_instr_def = p_instr;
}
void ir_varray_set_def_pair_def(p_ir_varray p_varray, p_ir_varray_def_pair p_pair) {
    p_varray->varray_def_type = varray_def_pair_def;
    p_varray->p_pair = p_pair;
}
void ir_varray_set_func_def(p_ir_varray p_varray, p_symbol_func p_func) {
    p_varray->varray_def_type = varray_func_def;
    p_varray->p_func = p_func;
}
void ir_varray_set_bb_phi_def(p_ir_varray p_varray, p_ir_varray_bb_phi p_varray_bb_phi) {
    p_varray->varray_def_type = varray_bb_phi_def;
    p_varray->p_varray_bb_phi = p_varray_bb_phi;
}
void ir_varray_set_global_def(p_ir_varray p_varray) {
    p_varray->varray_def_type = varray_global_def;
}
void ir_varray_set_instr_use(p_ir_varray_use p_varray_use, p_ir_instr p_instr) {
    p_varray_use->varray_use_type = varray_instr_use;
    p_varray_use->p_instr = p_instr;
}
void ir_varray_set_def_pair_use(p_ir_varray_use p_varray_use, p_ir_varray_def_pair p_pair) {
    p_varray_use->varray_use_type = varray_def_pair_use;
    p_varray_use->p_pair = p_pair;
}
void ir_varray_set_bb_parram_use(p_ir_varray_use p_varray_use, p_ir_varray_bb_param p_ir_varray_bb_param) {
    p_varray_use->varray_use_type = varray_bb_param_use;
    p_varray_use->p_varray_param = p_ir_varray_bb_param;
}

void ir_vmem_base_set_varray_id(p_ir_vmem_base p_base) {
    p_list_head p_node;
    size_t id = 0;
    list_for_each(p_node, &p_base->varray_list) {
        p_ir_varray p_varray = list_entry(p_node, ir_varray, node);
        p_varray->id = id++;
    }
    assert(id == p_base->num);
}
void ir_vmem_base_clear(p_ir_vmem_base p_base) {
    assert(p_base);
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_base->varray_list) {
        p_ir_varray p_varray = list_entry(p_node, ir_varray, node);
        ir_varray_drop(p_varray);
    }
}
void ir_vmem_base_clear_all(p_ir_vmem_base p_base) {
    p_list_head p_node, p_next;
    list_for_each_safe(p_node, p_next, &p_base->varray_list) {
        p_ir_varray p_varray = list_entry(p_node, ir_varray, node);
        p_list_head p_node, p_next;
        list_for_each_safe(p_node, p_next, &p_varray->use_list) {
            p_ir_varray_use p_use = list_entry(p_node, ir_varray_use, node);
            p_ir_instr p_use_instr;
            switch (p_use->varray_use_type) {
            case varray_instr_use:
                p_use_instr = p_use->p_instr;
                assert(p_use_instr);
                switch (p_use->p_instr->irkind) {
                case ir_load:
                    ir_load_instr_set_varray_src(p_use_instr, NULL);
                    break;
                case ir_store:
                    ir_store_instr_set_varray_des(p_use_instr, NULL);
                    ir_store_instr_set_varray_src(p_use_instr, NULL);
                    break;
                default:
                    assert(0);
                    break;
                }
            case varray_bb_param_use:
                break;
            case varray_def_pair_use:
                assert(p_use->p_pair);
                ir_varray_def_pair_drop(p_use->p_pair);
                break;
            }
        }
        if (p_varray->varray_def_type == varray_bb_phi_def) {
            assert(p_varray->p_varray_bb_phi);
            printf("fsd\n");
            ir_varray_bb_phi_drop(p_varray->p_varray_bb_phi);
        }
    }
    ir_vmem_base_clear(p_base);
}

void ir_vmem_base_drop(p_ir_vmem_base p_base) {
    ir_vmem_base_clear(p_base);
    free(p_base);
}
void ir_varray_use_drop(p_ir_varray_use p_use) {
    list_del(&p_use->node);
    free(p_use);
}
void ir_varray_drop(p_ir_varray p_varray) {
    assert(p_varray->varray_def_type != varray_bb_phi_def);
    if (p_varray->varray_def_type == varray_instr_def)
        assert(!p_varray->p_instr_def);
    assert(list_head_alone(&p_varray->use_list));
    list_del(&p_varray->node);
    p_varray->p_base->num--;
    if (p_varray->p_base->is_vmem) {
        if (p_varray->p_base->p_vmem_base->is_global)
            p_varray->p_base->p_vmem_base->p_program->varray_num--;
        else
            p_varray->p_base->p_vmem_base->p_func->varray_num--;
    }
    else {
        //        assert(p_varray->p_base->p_param_base->p_vreg->def_type == func_param_def);
        p_varray->p_base->p_param_base->p_func->varray_num--;
    }
    free(p_varray);
}
void ir_varray_def_pair_set_des(p_ir_varray_def_pair p_def_pair, p_ir_varray p_varray) {
    if (p_def_pair->p_des)
        ir_varray_set_instr_def(p_def_pair->p_des, NULL);
    if (p_varray)
        ir_varray_set_def_pair_def(p_varray, p_def_pair);
    p_def_pair->p_des = p_varray;
}
void ir_varray_def_pair_set_src(p_ir_varray_def_pair p_def_pair, p_ir_varray_use p_varray_use) {
    assert(p_def_pair->p_src);
    assert(p_varray_use);
    ir_varray_use_drop(p_def_pair->p_src);
    ir_varray_set_def_pair_use(p_varray_use, p_def_pair);
    p_def_pair->p_src = p_varray_use;
}
void ir_varray_def_pair_drop(p_ir_varray_def_pair p_def_pair) {
    list_del(&p_def_pair->node);
    // def_pair des 为空时表示调用函数内只有 load 或者 有store 但在之后未被使用, 但src必定存在
    assert(p_def_pair->p_src);
    if (p_def_pair->p_des) {
        assert(p_def_pair->p_des->varray_def_type == varray_def_pair_def);
        assert(p_def_pair->p_des->p_pair == p_def_pair);
        ir_varray_set_instr_def(p_def_pair->p_des, NULL);
    }
    assert(p_def_pair->p_src->varray_use_type == varray_def_pair_use);
    assert(p_def_pair->p_src->p_pair == p_def_pair);
    ir_varray_use_drop(p_def_pair->p_src);
    free(p_def_pair);
}
void ir_param_vmem_base_drop(p_ir_param_vmem_base p_base) {
    ir_vmem_base_drop(p_base->p_param_base);
    list_del(&p_base->node);
    free(p_base);
}

void clear_var_varray(p_list_head p_head) {
    p_list_head p_node;
    list_for_each(p_node, p_head) {
        p_symbol_var p_var = list_entry(p_node, symbol_var, node);
        assert(p_var->p_base);
        ir_vmem_base_clear_all(p_var->p_base);
    }
}
void clear_param_varray(p_symbol_func p_func) {
    p_list_head p_node;
    list_for_each(p_node, &p_func->param_vmem_base) {
        p_ir_param_vmem_base p_param = list_entry(p_node, ir_param_vmem_base, node);
        assert(p_param->p_param_base);
        ir_vmem_base_clear_all(p_param->p_param_base);
    }
}
void clear_all(p_program p_ir){
    clear_var_varray(&p_ir->variable);
    p_list_head p_node;
    list_for_each(p_node, &p_ir->function){
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        clear_var_varray(&p_func->variable);
        clear_param_varray(p_func);
    }
}
