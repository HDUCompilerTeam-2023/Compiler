#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <backend/arm/ir_arm_asm.h>
#include <ir_manager/set_cond.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/lir_gen/arm_imme_trans.h>
#include <ir_opt/lir_gen/arm_param_trans.h>
#include <ir_opt/lir_gen/arm_trans_after.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <ir_opt/lir_gen/delete_cmp.h>
#include <ir_opt/lir_gen/share_trans.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/simplify_cfg.h>
#include <ir_opt/copy_propagation.h>
#include <ir_opt/reassociate.h>
#include <ir_opt/globalopt.h>
#include <ir_opt/inline.h>
#include <ir_opt/sccp.h>
#include <ir_opt/gcm.h>
#include <ir_opt/gvn.h>
#include <backend/arm/arm_delete_non_use_block.h>
#include <stdio.h>
#include <ir_opt/mem_copy_propagation.h>
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
    ir_opt_mem_copy_propagation(p_program);
    ir_deadcode_elimate_pass(p_program, true);

    size_t n = 3;
    do {
        ir_opt_inline(p_program);

        ir_opt_globalopt(p_program);
        mem2reg_program_pass(p_program);
        ir_deadcode_elimate_pass(p_program, true);

        // optimize - need keep block information
        if (is_opt) {
            ir_side_effects(p_program);
            set_cond_pass(p_program);
            ir_reassociate(p_program);
            ir_opt_gvn(p_program);
            ir_opt_gcm(p_program);
            ir_deadcode_elimate_pass(p_program, true);
        }
        ir_opt_copy_propagation(p_program);
        ir_opt_sccp(p_program);

        // deadcode elimate
        ir_deadcode_elimate_pass(p_program, true);
    } while(n--);

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
    arm_delete_non_use_block(p_program);
    program_arm_asm_output(p_program);
    // drop ir
    program_ir_print(p_program);
    program_drop(p_program);
    return 0;
}
