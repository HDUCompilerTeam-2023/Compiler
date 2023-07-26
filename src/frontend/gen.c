#include <frontend/lexer.h>
#include <frontend/parser.h>
#include <frontend/syntax/info/gen.h>

#include <program/gen.h>

p_program frontend_trans(const char *input, const char *output) {
    yyscan_t scanner;
    p_syntax_info p_info = syntax_info_gen(input, output);
    yylex_init_extra(p_info, &scanner);
    if (input) {
        yyrestart(fopen(input, "r"), scanner);
    }

    yyparse(scanner);
    p_program p_program = syntax_info_get_program(p_info);
    syntax_info_drop(p_info);
    program_global_set_id(p_program);

    yylex_destroy(scanner);
    return p_program;
}
