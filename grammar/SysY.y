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

%left NOELSE
%left ELSE

%%
CompUnit : CompUnit Decl           { yydebug("CompUnit Decl -> CompUnit");           }
         | CompUnit FuncDecl ';'   { yydebug("CompUnit FuncDecl ';' -> CompUnit");   }
         | CompUnit FuncDecl Block { yydebug("CompUnit FuncDecl Block -> CompUnit"); }
         | /* *empty */            { yydebug("*empty -> CompUnit");                  }
         ;

Decl : Type VarDefList ';' { yydebug("Type DefList ';' -> Decl"); }
     ;

Type : CONST BType { yydebug("CONST BType -> Type"); }
     | BType       { yydebug("BType -> Type");       }
     ;

BType : INT   { yydebug("INT -> BType");   }
      | FLOAT { yydebug("FLOAT -> BType"); }
      ;

VarDefList : VarDefList ',' VarDef { yydebug("VarDefList ',' VarDef -> VarDefList"); }
           | VarDef                { yydebug("VarDef -> VarDefList");                }
           ;

VarDef : ID VarDefArr '=' InitVal { yydebug("ID VarDefArr '=' InitVal -> VarDef"); }
       | ID VarDefArr             { yydebug("ID VarDefArr -> VarDef");             }
       ;

VarDefArr : VarDefArr '[' ConstExp ']' { yydebug("VarDefArr '[' ConstExp ']' -> VarDefArr"); }
          | /* *empty */               { yydebug("*empty -> VarDefArr");                     }
          ;

InitVal : DefExp              { yydebug("Exp -> InitVal");                 }
        | '{' InitValList '}' { yydebug("'{' InitValList '}' -> InitVal"); }
        | '{' '}'             { yydebug("'{' '}' -> InitVal");             }
        ;

InitValList : InitValList ',' InitVal { yydebug("InitValList ',' InitVal -> InitValList"); }
            | InitVal                 { yydebug("InitVal -> InitValList");                 }
            ;

FuncDecl : VOID ID '(' FuncFParams ')'
         | Type ID '(' FuncFParams ')'
         ;

FuncFParams : FuncFParamList { yydebug("FuncFParamList -> FuncFParams"); }
            | /* *empty */   { yydebug("*empty -> FuncFParams");         }
            ;

FuncFParamList : FuncFParamList ',' FuncFParam { yydebug("FuncFParamList ',' FuncFParam -> FuncFParamList"); }
               | FuncFParam                    { yydebug("FuncFParam -> FuncFParamList");                    }
               ;

FuncFParam : Type ID               { yydebug("Type ID -> FuncFParam");               }
           | Type ID FuncFParamArr { yydebug("Type ID FuncFParamArr -> FuncFParam"); }
           ;

FuncFParamArr : FuncFParamArr '[' DefExp ']' { yydebug("FuncFParamArr '[' DefExp ']' -> FuncFParamArr"); }
              | '[' ']'                      { yydebug("'[' ']' -> FuncFParamArr");                      }
              ;

Block : '{' BlockItems '}' { yydebug("'{' BlockItems '}' -> Block"); }
      ;

BlockItems : BlockItems Decl { yydebug("BlockItems Decl -> BlockItems"); }
           | BlockItems Stmt { yydebug("BlockItems Stmt -> BlockItems"); }
           | /* *empty */    { yydebug("*empty -> BlockItems");          }
           ;

Stmt : IfMatchedStmt   { yydebug("IfMatchedStmt -> Stmt");   }
     | IfUnMatchedStmt { yydebug("IfUnMatchedStmt -> Stmt"); }
     ;

IfMatchedStmt : Block                                           { yydebug("Block -> IfMatchedStmt");                                           }
              | ';'                                             { yydebug("';' -> IfMatchedStmt");                                             }
              | Exp ';'                                         { yydebug("Exp ';' -> IfMatchedStmt");                                         }
              | WHILE '(' Exp ')' Stmt                          { yydebug("WHILE '(' Exp ')' Stmt -> IfMatchedStmt");                          }
              | RETURN Exp ';'                                  { yydebug("RETURN Exp ';' -> IfMatchedStmt");                                  }
              | RETURN ';'                                      { yydebug("RETURN ';' -> IfMatchedStmt");                                      }
              | BREAK ';'                                       { yydebug("BREAK ';' -> IfMatchedStmt");                                       }
              | CONTINUE ';'                                    { yydebug("CONTINUE ';' -> IfMatchedStmt");                                    }
              | IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt { yydebug("IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt -> IfMatchedStmt"); }
              ;

IfUnMatchedStmt : IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt { yydebug("IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt -> IfUnMatchedStmt"); }
                | IF '(' Exp ')' IfMatchedStmt %prec NOELSE         { yydebug("IF '(' Exp ')' IfMatchedStmt -> IfUnMatchedStmt");                      }
                | IF '(' Exp ')' IfUnMatchedStmt                    { yydebug("IF '(' Exp ')' IfUnMatchedStmt -> IfUnMatchedStmt");                    }
                ;

Exp : Exp ',' AssignExp { yydebug("Exp ',' AssignExp -> Exp"); }
    | AssignExp         { yydebug("AssignExp -> Exp");         }
    ;

AssignExp : LVal '=' AssignExp { yydebug("ID '=' AssignExp -> AssignExp"); }
          | LOrExp             { yydebug("LOrExp -> AssignExp");           }
          ;

LOrExp : LOrExp OR LAndExp { yydebug("LOrExp OR LAndExp -> LOrExp"); }
       | LAndExp           { yydebug("LAndExp -> LOrExp");           }
       ;

LAndExp : LAndExp AND BOrExp { yydebug("LAndExp AND BOrExp -> LAndExp"); }
        | BOrExp             { yydebug("BOrExp -> LAndExp");             }
        ;

BOrExp : BOrExp '|' BNorExp { yydebug("BOrExp '|' BNorExp -> BOrExp"); }
       | BNorExp            { yydebug("BNorExp -> BOrExp");            }
       ;

BNorExp : BNorExp '^' BAndExp
        | BAndExp
        ;

BAndExp : BAndExp '&' EqExp
        | EqExp
        ;

EqExp : EqExp EQ RelExp
      | EqExp NEQ RelExp
      | RelExp
      ;

RelExp : RelExp '<' AddExp
       | RelExp '>' AddExp
       | RelExp LE AddExp
       | RelExp GE AddExp
       | AddExp
       ;

AddExp : AddExp '+' MulExp
       | AddExp '-' MulExp
       | MulExp
       ;

MulExp : MulExp '*' UnaryExp
       | MulExp '/' UnaryExp
       | MulExp '%' UnaryExp
       | UnaryExp
       ;

UnaryExp : '-' UnaryExp
         | '+' UnaryExp
         | '!' UnaryExp
         | '~' UnaryExp
         | '(' Type ')' UnaryExp
         | PrimaryExp
         ;

PrimaryExp : '(' Exp ')'
           | Number
           | ID '(' FuncRParams ')'
           | LVal
           ;

LVal : ID ArrIndex
     ;

Number : INTCONST { yydebug("INTCONST -> Number"); }
       ;

FuncRParams : FuncRParamList { yydebug("FuncRParamList -> FuncRParams"); }
            | /* *empty */   { yydebug("*empty -> FuncRParams");         }
            ;

FuncRParamList : FuncRParamList ',' FuncRParam { yydebug("FuncRParamList ',' FuncRParam -> FuncRParamList"); }
               | FuncRParam                    { yydebug("FuncRParam -> FuncRParamList");                    }
               ;

FuncRParam : DefExp { yydebug("Exp -> FuncRParam"); }

ArrIndex : ArrIndex '[' Exp ']' { yydebug("ArrIndex '[' Exp ']' -> ArrIndex"); }
         | /* *empty */         { yydebug("*empty -> ArrIndex");               }
         ;

DefExp : AssignExp { yydebug("AssignExp -> DefExp"); }

ConstExp : DefExp { yydebug("DefExp -> ConstExp"); }
         ;
%%
