#include <frontend/use.h>

#include <program/gen.h>
#include <program/print.h>

#include <ir_opt/deadcode_elimate.h>
#include <ir_opt/lir_gen/arm_trans.h>
#include <ir_opt/lir_gen/critical_edge_cut.h>
#include <ir_opt/lir_gen/set_cond.h>
#include <ir_opt/lir_gen/share_trans.h>
#include <ir_opt/mem2reg.h>
#include <ir_opt/reg_alloca/reg_alloca.h>
#include <ir_opt/simplify_cfg.h>
#include <ir_opt/lir_gen/delete_cmp.h>
#include <ir_opt/lir_gen/update_call_live.h>
#include <backend/arm/codegen.h>
#include <stdio.h>

static inline char *get_prefix(char *file) {
    char *output = malloc(sizeof(*file) * (strlen(file) * 2));
    char *p1 = output;
    char *p2 = file;
    while (*p2 != '.' && *p2 != '\0') {
        *p1 = *p2;
        p1++;
        p2++;
    }
    *(p1) = '\0';
    return output;
}

static inline void asm2file(char *output_file, char *asm_code) {
    FILE *file = fopen(output_file, "w");
    fputs(asm_code, file);
    fclose(file);
}

int main(int argc, char *argv[]) {
    if (argc == 1)
        argv[argc++] = NULL;
    for (int i = 1; i < argc; ++i) {
        char asm_code[100000] = "";
        char *output_file = NULL;
        int j = i;
        strcat(asm_code, ".file \"");
        if (i + 1 < argc && !strcmp(argv[i + 1], "-o")) {
            output_file = malloc(strlen(argv[i + 2]) + 1);
            strcpy(output_file, argv[i + 2]);
            strcat(asm_code, argv[i]);
            j += 2;
        }
        else {
            if (argv[i]) { // 默认目标与文件路径一致
                output_file = strcat(get_prefix(argv[i]), ".s");
                strcat(asm_code, argv[i]);
            }
            else { // 从输入读取
                output_file = malloc(sizeof(*output_file) * 20);
                strcpy(output_file, "./output.s");
                strcat(asm_code, "input");
            }
        }
        strcat(asm_code, "\"\n");
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

        critical_edge_cut_pass(p_program);
        program_ir_print(p_program);
        // set_cond
        set_cond_pass(p_program);
        program_ir_print(p_program);

        reg_alloca_pass(alloca_color_graph, 13, 32, p_program);
        program_ir_print(p_program);

        delete_cmp_pass(p_program);
        program_ir_print(p_program);

        update_call_live_pass(p_program);
        program_ir_print(p_program);

        arm_codegen_pass(p_program, asm_code);
        printf("%s", asm_code);
        // drop ir
        program_drop(p_program);
        asm2file(output_file, asm_code);
        i = j;
        free(output_file);
    }
    return 0;
}
