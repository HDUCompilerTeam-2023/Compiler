#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <ir_opt/simplify_cfg.h>
#include <optimizer.h>

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
        convert_ssa_program(p_program);
        program_ir_dom_info_print(p_program);
        program_ir_print(p_program);

        // drop ir
        program_drop(p_program);
    }
    return 0;
}
