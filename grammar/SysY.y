/*
 * SysY.y : Parser for SysY language
 */

%define parse.error verbose
%param { yyscan_t yyscanner }

%{
#include <lexer.h>
#include <frontend/log.h>
#define extra yyget_extra(yyscanner)
%}

%code requires{
#include <frontend/syntaxtree.h>
typedef void *yyscan_t;
}

%define api.pure full

%union {
       pCompUnitNode CompUnit;
       pDeclarationNode Declaration;
       pDeclarationSpecifiersNode DeclarationSpecifiers;
       pDeclarationSpecifierNode DeclarationSpecifier;
       pTypeSpecifierNode TypeSpecifier;
       pTypeQualifierNode TypeQualifier;
       pInitDeclaratorListNode InitDeclaratorList;
       pInitDeclaratorNode InitDeclarator;
       pDeclaratorNode Declarator;
       pPointerNode Pointer;
       pTypeQualifiersNode TypeQualifiers;
       pParameterListNode ParameterList;
       pParameterDeclarationNode ParameterDeclaration;
       pInitializerNode Initializer;
       pInitializerListNode InitializerList;
       pExpressionNode Expression;
       pFuncRParamListNode FuncRParamList;
       pBlockItemsNode BlockItems;
       pStmtNode Stmt;
       pIfMatchedStmtNode IfMatchedStmt;
       pIfUnMatchedStmtNode IfUnMatchedStmt;

       pIDNode ID;
       pCONSTNUMNode CONSTNUM;
}

%type <CompUnit> CompUnit
%type <Declaration> Declaration
%type <DeclarationSpecifiers> DeclarationSpecifiers
%type <DeclarationSpecifier> DeclarationSpecifier
%type <TypeSpecifier> TypeSpecifier
%type <TypeQualifier> TypeQualifier
%type <InitDeclaratorList> InitDeclaratorList
%type <InitDeclarator> InitDeclarator
%type <Declarator> Declarator
%type <Pointer> Pointer
%type <TypeQualifiers> TypeQualifiers
%type <Declarator> DirectDeclarator
%type <ParameterList> Parameters
%type <ParameterList> ParameterList
%type <ParameterDeclaration> ParameterDeclaration
%type <Initializer> Initializer
%type <InitializerList> InitializerList
%type <Expression> AssignExp
%type <Expression> LOrExp
%type <Expression> LAndExp
%type <Expression> BOrExp
%type <Expression> BNorExp
%type <Expression> BAndExp
%type <Expression> EqExp
%type <Expression> RelExp
%type <Expression> AddExp
%type <Expression> MulExp
%type <Expression> UnaryExp
%type <Expression> PostfixExp
%type <Expression> PrimaryExp
%type <Expression> Exp
%type <FuncRParamList> FuncRParams
%type <FuncRParamList> FuncRParamList
%type <BlockItems> Block
%type <BlockItems> BlockItems
%type <Stmt> Stmt
%type <IfMatchedStmt> IfMatchedStmt
%type <IfUnMatchedStmt> IfUnMatchedStmt

%locations

%token UNSIGNED SIGNED
%token LONG SHORT
%token INT FLOAT CHAR
%token VOID
%token CONST
%token DO WHILE FOR BREAK CONTINUE
%token IF ELSE
%token RETURN

%token <ID> ID
%token <CONSTNUM> CONSTNUM
%token AND OR LE GE EQ NEQ
%token SELFADD SELFSUB

%%
begin : CompUnit {
              if (extra->root) {
                     frontend_drop_syntaxtree(extra->root);
              }
              extra->root = $1;
       }
      ;

CompUnit : CompUnit Declaration { $$ = malloc(sizeof(*$$)); $$->CompUnit = $1; $$->Declaration = $2; }
         | /* *empty */         { $$ = NULL; }
         ;

Declaration : DeclarationSpecifiers InitDeclaratorList ';' { $$ = malloc(sizeof(*$$)); $$->is_func_def = false; $$->DeclarationSpecifiers = $1; $$->declarators.InitDeclaratorList = $2; $$->BlockItems = NULL; }
            | DeclarationSpecifiers Declarator Block       { $$ = malloc(sizeof(*$$)); $$->is_func_def = true;  $$->DeclarationSpecifiers = $1; $$->declarators.Declarator = $2;         $$->BlockItems = $3;   }
            ;

DeclarationSpecifiers : DeclarationSpecifiers DeclarationSpecifier { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1;   $$->DeclarationSpecifier = $2;}
                      | DeclarationSpecifier                       { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = NULL; $$->DeclarationSpecifier = $1;}
                      ;

DeclarationSpecifier : TypeSpecifier { $$ = malloc(sizeof(*$$)); $$->type = tTypeSpecifier; $$->select.TypeSpecifier = $1; }
                     | TypeQualifier { $$ = malloc(sizeof(*$$)); $$->type = tTypeQualifier; $$->select.TypeQualifier = $1; }
                     ;

TypeSpecifier : VOID     { $$ = malloc(sizeof(*$$)); $$->type = spec_VOID;      }
              | UNSIGNED { $$ = malloc(sizeof(*$$)); $$->type = spec_UNSIGNED;  }
              | SIGNED   { $$ = malloc(sizeof(*$$)); $$->type = spec_SIGNED;    }
              | LONG     { $$ = malloc(sizeof(*$$)); $$->type = spec_LONG;      }
              | SHORT    { $$ = malloc(sizeof(*$$)); $$->type = spec_SHORT;     }
              | INT      { $$ = malloc(sizeof(*$$)); $$->type = spec_INT;       }
              | FLOAT    { $$ = malloc(sizeof(*$$)); $$->type = spec_FLOAT;     }
              | CHAR     { $$ = malloc(sizeof(*$$)); $$->type = spec_CHAR;      }
              ;

TypeQualifier : CONST { $$ = malloc(sizeof(*$$)); $$->type = qual_CONST; }
              ;

InitDeclaratorList : InitDeclaratorList ',' InitDeclarator { $$ = malloc(sizeof(*$$)); $$->InitDeclaratorList = $1;   $$->InitDeclarator = $3; }
                   | InitDeclarator                        { $$ = malloc(sizeof(*$$)); $$->InitDeclaratorList = NULL; $$->InitDeclarator = $1; }
                   ;

InitDeclarator : Declarator '=' Initializer { $$ = malloc(sizeof(*$$)); $$->Declarator = $1; $$->Initializer = $3;   }
               | Declarator                 { $$ = malloc(sizeof(*$$)); $$->Declarator = $1; $$->Initializer = NULL; }
               ;

Declarator : Pointer DirectDeclarator {
                     if ($1) {
                            $$ = malloc(sizeof(*$$));
                            $$->type = tPointer;
                            $$->body.Declarator = $2;
                            $$->select.Pointer = $1;
                     }
                     else {
                            $$ = $2;
                     }
              }
           ;

Pointer : '*' TypeQualifiers Pointer { $$ = malloc(sizeof(*$$)); $$->TypeQualifiers = $2; $$->Pointer = $3; }
        | /* *empty */               { $$ = NULL;                                                           }
        ;

TypeQualifiers : TypeQualifiers TypeQualifier { $$ = malloc(sizeof(*$$)); $$->TypeQualifiers = $1; $$->TypeQualifier = $2; }
               | /* *empty */                 { $$ = NULL;                                                                 }
               ;

DirectDeclarator : DirectDeclarator '[' AssignExp ']'  { $$ = malloc(sizeof(*$$)); $$->type = tArrDec; $$->body.Declarator = $1; $$->select.Expression = $3;    }
                 | DirectDeclarator '[' ']'            { $$ = malloc(sizeof(*$$)); $$->type = tArrDec; $$->body.Declarator = $1; $$->select.Expression = NULL;  }
                 | DirectDeclarator '(' Parameters ')' { $$ = malloc(sizeof(*$$)); $$->type = tFunDec; $$->body.Declarator = $1; $$->select.ParameterList = $3; }
                 | '(' Declarator ')'                  { $$ = $2; }
                 | ID                                  { $$ = malloc(sizeof(*$$)); $$->type = tIDJust; $$->body.ID = $1;         $$->select.IDsuffix = NULL; }
                 ;

Parameters : ParameterList { $$ = $1;   }
           | /* *empty */  { $$ = NULL; }
           ;

ParameterList : ParameterList ',' ParameterDeclaration { $$ = malloc(sizeof(*$$)); $$->ParameterList = $1;   $$->ParameterDeclaration = $3; }
              | ParameterDeclaration                   { $$ = malloc(sizeof(*$$)); $$->ParameterList = NULL; $$->ParameterDeclaration = $1; }
              ;

ParameterDeclaration : DeclarationSpecifiers Declarator { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1; $$->Declarator = $2;   }
                     | DeclarationSpecifiers            { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1; $$->Declarator = NULL; }
                     ;

Initializer : '{' InitializerList ',' '}' { $$ = malloc(sizeof(*$$)); $$->isList = true;  $$->select.InitializerList = $2;   }
            | '{' InitializerList '}'     { $$ = malloc(sizeof(*$$)); $$->isList = true;  $$->select.InitializerList = $2;   }
            | '{' '}'                     { $$ = malloc(sizeof(*$$)); $$->isList = true;  $$->select.InitializerList = NULL; }
            | AssignExp                   { $$ = malloc(sizeof(*$$)); $$->isList = false; $$->select.Expression = $1;        }
            ;

InitializerList : InitializerList ',' Initializer { $$ = malloc(sizeof(*$$)); $$->InitializerList = $1;   $$->Initializer = $3; }
                | Initializer                     { $$ = malloc(sizeof(*$$)); $$->InitializerList = NULL; $$->Initializer = $1; }
                ;

AssignExp : UnaryExp '=' AssignExp { $$ = malloc(sizeof(*$$)); $$->type = assign; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
          | LOrExp                 { $$ = $1; }
          ;

LOrExp : LOrExp OR LAndExp { $$ = malloc(sizeof(*$$)); $$->type = logic_or; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | LAndExp           { $$ = $1; }
       ;

LAndExp : LAndExp AND BOrExp { $$ = malloc(sizeof(*$$)); $$->type = logic_and; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
        | BOrExp             { $$ = $1; }
        ;

BOrExp : BOrExp '|' BNorExp { $$ = malloc(sizeof(*$$)); $$->type = bit_or; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | BNorExp            { $$ = $1; }
       ;

BNorExp : BNorExp '^' BAndExp { $$ = malloc(sizeof(*$$)); $$->type = bit_nor; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
        | BAndExp             { $$ = $1; }
        ;

BAndExp : BAndExp '&' EqExp { $$ = malloc(sizeof(*$$)); $$->type = bit_and; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
        | EqExp             { $$ = $1; }
        ;

EqExp : EqExp EQ RelExp  { $$ = malloc(sizeof(*$$)); $$->type = eq;  $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
      | EqExp NEQ RelExp { $$ = malloc(sizeof(*$$)); $$->type = neq; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
      | RelExp           { $$ = $1; }
      ;

RelExp : RelExp '<' AddExp { $$ = malloc(sizeof(*$$)); $$->type = less;     $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | RelExp '>' AddExp { $$ = malloc(sizeof(*$$)); $$->type = great;    $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | RelExp LE AddExp  { $$ = malloc(sizeof(*$$)); $$->type = less_eq;  $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | RelExp GE AddExp  { $$ = malloc(sizeof(*$$)); $$->type = great_eq; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | AddExp            { $$ = $1; }
       ;

AddExp : AddExp '+' MulExp { $$ = malloc(sizeof(*$$)); $$->type = binary_add; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | AddExp '-' MulExp { $$ = malloc(sizeof(*$$)); $$->type = binary_sub; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | MulExp            { $$ = $1; }
       ;

MulExp : MulExp '*' UnaryExp { $$ = malloc(sizeof(*$$)); $$->type = binary_mul; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | MulExp '/' UnaryExp { $$ = malloc(sizeof(*$$)); $$->type = binary_div; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | MulExp '%' UnaryExp { $$ = malloc(sizeof(*$$)); $$->type = binary_mod; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3; }
       | UnaryExp            { $$ = $1; }
       ;

UnaryExp : '-' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->type = negative;    $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | '+' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->type = positive;    $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | '!' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->type = logic_not;   $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | '~' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->type = bit_not;     $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | '*' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->type = deref;       $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | '&' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->type = ref;         $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | SELFADD UnaryExp { $$ = malloc(sizeof(*$$)); $$->type = selfadd_pre; $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | SELFSUB UnaryExp { $$ = malloc(sizeof(*$$)); $$->type = selfsub_pre; $$->select.exp.first = $2; $$->select.exp.second.Expression = NULL; }
         | PostfixExp       { $$ = $1; }
         ;

PostfixExp : PostfixExp '[' Exp ']'         { $$ = malloc(sizeof(*$$)); $$->type = arrary_index; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3;     }
           | PostfixExp '(' FuncRParams ')' { $$ = malloc(sizeof(*$$)); $$->type = func_call;    $$->select.exp.first = $1; $$->select.exp.second.FuncRParamList = $3; }
           | PostfixExp SELFADD             { $$ = malloc(sizeof(*$$)); $$->type = selfadd;      $$->select.exp.first = $1; $$->select.exp.second.Expression = NULL;   }
           | PostfixExp SELFSUB             { $$ = malloc(sizeof(*$$)); $$->type = selfsub;      $$->select.exp.first = $1; $$->select.exp.second.Expression = NULL;   }
           | PrimaryExp                     { $$ = $1; }
           ;

PrimaryExp : '(' Exp ')' { $$ = $2; }
           | CONSTNUM    { $$ = malloc(sizeof(*$$)); $$->type = type_CONSTNUM; $$->select.CONSTNUM = $1; }
           | ID          { $$ = malloc(sizeof(*$$)); $$->type = type_ID;       $$->select.ID = $1;       }
           ;

Exp : Exp ',' AssignExp { $$ = malloc(sizeof(*$$)); $$->type = comma; $$->select.exp.first = $1; $$->select.exp.second.Expression = $3;     }
    | AssignExp         { $$ = $1; }
    ;

FuncRParams : FuncRParamList { $$ = $1;   }
            | /* *empty */   { $$ = NULL; }
            ;

FuncRParamList : FuncRParamList ',' AssignExp { $$ = malloc(sizeof(*$$)); $$->FuncRParamList = $1;   $$->Expression = $3; }
               | AssignExp                    { $$ = malloc(sizeof(*$$)); $$->FuncRParamList = NULL; $$->Expression = $1; }
               ;

Block : '{' BlockItems '}' { $$ = $2; }
      ;

BlockItems : BlockItems Declaration { $$ = malloc(sizeof(*$$)); $$->isStmt = false; $$->BlockItems = $1; $$->select.Declaration = $2; }
           | BlockItems Stmt        { $$ = malloc(sizeof(*$$)); $$->isStmt = true;  $$->BlockItems = $1; $$->select.Stmt = $2;        }
           | /* *empty */           { $$ = NULL;                                                                                      }
           ;

Stmt : IfMatchedStmt   { $$ = malloc(sizeof(*$$)); $$->isMatched = true;  $$->select.IfMatchedStmt = $1;   }
     | IfUnMatchedStmt { $$ = malloc(sizeof(*$$)); $$->isMatched = false; $$->select.IfUnMatchedStmt = $1; }
     ;

IfMatchedStmt : Block                                           { $$ = malloc(sizeof(*$$)); $$->type = '{';      $$->select.BlockItems = $1;   $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | ';'                                             { $$ = malloc(sizeof(*$$)); $$->type = ';';      $$->select.Expression = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | Exp ';'                                         { $$ = malloc(sizeof(*$$)); $$->type = ';';      $$->select.Expression = $1;   $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | RETURN Exp ';'                                  { $$ = malloc(sizeof(*$$)); $$->type = RETURN;   $$->select.Expression = $2;   $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | RETURN ';'                                      { $$ = malloc(sizeof(*$$)); $$->type = RETURN;   $$->select.Expression = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | BREAK ';'                                       { $$ = malloc(sizeof(*$$)); $$->type = BREAK;    $$->select.Expression = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | CONTINUE ';'                                    { $$ = malloc(sizeof(*$$)); $$->type = CONTINUE; $$->select.Expression = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt { $$ = malloc(sizeof(*$$)); $$->type = ELSE;     $$->select.Expression = $3;   $$->IfMatchedStmt_1 = $5;   $$->IfMatchedStmt_2 = $7;   }
              | WHILE '(' Exp ')' IfMatchedStmt                 { $$ = malloc(sizeof(*$$)); $$->type = WHILE;    $$->select.Expression = $3;   $$->IfMatchedStmt_1 = $5;   $$->IfMatchedStmt_2 = NULL; }
              ;

IfUnMatchedStmt : IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt { $$ = malloc(sizeof(*$$)); $$->type = ELSE;  $$->Expression = $3; $$->select.IfMatchedStmt = $5;   $$->IfUnMatchedStmt = $7;   }
                | IF '(' Exp ')' Stmt                               { $$ = malloc(sizeof(*$$)); $$->type = IF;    $$->Expression = $3; $$->select.Stmt = $5;            $$->IfUnMatchedStmt = NULL; }
                | WHILE '(' Exp ')' IfUnMatchedStmt                 { $$ = malloc(sizeof(*$$)); $$->type = WHILE; $$->Expression = $3; $$->select.IfUnMatchedStmt = $5; $$->IfUnMatchedStmt = NULL; }
                ;
%%
