#include <frontend.h>
#include <hir2mir.h>

#include <symbol_print.h>
#include <symbol_gen/store.h>

#include <mir_opt/simplify_cfg.h>
#include <optimizer.h>

int main(int argc, char *argv[]) {
    if (argc == 1)
        argv[argc++] = NULL;
    for (int i = 1; i < argc; ++i) {
        // gen hir
        p_program p_store = frontend_trans(argv[i]);
        symbol_store_hir_print(p_store);
        symbol_store_print(p_store);

        // gen mir
        hir2mir_program_gen(p_store);
        symbol_store_mir_print(p_store);

        // simplify cfg
        mir_simplify_cfg_pass(p_store);
        symbol_store_mir_print(p_store);

        // into ssa
        convert_ssa_program(p_store);
        symbol_store_mir_dom_info_print(p_store);
        symbol_store_mir_print(p_store);

        // drop mir
        symbol_store_mir_drop(p_store);
        symbol_store_drop(p_store);
    }
    return 0;
}
