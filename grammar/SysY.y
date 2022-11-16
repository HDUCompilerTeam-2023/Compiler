/*
 * SysY.y : Parser for SysY language
 */

%{
#include <log.h>

int yylex();
%}

%locations

%token INT FLOAT VOID CONST
%token DO WHILE FOR BREAK CONTINUE
%token IF ELSE
%token RETURN

%token ID
%token INTCONST
%token AND OR LE GE EQ NEQ

%%
CompUnit : CompUnit Decl
         | /* *empty */
         ;

Decl : ConstDecl
     ;

ConstDecl : CONST Type ConstDefList ';'
          ;

Type : INT
     | FLOAT
     ;

ConstDefList : ConstDefList ',' ConstDef
             | ConstDef
             ;

ConstDef : ID '=' ConstInitVal
         ;

ConstInitVal : INTCONST
             ;
%%
