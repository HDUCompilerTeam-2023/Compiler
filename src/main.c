#include <frontend/use.h>
#include <hir2mir.h>

#include <program/print.h>
#include <program/gen.h>

#include <mir_opt/simplify_cfg.h>
#include <optimizer.h>

int main(int argc, char *argv[]) {
    if (argc == 1)
        argv[argc++] = NULL;
    for (int i = 1; i < argc; ++i) {
        // gen ir
        p_program p_program = frontend_trans(argv[i]);
        program_variable_print(p_program);
        program_mir_print(p_program);

        // simplify cfg
        mir_simplify_cfg_pass(p_program);
        program_mir_print(p_program);

        // into ssa
        convert_ssa_program(p_program);
        program_mir_dom_info_print(p_program);
        program_mir_print(p_program);

        // drop mir
        program_mir_drop(p_program);
        program_drop(p_program);
    }
    return 0;
}
