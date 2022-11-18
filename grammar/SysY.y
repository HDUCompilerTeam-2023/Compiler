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
CompUnit : CompUnit VarDecl        { yydebug("CompUnit VarDecl -> CompUnit");        }
         | CompUnit FuncDecl ';'   { yydebug("CompUnit FuncDecl ';' -> CompUnit");   }
         | CompUnit FuncDecl Block { yydebug("CompUnit FuncDecl Block -> CompUnit"); }
         | /* *empty */            { yydebug("*empty -> CompUnit");                  }
         ;

VarDecl : Type VarDefList ';' { yydebug("Type DefList ';' -> VarDecl"); }
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

VarDef : ID Indexs '=' InitVal { yydebug("ID Indexs '=' InitVal -> VarDef"); }
       | ID Indexs             { yydebug("ID Indexs -> VarDef");             }
       ;

Indexs : Indexs Index { yydebug("Indexs Index -> Indexs"); }
       | /* *empty */ { yydebug("*empty -> Indexs");       }
       ;

Index : '[' AssignExp ']' { yydebug("'[' AssignExp ']' -> Index"); }
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

BNorExp : BNorExp '^' BAndExp { yydebug("BNorExp '^' BAndExp -> BNorExp"); }
        | BAndExp             { yydebug("BAndExp -> BOrExp");              }
        ;

BAndExp : BAndExp '&' EqExp { yydebug("BAndExp '&' EqExp"); }
        | EqExp             { yydebug("EqExp -> BAndExp");  }
        ;

EqExp : EqExp EQ RelExp  { yydebug("EqExp EQ RelExp -> EqExp");  }
      | EqExp NEQ RelExp { yydebug("EqExp NEQ RelExp -> EqExp"); }
      | RelExp           { yydebug("RelExp -> EqExp");           }
      ;

RelExp : RelExp '<' AddExp { yydebug("RelExp '<' AddExp -> RelExp"); }
       | RelExp '>' AddExp { yydebug("RelExp '>' AddExp -> RelExp"); }
       | RelExp LE AddExp  { yydebug("RelExp LE AddExp -> RelExp");  }
       | RelExp GE AddExp  { yydebug("RelExp GE AddExp -> RelExp");  }
       | AddExp            { yydebug("AddExp -> RelExp");            }
       ;

AddExp : AddExp '+' MulExp { yydebug("AddExp '+' MulExp -> AddExp"); }
       | AddExp '-' MulExp { yydebug("AddExp '-' MulExp -> AddExp"); }
       | MulExp            { yydebug("MulExp -> AddExp");            }
       ;

MulExp : MulExp '*' UnaryExp { yydebug("MulExp '*' UnaryExp - >MulExp");  }
       | MulExp '/' UnaryExp { yydebug("MulExp '/' UnaryExp - >MulExp");  }
       | MulExp '%' UnaryExp { yydebug("MulExp '%%' UnaryExp - >MulExp"); }
       | UnaryExp            { yydebug("UnaryExp -> MulExp");             }
       ;

UnaryExp : '-' UnaryExp          { yydebug("'-' UnaryExp -> UnaryExp");          }
         | '+' UnaryExp          { yydebug("'+' UnaryExp -> UnaryExp");          }
         | '!' UnaryExp          { yydebug("'!' UnaryExp -> UnaryExp");          }
         | '~' UnaryExp          { yydebug("'~' UnaryExp -> UnaryExp");          }
         | '(' Type ')' UnaryExp { yydebug("'(' Type ')' UnaryExp -> UnaryExp"); }
         | PrimaryExp            { yydebug("PrimaryExp -> UnaryExp");            }
         ;

PrimaryExp : '(' Exp ')'            { yydebug("'(' Exp ')' -> PrimaryExp");            }
           | Number                 { yydebug("Number -> PrimaryExp");                 }
           | ID '(' FuncRParams ')' { yydebug("ID '(' FuncRParams ')' -> PrimaryExp"); }
           | LVal                   { yydebug("LVal -> PrimaryExp");                   }
           ;

Exp : Exp ',' AssignExp { yydebug("Exp ',' AssignExp -> Exp"); }
    | AssignExp         { yydebug("AssignExp -> Exp");         }
    ;

Number : INTCONST { yydebug("INTCONST -> Number"); }
       ;

FuncRParams : Exp          { yydebug("Exp -> FuncRParams");    }
            | /* *empty */ { yydebug("*empty -> FuncRParams"); }
            ;

LVal : ID Indexs { yydebug("ID Indexs -> LVal"); }
     ;

InitVal : AssignExp           { yydebug("AssignExp -> InitVal");           }
        | '{' '}'             { yydebug("'{' '}' -> InitVal");             }
        | '{' InitValList '}' { yydebug("'{' InitValList '}' -> InitVal"); }
        ;

InitValList : InitValList ',' InitVal { yydebug("InitValList ',' InitVal -> InitValList"); }
            | InitVal                 { yydebug("InitVal -> InitValList");                 }
            ;

FuncDecl : VOID ID '(' FuncFParams ')' { yydebug("VOID ID '(' FuncFParams ')' -> FuncDecl"); }
         | Type ID '(' FuncFParams ')' { yydebug("Type ID '(' FuncFParams ')' -> FuncDecl"); }
         ;

FuncFParams : FuncFParamList { yydebug("FuncFParamList -> FuncFParams"); }
            | /* *empty */   { yydebug("*empty -> FuncFParams");         }
            ;

FuncFParamList : FuncFParamList ',' FuncFParam { yydebug("FuncFParamList ',' FuncFParam -> FuncFParamList"); }
               | FuncFParam                    { yydebug("FuncFParam -> FuncFParamList");                    }
               ;

FuncFParam : Type ID FuncFParamIndexs { yydebug("Type ID FuncFParamIndexs -> FuncFParam"); }
           ;

FuncFParamIndexs : FuncFParamFirstIndex Indexs { yydebug("FuncFParamFirstIndex Indexs -> FuncFParamIndexs"); }
                 | /* *empty */                { yydebug("*empty -> FuncFParamIndexs");                      }
                 ;

FuncFParamFirstIndex : '[' ']' { yydebug("'[' ']' -> FuncFParamFirstIndex"); }
                     | Index   { yydebug("Index -> FuncFParamFirstIndex");   }
                     ;

Block : '{' BlockItems '}' { yydebug("'{' BlockItems '}' -> Block"); }
      ;

BlockItems : BlockItems VarDecl { yydebug("BlockItems VarDecl -> BlockItems"); }
           | BlockItems Stmt    { yydebug("BlockItems Stmt -> BlockItems");    }
           | /* *empty */       { yydebug("*empty -> BlockItems");             }
           ;

Stmt : IfMatchedStmt   { yydebug("IfMatchedStmt -> Stmt");   }
     | IfUnMatchedStmt { yydebug("IfUnMatchedStmt -> Stmt"); }
     ;

IfMatchedStmt : Block                                           { yydebug("Block -> IfMatchedStmt");                                           }
              | ';'                                             { yydebug("';' -> IfMatchedStmt");                                             }
              | Exp ';'                                         { yydebug("Exp ';' -> IfMatchedStmt");                                         }
              | RETURN Exp ';'                                  { yydebug("RETURN Exp ';' -> IfMatchedStmt");                                  }
              | RETURN ';'                                      { yydebug("RETURN ';' -> IfMatchedStmt");                                      }
              | BREAK ';'                                       { yydebug("BREAK ';' -> IfMatchedStmt");                                       }
              | CONTINUE ';'                                    { yydebug("CONTINUE ';' -> IfMatchedStmt");                                    }
              | IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt { yydebug("IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt -> IfMatchedStmt"); }
              | WHILE '(' Exp ')' IfMatchedStmt                 { yydebug("WHILE '(' Exp ')' IfMatchedStmt -> IfMatchedStmt");                 }
              ;

IfUnMatchedStmt : IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt { yydebug("IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt -> IfUnMatchedStmt"); }
                | IF '(' Exp ')' Stmt                               { yydebug("IF '(' Exp ')' Stmt -> IfUnMatchedStmt");                               }
                | WHILE '(' Exp ')' IfUnMatchedStmt                 { yydebug("WHILE '(' Exp ')' IfUnMatchedStmt -> IfMatchedStmt");                   }
                ;
%%
