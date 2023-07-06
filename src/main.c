#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <backend/arm/codegen.h>
#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/lir_gen/arm_trans.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <ir_opt/lir_gen/delete_cmp.h>
#include <ir_opt/lir_gen/set_cond.h>
#include <ir_opt/lir_gen/share_trans.h>
#include <ir_opt/lir_gen/arm_trans_after.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/simplify_cfg.h>
#include <stdio.h>

static inline void asm2file(char *output_file, char *asm_code) {
    FILE *file = fopen(output_file, "w");
    fputs(asm_code, file);
    fclose(file);
}

int main(int argc, char *argv[]) {
    char *in_file = NULL, *out_file = NULL;
    bool is_opt = false, is_shell = false;
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
        assert(!in_file);
        assert(argv[i][0] != '-');
        in_file = argv[i];
    }
    if (!in_file) {
        in_file = "input";
        is_shell = true;
    }
    if (!out_file) {
        out_file = "output.s";
    }

    char asm_code[100000] = "";
    strcat(asm_code, ".file \"");
    strcat(asm_code, in_file);
    strcat(asm_code, "\"\n");

    // gen ir
    p_program p_program = frontend_trans(is_shell ? NULL : in_file);
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

    // set_cond
    set_cond_pass(p_program);
    program_ir_print(p_program);

    reg_alloca_pass(alloca_color_graph, 12, 32, p_program);
    program_ir_print(p_program);

    critical_edge_cut_pass(p_program);
    program_ir_print(p_program);

    delete_cmp_pass(p_program);
    program_ir_print(p_program);

    arm_trans_after_pass(p_program);
    program_ir_print(p_program);

    arm_codegen_pass(p_program, asm_code);
    printf("%s", asm_code);
    // drop ir
    program_drop(p_program);
    asm2file(out_file, asm_code);
    return 0;
}
