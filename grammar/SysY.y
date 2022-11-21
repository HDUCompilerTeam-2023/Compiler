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
CompUnit : CompUnit Decl { yydebug("CompUnit Decl -> CompUnit"); }
         | /* *empty */  { yydebug("*empty -> CompUnit");        }
         ;

Decl : ConstDecl { yydebug("ConstDecl -> Decl"); }
     ;

ConstDecl : CONST Type ConstDefList ';' { yydebug("CONST Type ConstDefList ';' -> ConstDecl"); }
          ;

Type : INT   { yydebug("INT -> Type");   }
     | FLOAT { yydebug("FLOAT -> Type"); }
     ;

ConstDefList : ConstDefList ',' ConstDef { yydebug("ConstDefList ',' ConstDef -> ConstDefList"); }
             | ConstDef                  { yydebug("ConstDef -> ConstDefList"); }
             ;

ConstDef : ID '=' ConstInitVal { yydebug("IDENTIFY '=' ConstInitVal -> ConstDef"); }
         ;

ConstInitVal : INTCONST { yydebug("INTCONST -> ConstInitVal"); }
             ;
%%
