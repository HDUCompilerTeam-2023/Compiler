/*
 * SysY.y : Parser for SysY language
 */

%{
#include <log.h>
#include <lexer.h>
%}

%locations

%token INT FLOAT VOID
%token CONST
%token DO WHILE FOR BREAK CONTINUE
%token IF ELSE
%token RETURN

%token ID
%token INTCONST
%token AND OR LE GE EQ NEQ
%token SELFADD SELFSUB

%%
CompUnit : CompUnit Declaration { yydebug("CompUnit Declaration -> CompUnit"); }
         | CompUnit FunctionDef { yydebug("CompUnit FunctionDef - >CompUnit"); }
         | /* *empty */         { yydebug("*empty -> CompUnit");               }
         ;

Declaration : DeclarationSpecifiers InitDeclaratorList ';' { yydebug("DeclarationSpecifiers InitDeclaratorList -> Declaration"); }
            ;

DeclarationSpecifiers : DeclarationSpecifiers DeclarationSpecifier { yydebug("DeclarationSpecifiers DeclarationSpecifier -> DeclarationSpecifiers"); }
                      | DeclarationSpecifier                       { yydebug("DeclarationSpecifier -> DeclarationSpecifiers");                       }
                      ;

DeclarationSpecifier : TypeSpecifier { yydebug("TypeSpecifier -> DeclarationSpecifier"); }
                     | TypeQualifier { yydebug("TypeQualifier -> DeclarationSpecifier"); }
                     ;

TypeSpecifier : VOID  { yydebug("VOID -> TypeSpecifier");  }
              | INT   { yydebug("INT -> TypeSpecifier");   }
              | FLOAT { yydebug("FLOAT -> TypeSpecifier"); }
              ;

TypeQualifier : CONST { yydebug("CONST -> TypeQualifier"); }
              ;

InitDeclaratorList : InitDeclaratorList ',' InitDeclarator { yydebug("InitDeclaratorList ',' InitDeclarator -> InitDeclaratorList"); }
                   | InitDeclarator                        { yydebug("InitDeclarator -> InitDeclaratorList");                        }
                   ;

InitDeclarator : Declarator '=' Initializer { yydebug("Declarator '=' Initializer -> InitDeclarator"); }
               | Declarator                 { yydebug("Declarator -> InitDeclarator");                 }
               ;

Declarator : Pointer DirectDeclarator { yydebug("Pointer DirectDeclarator -> Declarator"); }
           ;

Pointer : '*' TypeQualifiers Pointer { yydebug("'*' TypeQualifiers Pointer -> Pointer"); }
        | /* *empty */               { yydebug("*empty -> Pointer");                     }
        ;

TypeQualifiers : TypeQualifiers TypeQualifier { yydebug("TypeQualifiers TypeQualifier -> TypeQualifiers"); }
               | /* *empty */                 { yydebug("*empty -> TypeQualifiers");                       }
               ;

DirectDeclarator : VarDirectDeclarator  { yydebug("VarDirectDeclarator -> DirectDeclarator");  }
                 | FuncDirectDeclarator { yydebug("FuncDirectDeclarator -> DirectDeclarator"); }
                 ;

VarDirectDeclarator : VarDirectDeclarator '[' AssignExp ']' { yydebug("VarDirectDeclarator '[' AssignExp ']' -> VarDirectDeclarator"); }
                    | VarDirectDeclarator '[' ']'           { yydebug("VarDirectDeclarator '[' ']' -> VarDirectDeclarator");           }
                    | '(' Declarator ')'                    { yydebug("'(' Declarator ')' -> VarDirectDeclarator");                    }
                    | ID                                    { yydebug("ID -> VarDirectDeclarator");                                    }
                    ;

FuncDirectDeclarator : '(' Declarator ')' '(' Parameters ')' { yydebug("'(' Declarator ')' '(' Parameters ')' -> FuncDirectDeclarator"); }
                     | ID '(' Parameters ')'                 { yydebug("ID '(' Parameters ')' -> FuncDirectDeclarator");                 }
                     ;

Parameters : ParameterList { yydebug("ParameterList -> Parameters"); }
           | /* *empty */  { yydebug("*empty -> Parameters");        }
           ;

ParameterList : ParameterList ',' ParameterDeclaration { yydebug("ParameterList ',' ParameterDeclaration -> ParameterList"); }
              | ParameterDeclaration                   { yydebug("ParameterDeclaration -> ParameterList");                   }
              ;

ParameterDeclaration : DeclarationSpecifiers Declarator { yydebug("DeclarationSpecifiers Declarator -> ParameterDeclaration"); }
                     ;

Initializer : '{' InitializerList ',' '}' { yydebug("'{' InitializerList ',' '}' -> Initializer"); }
            | '{' InitializerList '}'     { yydebug("'{' InitializerList '}' -> Initializer");     }
            | '{' '}'                     { yydebug("'{' '}' -> Initializer");                     }
            | AssignExp                   { yydebug("AssignExp -> Initializer");                   }
            ;

InitializerList : InitializerList ',' Initializer { yydebug("InitializerList Initializer -> InitializerList"); }
                | Initializer                     { yydebug("Initializer -> InitializerList");                 }
                ;

AssignExp : UnaryExp '=' AssignExp { yydebug("UnaryExp '=' AssignExp -> AssignExp"); }
          | LOrExp                 { yydebug("LOrExp -> AssignExp");                 }
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

UnaryExp : '-' UnaryExp     { yydebug("'-' UnaryExp -> UnaryExp");     }
         | '+' UnaryExp     { yydebug("'+' UnaryExp -> UnaryExp");     }
         | '!' UnaryExp     { yydebug("'!' UnaryExp -> UnaryExp");     }
         | '~' UnaryExp     { yydebug("'~' UnaryExp -> UnaryExp");     }
         | SELFADD UnaryExp { yydebug("SELFADD UnaryExp -> UnaryExp"); }
         | SELFSUB UnaryExp { yydebug("SELFSUB UnaryExp -> UnaryExp"); }
         | PostfixExp       { yydebug("PostfixExp -> UnaryExp");       }
         ;

PostfixExp : PostfixExp '[' Exp ']'         { yydebug("PostfixExp '[' Exp ']' -> PostfixExp");         }
           | PostfixExp '(' FuncRParams ')' { yydebug("PostfixExp '(' FuncRParams ')' -> PostfixExp"); }
           | PostfixExp SELFADD             { yydebug("PostfixExp SELFADD -> PostfixExp");             }
           | PostfixExp SELFSUB             { yydebug("PostfixExp SELFSUB -> PostfixExp");             }
           | PrimaryExp                     { yydebug("PrimaryExp -> PostfixExp");                     }
           ;

PrimaryExp : '(' Exp ')' { yydebug("'(' Exp ')' -> PrimaryExp");            }
           | Number      { yydebug("Number -> PrimaryExp");                 }
           | ID          { yydebug("ID '(' FuncRParams ')' -> PrimaryExp"); }
           ;

Exp : Exp ',' AssignExp { yydebug("Exp ',' AssignExp -> Exp"); }
    | AssignExp         { yydebug("AssignExp -> Exp");         }
    ;

Number : INTCONST { yydebug("INTCONST -> Number"); }
       ;

FuncRParams : FuncRParamList { yydebug("FuncRParamList -> FuncRParams"); }
            | /* *empty */   { yydebug("*empty -> FuncRParams");         }
            ;

FuncRParamList : FuncRParamList ',' FuncRParam { yydebug("FuncRParamList ',' FuncRParam -> FuncRParamList"); }
               | FuncRParam                    { yydebug("FuncRParam -> FuncRParamList");                    }
               ;

FuncRParam : AssignExp { yydebug("AssignExp -> FuncRParam"); }
           ;

FunctionDef : DeclarationSpecifiers Pointer ID '(' Parameters ')' Block { yydebug("DeclarationSpecifiers Pointer ID '(' Parameters ')' -> FunctionDef"); }
            ;

Block : '{' BlockItems '}' { yydebug("'{' BlockItems '}' -> Block"); }
      ;

BlockItems : BlockItems Declaration { yydebug("BlockItems Declaration -> BlockItems"); }
           | BlockItems Stmt        { yydebug("BlockItems Stmt -> BlockItems");        }
           | /* *empty */           { yydebug("*empty -> BlockItems");                 }
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
