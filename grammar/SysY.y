/*
 * SysY.y : Parser for SysY language
 */

%define parse.error verbose
%param { yyscan_t yyscanner }

%{
#include <hir_gen/lexer.h>
#include <hir_gen/log.h>

#define extra yyget_extra(yyscanner)
#define p_ast (extra->p_ast)
#define pss (p_ast->pss)

#define new_sym(name) symbol_add(pss, name)
#define find_sym(name) symbol_find(pss, name)
%}

%initial-action
{
       p_ast = hir_program_gen();
}

%code requires{
#include <hir_gen/syntax.h>
typedef void *yyscan_t;
}

%define api.pure full

%union {
       p_hir_block p_block;
       p_hir_stmt p_stmt;
       p_hir_exp p_exp;

       p_hir_program p_program;
       p_hir_func p_func;

       p_hir_param_list p_param_list;

       p_syntax_decl p_decl;
       p_syntax_decl_list p_decl_list;

       p_syntax_param_decl p_param_decl;
       p_syntax_param_list p_parameter_list;

       p_syntax_init p_init;

       basic_type type;

       ID_t ID;
       INTCONST_t INTCONST;
       FLOATCONST_t FLOATCONST;
}

%type <p_exp> Exp
%type <p_exp> AssignExp
%type <p_exp> LOrExp
%type <p_exp> LAndExp
%type <p_exp> EqExp
%type <p_exp> RelExp
%type <p_exp> AddExp
%type <p_exp> MulExp
%type <p_exp> UnaryExp
%type <p_exp> PrimaryExp

%type <p_exp> Call
%type <p_exp> Val

%type <p_exp> StmtExp
%type <p_stmt> Stmt

%type <p_block> BlockItems
%type <p_block> Block

%type <p_param_list> FuncRParamList
%type <p_param_list> FuncRParams

%type <p_func> FuncDeclaration
%type <p_program> CompUnit
%type <p_program> CompUnitInit

%type <p_param_decl> ParameterDeclaration
%type <p_parameter_list> ParameterList
%type <p_parameter_list> Parameters

%type <p_decl> Declarator
%type <p_decl> InitDeclarator
%type <p_decl_list> InitDeclaratorList
%type <p_decl_list> VarDeclaration

%type <p_init> Initializer
%type <p_init> InitializerList

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

%token <INTCONST> INTCONST
%token <FLOATCONST> FLOATCONST
%token <ID> ID
%destructor { free($$); } ID

%nonassoc NO_ELSE
%nonassoc ELSE

%%
begin : PUSHZONE CompUnit POPZONE
      ;

CompUnit : CompUnit VarDeclaration  { syntax_global_vardecl(pss, $2); }
         | CompUnit FuncDeclaration { $$ = hir_program_add($1, $2); }
         | CompUnitInit
         | CompUnit error
         ;

Type : INT   { $$ = type_int; }
     | FLOAT { $$ = type_float; }
     ;

VarDeclaration : CONST Type InitDeclaratorList ';' { $$ = syntax_decl_list_set($3, true, $2); }
               | Type InitDeclaratorList ';'       { $$ = syntax_decl_list_set($2, false, $1); }
               ;

InitDeclaratorList : InitDeclaratorList ',' InitDeclarator { $$ = syntax_decl_list_add($1, $3); }
                   | InitDeclarator                        { $$ = syntax_decl_list_add(syntax_decl_list_gen(), $1); }
                   ;

InitDeclarator : Declarator '=' Initializer { $$ = syntax_decl_init($1, $3); }
               | Declarator                 { $$ = syntax_decl_init($1, NULL); }
               ;

Declarator : Declarator '[' Exp ']' { $$ = syntax_decl_arr($1, $3); }
           | Declarator '[' ']'     { $$ = syntax_decl_arr($1, NULL); }
           | ID                     { $$ = syntax_decl_gen($1); }
           | Declarator error
           ;

FuncDeclaration : Type ID '(' Parameters ')'
                     { syntax_func_define(pss, $1, $2, $4); }
                  PUSHZONE
                     { syntax_func_param(pss, $4); }
                  Block
                  POPZONE
                     { $$ = hir_func_gen(find_sym($2), $9); free($2); }
                | VOID ID '(' Parameters ')'
                     { syntax_func_define(pss, type_void, $2, $4); }
                  PUSHZONE
                     { syntax_func_param(pss, $4); }
                  Block
                  POPZONE
                     { $$ = hir_func_gen(find_sym($2), $9); free($2); }
                ;

Parameters : ParameterList
           | /* *empty */  { $$ = syntax_param_list_gen(); }
           ;

ParameterList : ParameterList ',' ParameterDeclaration { $$ = syntax_param_list_add($1, $3); }
              | ParameterDeclaration                   { $$ = syntax_param_list_add(NULL, $1); }
              ;

ParameterDeclaration : Type Declarator { $$ = syntax_param_decl_gen($1, $2); }
                     ;

Initializer : '{' InitializerList '}'     { $$ = $2; }
            | '{' '}'                     { $$ = syntax_init_list_gen(); }
            | AssignExp                   { $$ = syntax_init_exp_gen($1); }
            ;

InitializerList : InitializerList ',' Initializer { $$ = syntax_init_list_add($1, $3); }
                | Initializer                     { $$ = syntax_init_list_add(syntax_init_list_gen(), $1); }
                ;

AssignExp : UnaryExp '=' AssignExp { $$ = hir_exp_assign_gen($1, $3); }
          | LOrExp
          ;

LOrExp : LOrExp OR LAndExp { $$ = hir_exp_lexec_gen(hir_exp_op_bool_or, $1, $3); }
       | LAndExp
       ;

LAndExp : LAndExp AND EqExp { $$ = hir_exp_lexec_gen(hir_exp_op_bool_and, $1, $3); }
        | EqExp
        ;

EqExp : EqExp EQ RelExp  { $$ = hir_exp_lexec_gen(hir_exp_op_eq, $1, $3); }
      | EqExp NEQ RelExp { $$ = hir_exp_lexec_gen(hir_exp_op_neq, $1, $3); }
      | RelExp
      ;

RelExp : RelExp '<' AddExp { $$ = hir_exp_lexec_gen(hir_exp_op_l, $1, $3); }
       | RelExp '>' AddExp { $$ = hir_exp_lexec_gen(hir_exp_op_g, $1, $3); }
       | RelExp LE AddExp  { $$ = hir_exp_lexec_gen(hir_exp_op_leq, $1, $3); }
       | RelExp GE AddExp  { $$ = hir_exp_lexec_gen(hir_exp_op_geq, $1, $3); }
       | AddExp
       ;

AddExp : AddExp '+' MulExp { $$ = hir_exp_exec_gen(hir_exp_op_add, $1, $3); }
       | AddExp '-' MulExp { $$ = hir_exp_exec_gen(hir_exp_op_sub, $1, $3); }
       | MulExp
       ;

MulExp : MulExp '*' UnaryExp { $$ = hir_exp_exec_gen(hir_exp_op_mul, $1, $3); }
       | MulExp '/' UnaryExp { $$ = hir_exp_exec_gen(hir_exp_op_div, $1, $3); }
       | MulExp '%' UnaryExp { $$ = hir_exp_exec_gen(hir_exp_op_mod, $1, $3); }
       | UnaryExp
       ;

UnaryExp : '-' UnaryExp     { $$ = hir_exp_uexec_gen(hir_exp_op_minus, $2); }
         | '+' UnaryExp     { $$ = $2; }
         | '!' UnaryExp     { $$ = hir_exp_uexec_gen(hir_exp_op_bool_not, $2); }
         | PrimaryExp
         ;

PrimaryExp : '(' Exp ')' { $$ = $2; }
           | INTCONST    { $$ = hir_exp_int_gen($1); }
           | FLOATCONST  { $$ = hir_exp_float_gen($1); }
           | Val         { $$ = $1; }
           | Call        { $$ = $1; }
           ;

Call : ID '(' FuncRParams ')' { $$ = hir_exp_call_gen(find_sym($1), $3); free($1); }
     ;

Val : ID                 { $$ = hir_exp_id_gen(find_sym($1)); free($1); }
    | Val '[' Exp ']'    { $$ = hir_exp_arr_gen($1, $3); }
    ;

Exp : Exp ',' AssignExp { $$ = hir_exp_dot_gen($1, $3); }
    | AssignExp
    ;

FuncRParams : FuncRParamList { $$ = $1; }
            | /* *empty */   { $$ = hir_param_list_init(); }
            ;

FuncRParamList : FuncRParamList ',' AssignExp { $$ = hir_param_list_add($1, $3); }
               | AssignExp                    { $$ = hir_param_list_add(hir_param_list_init(), $1); }
               ;

Block : '{' BlockItems '}' { $$ = $2; }
      ;

BlockItems : BlockItems VarDeclaration { $$ = syntax_local_vardecl(pss, $1, $2); }
           | BlockItems Stmt           { $$ = hir_block_add($1, $2); }
           | /* *empty */              { $$ = hir_block_gen(); }
           ;

StmtExp : /* *empty */ { $$ = NULL; }
        | Exp
        ;

Stmt : PUSHZONE Block POPZONE            { $$ = hir_stmt_block_gen($2); }
     | StmtExp ';'                       { $$ = hir_stmt_exp_gen($1); }
     | RETURN StmtExp ';'                { $$ = hir_stmt_return_gen($2); }
     | BREAK ';'                         { $$ = hir_stmt_break_gen(); }
     | CONTINUE ';'                      { $$ = hir_stmt_continue_gen(); }
     | IF '(' Exp ')' Stmt ELSE Stmt     { $$ = hir_stmt_if_else_gen($3, $5, $7); }
     | IF '(' Exp ')' Stmt %prec NO_ELSE { $$ = hir_stmt_if_gen($3, $5); }
     | WHILE '(' Exp ')' Stmt            { $$ = hir_stmt_while_gen($3, $5); }
     | error                             { $$ = hir_stmt_exp_gen(NULL); }
     ;

PUSHZONE : /* *empty */ { symbol_push_zone(pss); }
         ;

POPZONE : /* *empty */ { symbol_pop_zone(pss); }
        ;

CompUnitInit : /* *empty */ { $$ = p_ast; }
             ;
%%
