#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <ir_opt/simplify_cfg.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/lir_gen/share_trans.h>
#include <ir_opt/lir_gen/arm_trans.h>
#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/lir_gen/set_cond.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>

#include <backend/arm/codegen.h>
#include <stdio.h>
int main(int argc, char *argv[]) {
    if (argc == 1)
        argv[argc++] = NULL;
    for (int i = 1; i < argc; ++i) {
        char asm_code[100000] = "";
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

        // arm lir trans
        arm_lir_trans_pass(p_program);
        program_ir_print(p_program);
        
        reg_alloca_pass(alloca_whole_in_mem, 13, p_program);
        program_ir_print(p_program);
        
        critical_edge_cut_pass(p_program);
        program_ir_print(p_program);
        // set_cond
        set_cond_pass(p_program);
        program_ir_print(p_program);
        arm_codegen_pass(p_program, asm_code);
        printf("%s", asm_code);
        // drop ir
        program_drop(p_program);
    }
    return 0;
}
