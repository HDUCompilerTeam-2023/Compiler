/*
 * SysY.y : Parser for SysY language
 */

%define parse.error verbose
%param { yyscan_t yyscanner }

%{
#include <frontend/lexer.h>
#include <frontend/log.h>
#include <frontend/syntax/init/gen.h>
#include <frontend/syntax/decl_head/gen.h>
#include <frontend/syntax/decl/gen.h>

#include <ast_gen.h>

#define extra yyget_extra(yyscanner)

#define find_var(name) syntax_find_var(extra, name)
#define find_func(name) syntax_find_func(extra, name)
#define find_str(str) syntax_get_str(extra, str)
%}

%code requires{
#include <frontend/syntax/use.h>
}

%define api.pure full
%locations

%union {
       p_ast_block p_block;
       p_ast_stmt p_stmt;
       p_ast_exp p_exp;

       p_ast_param_list p_param_list;

       p_syntax_decl p_decl;
       p_syntax_decl_head p_decl_head;

       p_syntax_init p_init;

       basic_type type;

       char *ID;
       char *STRING;
       I32CONST_t I32CONST;
       F32CONST_t F32CONST;
}
%type <p_exp> Cond
%type <p_exp> LOrExp
%type <p_exp> LAndExp
%type <p_exp> EqExp
%type <p_exp> RelExp

%type <p_exp> Exp
%type <p_exp> ConstExp
%type <p_exp> AddExp
%type <p_exp> MulExp
%type <p_exp> UnaryExp
%type <p_exp> PrimaryExp

%type <p_exp> Call
%type <p_exp> Val
%type <p_exp> Str

%type <p_exp> StmtExp
%type <p_stmt> Stmt

%type <p_block> BlockItems
%type <p_block> Block

%type <p_param_list> FuncRParamList
%type <p_param_list> FuncRParams

%type <p_decl> ArraryParameter

%type <p_decl> Declarator
%type <p_decl> VarInitDeclarator
%type <p_decl_head> VarInitDeclaratorList
%type <p_decl> ConstInitDeclarator
%type <p_decl_head> ConstInitDeclaratorList

%type <p_init> VarInitializer
%type <p_init> VarInitializerList
%type <p_init> ConstInitializer
%type <p_init> ConstInitializerList

%type <type> Type

%locations

%token UNSIGNED SIGNED
%token LONG SHORT
%token INT FLOAT DOUBLE CHAR
%token VOID
%token CONST VOLATILE
%token DO WHILE FOR BREAK CONTINUE
%token IF ELSE
%token RETURN

%token AND OR LE GE EQ NEQ
%token SELFADD SELFSUB

%token <I32CONST> I32CONST
%token <F32CONST> F32CONST
%token <ID> ID
%destructor { free($$); } ID
%token <STRING> STRING
%destructor { free($$); } STRING

%nonassoc NO_ELSE
%nonassoc ELSE

%%
begin : PUSHZONE CompUnit POPZONE
      ;

CompUnit : CompUnit Declaration
         | CompUnit FuncDeclaration
         | CompUnit error
         | /* *empty */             { syntax_rtlib_func_init(extra); }
         ;

Type : INT   { $$ = type_i32; }
     | FLOAT { $$ = type_f32; }
     ;

Declaration : ConstDeclaration
            | VarDeclaration
            ;

ConstDeclaration : ConstInitDeclaratorList ';' { syntax_decl_head_drop($1); }
                 ;

VarDeclaration : VarInitDeclaratorList ';' { syntax_decl_head_drop($1); }
               ;

ConstInitDeclaratorList : ConstInitDeclaratorList ',' ConstInitDeclarator { $$ = syntax_declaration(extra, $1, $3); }
                        | CONST Type ConstInitDeclarator                  { $$ = syntax_declaration(extra, syntax_decl_head_gen($2, true), $3); }
                        ;

VarInitDeclaratorList : VarInitDeclaratorList ',' VarInitDeclarator { $$ = syntax_declaration(extra, $1, $3); }
                      | Type VarInitDeclarator                      { $$ = syntax_declaration(extra, syntax_decl_head_gen($1, false), $2); }
                      ;

ConstInitDeclarator : Declarator '=' ConstInitializer { $$ = syntax_decl_init($1, $3); }
                    ;

VarInitDeclarator : Declarator '=' VarInitializer { $$ = syntax_decl_init($1, $3); }
                  | Declarator                    { $$ = syntax_decl_init($1, NULL); }
                  ;

Declarator : Declarator '[' ConstExp ']' { $$ = syntax_decl_arr($1, $3); }
           | ID                          { $$ = syntax_decl_gen($1); }
           | Declarator error
           ;

ConstInitializer : '{' ConstInitializerList '}'     { $$ = $2; }
                 | '{' '}'                          { $$ = syntax_init_list_gen(); }
                 | ConstExp                         { $$ = syntax_init_exp_gen($1); }
                 ;

ConstInitializerList : ConstInitializerList ',' ConstInitializer { $$ = syntax_init_list_add($1, $3); }
                     | ConstInitializer                          { $$ = syntax_init_list_add(syntax_init_list_gen(), $1); }
                     ;

VarInitializer : '{' VarInitializerList '}'     { $$ = $2; }
               | '{' '}'                        { $$ = syntax_init_list_gen(); }
               | Exp                      { $$ = syntax_init_exp_gen($1); }
               ;

VarInitializerList : VarInitializerList ',' VarInitializer { $$ = syntax_init_list_add($1, $3); }
                   | VarInitializer                        { $$ = syntax_init_list_add(syntax_init_list_gen(), $1); }
                   ;

FuncHead : Type ID { syntax_func_head(extra, $1, $2); }
         | VOID ID { syntax_func_head(extra, type_void, $2); }
         ;

FuncDeclaration : FuncHead '(' Parameters ')' Block { syntax_func_end(extra, $5); }
                ;

Parameters : ParameterList
           | /* *empty */
           ;

ParameterList : ParameterList ',' ParameterDeclaration
              | ParameterDeclaration
              ;

ParameterDeclaration : Type ArraryParameter { p_syntax_decl_head p_head = syntax_decl_head_gen($1, false); syntax_declaration(extra, p_head, $2); syntax_decl_head_drop(p_head); }
                     | Type ID              { p_syntax_decl_head p_head = syntax_decl_head_gen($1, false); syntax_declaration(extra, p_head, syntax_decl_gen($2)); syntax_decl_head_drop(p_head); }
                     ;

ArraryParameter : ID '[' ']'                  { $$ = syntax_decl_arr(syntax_decl_gen($1), NULL); }
                | ArraryParameter '[' Exp ']' { $$ = syntax_decl_arr($1, $3); }
                ;

Cond : LOrExp
     ;

LOrExp : LOrExp OR LAndExp { $$ = ast_exp_logic_gen(ast_exp_op_bool_or, $1, $3); }
       | LAndExp
       ;

LAndExp : LAndExp AND EqExp { $$ = ast_exp_logic_gen(ast_exp_op_bool_and, $1, $3); }
        | EqExp             { $$ = ast_exp_to_cond($1); }
        ;

EqExp : EqExp EQ RelExp  { $$ = ast_exp_relational_gen(ast_exp_op_eq, $1, $3); }
      | EqExp NEQ RelExp { $$ = ast_exp_relational_gen(ast_exp_op_neq, $1, $3); }
      | RelExp
      ;

RelExp : RelExp '<' AddExp { $$ = ast_exp_relational_gen(ast_exp_op_l, $1, $3); }
       | RelExp '>' AddExp { $$ = ast_exp_relational_gen(ast_exp_op_g, $1, $3); }
       | RelExp LE AddExp  { $$ = ast_exp_relational_gen(ast_exp_op_leq, $1, $3); }
       | RelExp GE AddExp  { $$ = ast_exp_relational_gen(ast_exp_op_geq, $1, $3); }
       | AddExp
       ;

ConstExp : Exp { $$ = ast_exp_ptr_check_const($1); }
         ;

Exp : AddExp
    ;

AddExp : AddExp '+' MulExp { $$ = ast_exp_binary_gen(ast_exp_op_add, $1, $3); }
       | AddExp '-' MulExp { $$ = ast_exp_binary_gen(ast_exp_op_sub, $1, $3); }
       | MulExp
       ;

MulExp : MulExp '*' UnaryExp { $$ = ast_exp_binary_gen(ast_exp_op_mul, $1, $3); }
       | MulExp '/' UnaryExp { $$ = ast_exp_binary_gen(ast_exp_op_div, $1, $3); }
       | MulExp '%' UnaryExp { $$ = ast_exp_binary_gen(ast_exp_op_mod, $1, $3); }
       | UnaryExp
       ;

UnaryExp : '-' UnaryExp     { $$ = ast_exp_unary_gen(ast_exp_op_minus, $2); }
         | '+' UnaryExp     { $$ = $2; }
         | '!' UnaryExp     { $$ = ast_exp_ulogic_gen(ast_exp_op_bool_not, $2); }
         | PrimaryExp
         ;

PrimaryExp : '(' Exp ')' { $$ = $2; }
           | I32CONST    { $$ = ast_exp_int_gen($1); }
           | F32CONST    { $$ = ast_exp_float_gen($1); }
           | Val         { $$ = $1; }
           | Call        { $$ = $1; }
           | Str         { $$ = $1; }
           ;

Call : ID '(' FuncRParams ')' { $$ = ast_exp_call_gen(find_func($1), $3); free($1); }
     ;

Val : ID                 { $$ = ast_exp_ptr_gen(find_var($1)); free($1); }
    | Val '[' Exp ']'    { $$ = syntax_val_offset($1, $3); }
    ;

Str : STRING { $$ = ast_exp_str_gen(find_str($1)); free($1); }
    ;

FuncRParams : FuncRParamList { $$ = $1; }
            | /* *empty */   { $$ = ast_param_list_init(); }
            ;

FuncRParamList : FuncRParamList ',' Exp { $$ = ast_param_list_add($1, $3); }
               | Exp                    { $$ = ast_param_list_add(ast_param_list_init(), $1); }
               ;

Block : '{' BlockItems '}' { $$ = $2; syntax_set_block(extra, NULL); }
      ;

BlockItems : BlockItems Declaration
           | BlockItems Stmt           { $$ = ast_block_add($1, $2); syntax_set_block(extra, $$); }
           | /* *empty */              { $$ = ast_block_gen(); syntax_set_block(extra, $$); }
           ;

StmtExp : /* *empty */ { $$ = NULL; }
        | Exp
        ;

Stmt : PUSHZONE Block POPZONE             { $$ = ast_stmt_block_gen($2); }
     | Val '=' Exp ';'                    { $$ = ast_stmt_assign_gen($1, $3); }
     | StmtExp ';'                        { $$ = ast_stmt_exp_gen($1); }
     | RETURN StmtExp ';'                 { $$ = syntax_return(extra, $2); }
     | BREAK ';'                          { $$ = ast_stmt_break_gen(); }
     | CONTINUE ';'                       { $$ = ast_stmt_continue_gen(); }
     | IF '(' Cond ')' Stmt ELSE Stmt     { $$ = ast_stmt_if_else_gen($3, $5, $7); }
     | IF '(' Cond ')' Stmt %prec NO_ELSE { $$ = ast_stmt_if_gen($3, $5); }
     | WHILE '(' Cond ')' Stmt            { $$ = ast_stmt_while_gen($3, $5); }
     | error                              { $$ = ast_stmt_exp_gen(NULL); }
     ;

PUSHZONE : /* *empty */ { syntax_zone_push(extra); }
         ;

POPZONE : /* *empty */ { syntax_zone_pop(extra); }
        ;
%%
