#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <backend/arm/codegen.h>
#include <ir_manager/set_cond.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/lir_gen/arm_trans.h>
#include <ir_opt/lir_gen/arm_trans_after.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <ir_opt/lir_gen/delete_cmp.h>
#include <ir_opt/lir_gen/share_trans.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/simplify_cfg.h>
#include <stdio.h>

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

    // simplify cfg
    ir_simplify_cfg_pass(p_program);

    // into ssa
    mem2reg_program_pass(p_program);
    program_ir_dom_info_print(p_program);

    // deadcode elimate
    ir_deadcode_elimate_pass(p_program, true);

    // shared lir trans
    share_lir_trans_pass(p_program);

    // arm lir trans
    arm_lir_trans_pass(p_program);

    // set_cond
    set_cond_pass(p_program);

    reg_alloca_pass(alloca_color_graph, 13, 32, p_program);

    critical_edge_cut_pass(p_program);

    delete_cmp_pass(p_program);

    arm_trans_after_pass(p_program);
    program_ir_print(p_program);

    arm_codegen_pass(p_program);
    // drop ir
    program_drop(p_program);
    return 0;
}
