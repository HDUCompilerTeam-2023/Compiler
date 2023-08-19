#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <backend/arm/ir_arm_asm.h>
#include <ir_manager/loop_normalization.h>
#include <ir_manager/memssa.h>
#include <ir_manager/set_cond.h>
#include <ir_opt/copy_propagation.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/gcm.h>
#include <ir_opt/globalopt.h>
#include <ir_opt/gvn.h>
#include <ir_opt/inline.h>
#include <ir_opt/lir_gen/arm_imme_trans.h>
#include <ir_opt/lir_gen/arm_param_trans.h>
#include <ir_opt/lir_gen/arm_trans.h>
#include <ir_opt/lir_gen/arm_trans_after.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <ir_opt/lir_gen/delete_cmp.h>
#include <ir_opt/lir_gen/share_trans.h>
#include <ir_opt/loop_unrolling.h>
#include <ir_opt/loop_unswitch.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/mem_copy_propagation.h>
#include <ir_opt/reassociate.h>
#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/sccp.h>
#include <ir_opt/scev.h>
#include <ir_opt/simplify_cfg.h>
#include <stdio.h>

#include <symbol_gen/func.h>

inline static bool _block_check(p_program p_program) {
    p_list_head p_node;
    list_for_each(p_node, &p_program->function) {
        p_symbol_func p_func = list_entry(p_node, symbol_func, node);
        assert(p_func->p_entry_block);
        printf("%s %ld bb\n", p_func->name, p_func->block_cnt);
        p_list_head p_block_node;
        list_for_each(p_block_node, &p_func->block) {
            p_ir_basic_block p_bb = list_entry(p_block_node, ir_basic_block, node);
            if (p_bb == p_func->p_entry_block) continue;
            if (!p_bb) return false;
            if (list_head_alone(&p_bb->prev_branch_target_list)) return false;
        }
    }
    return true;
}

int main(int argc, char *argv[]) {
    char *in_file = NULL, *out_file = NULL;
    bool is_opt = false;
    for (int i = 1; i < argc; ++i) {
        if (argv[i] && !strcmp(argv[i], "-o")) {
            assert(!out_file);
            assert(++i < argc);
            assert(argv[i][0] != '-');
            out_file = argv[i];
            continue;
        }
        if (argv[i] && !strcmp(argv[i], "-O1")) {
            assert(!is_opt);
            is_opt = true;
            continue;
        }
        if (argv[i] && !strcmp(argv[i], "-S")) {
            continue;
        }
        assert(!in_file);
        assert(argv[i][0] != '-');
        in_file = argv[i];
    }

    // gen ir
    p_program p_program = frontend_trans(in_file, out_file);
    program_variable_print(p_program);
    ir_build_call_graph(p_program);

    // into ssa
    ir_simplify_cfg_pass(p_program);
    mem2reg_program_pass(p_program);
    ir_deadcode_elimate_pass(p_program, true);

    size_t n = 3;
    do {
        printf("ddddd %zu\n", n);
        ir_opt_inline(p_program);
        assert(_block_check(p_program));

        ir_opt_globalopt(p_program);
        assert(_block_check(p_program));
        memssa_program_pass(p_program);
        assert(_block_check(p_program));
        ir_deadcode_elimate_pass(p_program, true);
        assert(_block_check(p_program));

        // optimize - need keep block information
        if (1) {
            ir_side_effects(p_program);
            assert(_block_check(p_program));
            set_cond_pass(p_program);
            assert(_block_check(p_program));
            ir_reassociate(p_program);
            assert(_block_check(p_program));
            ir_opt_gvn(p_program);
            assert(_block_check(p_program));
            ir_opt_gcm(p_program);
            assert(_block_check(p_program));
            // program_loop_normalization(p_program);
            // program_var_analysis(p_program,false);
            // ir_opt_loop_unswitch(p_program);
            program_loop_normalization(p_program);
            program_var_analysis(p_program, false);
            ir_opt_loop_full_unrolling(p_program);
            // program_loop_normalization(p_program);
            // program_var_analysis(p_program, false);
            // ir_opt_loop_full_unrolling(p_program);
            program_ir_print(p_program);
            printf("loop begin\n");
            program_loop_normalization(p_program);
            assert(_block_check(p_program));
            printf("loop end\n");
            program_ir_print(p_program);
            program_var_analysis(p_program, false);
            assert(_block_check(p_program));
            program_ir_print(p_program);
            ir_opt_loop_full_unrolling(p_program);
            assert(_block_check(p_program));

            program_loop_normalization(p_program);
            assert(_block_check(p_program));
            // ir_simplify_cfg_pass(p_program);
            memssa_program_pass(p_program);
            assert(_block_check(p_program));
            program_ir_print(p_program);

            ir_deadcode_elimate_pass(p_program, true);
            assert(_block_check(p_program));
        }
        program_ir_print(p_program);
        printf("into copy");
        ir_opt_copy_propagation(p_program);
        assert(_block_check(p_program));
        program_ir_print(p_program);

        ir_opt_mem_copy_propagation(p_program);
        assert(_block_check(p_program));
        program_ir_print(p_program);
        printf("into\n");
        ir_opt_sccp(p_program);
        assert(_block_check(p_program));

        // deadcode elimate
        ir_deadcode_elimate_pass(p_program, true);
        assert(_block_check(p_program));
    } while (n--);

    // shared lir trans
    share_lir_trans_pass(p_program);

    // arm lir trans
    arm_param_trans_pass(p_program);
    arm_imme_trans_pass(p_program);
    set_cond_pass(p_program);
    reg_alloca_pass(alloca_color_graph, 13, 32, p_program);
    arm_trans_after_pass(p_program);
    set_cond_pass(p_program);
    critical_edge_cut_pass(p_program);

    ir_arm_asm_pass(p_program);
    program_arm_asm_output(p_program);
    // drop ir
    program_ir_print(p_program);
    program_drop(p_program);
    return 0;
}
