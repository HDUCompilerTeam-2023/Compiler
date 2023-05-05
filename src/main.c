#include <frontend.h>
#include <hir2mir.h>
#include <mir_manager.h>

#include <hir_print.h>
#include <mir_print.h>
#include <optimizer.h>
#include <mir_opt/simplify_cfg.h>

int main(int argc, char *argv[]) {
    if (argc == 1)
        argv[argc++] = NULL;
    for (int i = 1; i < argc; ++i) {
        // gen hir
        p_hir_program p_hir = frontend_trans(argv[i]);
        hir_program_print(p_hir);

        // gen mir
        p_mir_program p_mir = hir2mir_program_gen(p_hir);
        mir_program_print(p_mir);

        // simplify cfg
        mir_simplify_cfg_pass(p_mir);
        mir_program_print(p_mir);

        // into ssa
        convert_ssa_program(p_mir);
        mir_program_dom_info_print(p_mir);
        mir_program_print(p_mir);

        // drop mir
        mir_program_drop(p_mir);
    }
    return 0;
}