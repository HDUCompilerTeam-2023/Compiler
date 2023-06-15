#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <ir_opt/simplify_cfg.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/lir_gen/share_trans.h>

int main(int argc, char *argv[]) {
    if (argc == 1)
        argv[argc++] = NULL;
    for (int i = 1; i < argc; ++i) {
        // gen ir
        p_program p_program = frontend_trans(argv[i]);
        program_variable_print(p_program);
        program_ir_print(p_program);

        // simplify cfg
        ir_simplify_cfg_pass(p_program);
        program_ir_print(p_program);

        // into ssa
        mem2reg_program_pass(p_program);
        program_ir_dom_info_print(p_program);
        program_ir_print(p_program);

        // deadcode elimate
        ir_deadcode_elimate_pass(p_program, true);
        program_ir_print(p_program);

        // shared lir trans
        share_lir_trans_pass(p_program);
        program_ir_print(p_program);

        // drop ir
        program_drop(p_program);
    }
    return 0;
}
