/*
 * SysY.y : Parser for SysY language
 */

%define parse.error verbose
%param { yyscan_t yyscanner }
%parse-param { pCompUnitNode *root }

%{
#include <lexer.h>
#include <frontend/log.h>
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
       pDirectDeclaratorNode DirectDeclarator;
       pVarDirectDeclaratorNode VarDirectDeclarator;
       pFuncDirectDeclaratorNode FuncDirectDeclarator;
       pParametersNode Parameters;
       pParameterListNode ParameterList;
       pParameterDeclarationNode ParameterDeclaration;
       pInitializerNode Initializer;
       pInitializerListNode InitializerList;
       pAssignExpNode AssignExp;
       pLOrExpNode LOrExp;
       pLAndExpNode LAndExp;
       pBOrExpNode BOrExp;
       pBNorExpNode BNorExp;
       pBAndExpNode BAndExp;
       pEqExpNode EqExp;
       pRelExpNode RelExp;
       pAddExpNode AddExp;
       pMulExpNode MulExp;
       pUnaryExpNode UnaryExp;
       pPostfixExpNode PostfixExp;
       pPrimaryExpNode PrimaryExp;
       pExpNode Exp;
       pNumberNode Number;
       pFuncRParamsNode FuncRParams;
       pFuncRParamListNode FuncRParamList;
       pFuncRParamNode FuncRParam;
       pFunctionDefNode FunctionDef;
       pBlockNode Block;
       pBlockItemsNode BlockItems;
       pStmtNode Stmt;
       pIfMatchedStmtNode IfMatchedStmt;
       pIfUnMatchedStmtNode IfUnMatchedStmt;

       pIDNode ID;
       pINTCONSTNode INTCONST;
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
%type <DirectDeclarator> DirectDeclarator
%type <VarDirectDeclarator> VarDirectDeclarator
%type <FuncDirectDeclarator> FuncDirectDeclarator
%type <Parameters> Parameters
%type <ParameterList> ParameterList
%type <ParameterDeclaration> ParameterDeclaration
%type <Initializer> Initializer
%type <InitializerList> InitializerList
%type <AssignExp> AssignExp
%type <LOrExp> LOrExp
%type <LAndExp> LAndExp
%type <BOrExp> BOrExp
%type <BNorExp> BNorExp
%type <BAndExp> BAndExp
%type <EqExp> EqExp
%type <RelExp> RelExp
%type <AddExp> AddExp
%type <MulExp> MulExp
%type <UnaryExp> UnaryExp
%type <PostfixExp> PostfixExp
%type <PrimaryExp> PrimaryExp
%type <Exp> Exp
%type <Number> Number
%type <FuncRParams> FuncRParams
%type <FuncRParamList> FuncRParamList
%type <FuncRParam> FuncRParam
%type <FunctionDef> FunctionDef
%type <Block> Block
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
%token <INTCONST> INTCONST
%token AND OR LE GE EQ NEQ
%token SELFADD SELFSUB

%%
CompUnit : CompUnit Declaration { *root = $$ = malloc(sizeof(*$$)); $$->type = tDeclaration; $$->CompUnit = $1; $$->select.Declaration = $2;}
         | CompUnit FunctionDef { *root = $$ = malloc(sizeof(*$$)); $$->type = tFunctionDef; $$->CompUnit = $1; $$->select.FunctionDef = $2;}
         | /* *empty */         { *root = $$ = NULL; }
         ;

Declaration : DeclarationSpecifiers InitDeclaratorList ';' { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1; $$->InitDeclaratorList = $2; }
            ;

DeclarationSpecifiers : DeclarationSpecifiers DeclarationSpecifier { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1;   $$->DeclarationSpecifier = $2;}
                      | DeclarationSpecifier                       { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = NULL; $$->DeclarationSpecifier = $1;}
                      ;

DeclarationSpecifier : TypeSpecifier { $$ = malloc(sizeof(*$$)); $$->type = tTypeSpecifier; $$->select.TypeSpecifier = $1; }
                     | TypeQualifier { $$ = malloc(sizeof(*$$)); $$->type = tTypeQualifier; $$->select.TypeQualifier = $1; }
                     ;

TypeSpecifier : VOID     { $$ = malloc(sizeof(*$$)); $$->type = tVOID;      }
              | UNSIGNED { $$ = malloc(sizeof(*$$)); $$->type = tUNSIGNED;  }
              | SIGNED   { $$ = malloc(sizeof(*$$)); $$->type = tSIGNED;    }
              | LONG     { $$ = malloc(sizeof(*$$)); $$->type = tLONG;      }
              | SHORT    { $$ = malloc(sizeof(*$$)); $$->type = tSHORT;     }
              | INT      { $$ = malloc(sizeof(*$$)); $$->type = tINT;       }
              | FLOAT    { $$ = malloc(sizeof(*$$)); $$->type = tFLOAT;     }
              | CHAR     { $$ = malloc(sizeof(*$$)); $$->type = tCHAR;      }
              ;

TypeQualifier : CONST { $$ = malloc(sizeof(*$$)); $$->type = tCONST; }
              ;

InitDeclaratorList : InitDeclaratorList ',' InitDeclarator { $$ = malloc(sizeof(*$$)); $$->InitDeclaratorList = $1;   $$->InitDeclarator = $3; }
                   | InitDeclarator                        { $$ = malloc(sizeof(*$$)); $$->InitDeclaratorList = NULL; $$->InitDeclarator = $1; }
                   ;

InitDeclarator : Declarator '=' Initializer { $$ = malloc(sizeof(*$$)); $$->Declarator = $1; $$->Initializer = $3;   }
               | Declarator                 { $$ = malloc(sizeof(*$$)); $$->Declarator = $1; $$->Initializer = NULL; }
               ;

Declarator : Pointer DirectDeclarator { $$ = malloc(sizeof(*$$)); $$->Pointer = $1; $$->DirectDeclarator = $2; }
           ;

Pointer : '*' TypeQualifiers Pointer { $$ = malloc(sizeof(*$$)); $$->TypeQualifiers = $2; $$->Pointer = $3; }
        | /* *empty */               { $$ = NULL;                                                           }
        ;

TypeQualifiers : TypeQualifiers TypeQualifier { $$ = malloc(sizeof(*$$)); $$->TypeQualifiers = $1; $$->TypeQualifier = $2; }
               | /* *empty */                 { $$ = NULL;                                                                 }
               ;

DirectDeclarator : VarDirectDeclarator  { $$ = malloc(sizeof(*$$)); $$->type = tVarDirectDeclarator;  $$->select.VarDirectDeclarator = $1;  }
                 | FuncDirectDeclarator { $$ = malloc(sizeof(*$$)); $$->type = tFuncDirectDeclarator; $$->select.FuncDirectDeclarator = $1; }
                 ;

VarDirectDeclarator : VarDirectDeclarator '[' AssignExp ']' { $$ = malloc(sizeof(*$$)); $$->op = '['; $$->select.VarDirectDeclarator = $1; $$->AssignExp = $3;   }
                    | VarDirectDeclarator '[' ']'           { $$ = malloc(sizeof(*$$)); $$->op = '['; $$->select.VarDirectDeclarator = $1; $$->AssignExp = NULL; }
                    | '(' Declarator ')'                    { $$ = malloc(sizeof(*$$)); $$->op = '('; $$->select.Declarator = $2;                                }
                    | ID                                    { $$ = malloc(sizeof(*$$)); $$->op = ID;  $$->select.ID = $1;                                        }
                    ;

FuncDirectDeclarator : '(' Declarator ')' '(' Parameters ')' { $$ = malloc(sizeof(*$$)); $$->SimpleFunc = false; $$->select.Declarator = $2; $$->Parameters = $5; }
                     | ID '(' Parameters ')'                 { $$ = malloc(sizeof(*$$)); $$->SimpleFunc = true;  $$->select.ID = $1;         $$->Parameters = $3; }
                     ;

Parameters : ParameterList { $$ = malloc(sizeof(*$$)); $$->ParameterList = $1;   }
           | /* *empty */  { $$ = malloc(sizeof(*$$)); $$->ParameterList = NULL; }
           ;

ParameterList : ParameterList ',' ParameterDeclaration { $$ = malloc(sizeof(*$$)); $$->ParameterList = $1;   $$->ParameterDeclaration = $3; }
              | ParameterDeclaration                   { $$ = malloc(sizeof(*$$)); $$->ParameterList = NULL; $$->ParameterDeclaration = $1; }
              ;

ParameterDeclaration : DeclarationSpecifiers Declarator { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1; $$->Declarator = $2; }
                     ;

Initializer : '{' InitializerList ',' '}' { $$ = malloc(sizeof(*$$)); $$->isList = true;  $$->select.InitializerList = $2;   }
            | '{' InitializerList '}'     { $$ = malloc(sizeof(*$$)); $$->isList = true;  $$->select.InitializerList = $2;   }
            | '{' '}'                     { $$ = malloc(sizeof(*$$)); $$->isList = true;  $$->select.InitializerList = NULL; }
            | AssignExp                   { $$ = malloc(sizeof(*$$)); $$->isList = false; $$->select.AssignExp = $1;         }
            ;

InitializerList : InitializerList ',' Initializer { $$ = malloc(sizeof(*$$)); $$->InitializerList = $1;   $$->Initializer = $3; }
                | Initializer                     { $$ = malloc(sizeof(*$$)); $$->InitializerList = NULL; $$->Initializer = $1; }
                ;

AssignExp : UnaryExp '=' AssignExp { $$ = malloc(sizeof(*$$)); $$->select.UnaryExp = $1; $$->AssignExp = $3;   }
          | LOrExp                 { $$ = malloc(sizeof(*$$)); $$->select.LOrExp = $1;   $$->AssignExp = NULL; }
          ;

LOrExp : LOrExp OR LAndExp { $$ = malloc(sizeof(*$$)); $$->LOrExp = $1;   $$->LAndExp = $3; }
       | LAndExp           { $$ = malloc(sizeof(*$$)); $$->LOrExp = NULL; $$->LAndExp = $1; }
       ;

LAndExp : LAndExp AND BOrExp { $$ = malloc(sizeof(*$$)); $$->LAndExp = $1;   $$->BOrExp = $3; }
        | BOrExp             { $$ = malloc(sizeof(*$$)); $$->LAndExp = NULL; $$->BOrExp = $1; }
        ;

BOrExp : BOrExp '|' BNorExp { $$ = malloc(sizeof(*$$)); $$->BOrExp = $1;   $$->BNorExp = $3; }
       | BNorExp            { $$ = malloc(sizeof(*$$)); $$->BOrExp = NULL; $$->BNorExp = $1; }
       ;

BNorExp : BNorExp '^' BAndExp { $$ = malloc(sizeof(*$$)); $$->BNorExp = $1;   $$->BAndExp = $3; }
        | BAndExp             { $$ = malloc(sizeof(*$$)); $$->BNorExp = NULL; $$->BAndExp = $1; }
        ;

BAndExp : BAndExp '&' EqExp { $$ = malloc(sizeof(*$$)); $$->BAndExp = $1;   $$->EqExp = $3; }
        | EqExp             { $$ = malloc(sizeof(*$$)); $$->BAndExp = NULL; $$->EqExp = $1; }
        ;

EqExp : EqExp EQ RelExp  { $$ = malloc(sizeof(*$$)); $$->op = EQ;      $$->EqExp = $1;   $$-> RelExp = $3; }
      | EqExp NEQ RelExp { $$ = malloc(sizeof(*$$)); $$->op = NEQ;     $$->EqExp = $1;   $$-> RelExp = $3; }
      | RelExp           { $$ = malloc(sizeof(*$$)); $$->op = YYEMPTY; $$->EqExp = NULL; $$-> RelExp = $1; }
      ;

RelExp : RelExp '<' AddExp { $$ = malloc(sizeof(*$$)); $$->op = '<';     $$->RelExp = $1;   $$->AddExp = $3; }
       | RelExp '>' AddExp { $$ = malloc(sizeof(*$$)); $$->op = '>';     $$->RelExp = $1;   $$->AddExp = $3; }
       | RelExp LE AddExp  { $$ = malloc(sizeof(*$$)); $$->op = LE;      $$->RelExp = $1;   $$->AddExp = $3; }
       | RelExp GE AddExp  { $$ = malloc(sizeof(*$$)); $$->op = GE;      $$->RelExp = $1;   $$->AddExp = $3; }
       | AddExp            { $$ = malloc(sizeof(*$$)); $$->op = YYEMPTY; $$->RelExp = NULL; $$->AddExp = $1; }
       ;

AddExp : AddExp '+' MulExp { $$ = malloc(sizeof(*$$)); $$->op = '+';     $$->AddExp = $1;   $$->MulExp = $3; }
       | AddExp '-' MulExp { $$ = malloc(sizeof(*$$)); $$->op = '-';     $$->AddExp = $1;   $$->MulExp = $3; }
       | MulExp            { $$ = malloc(sizeof(*$$)); $$->op = YYEMPTY; $$->AddExp = NULL; $$->MulExp = $1; }
       ;

MulExp : MulExp '*' UnaryExp { $$ = malloc(sizeof(*$$)); $$->op = '*';     $$->MulExp = $1;   $$->UnaryExp = $3; }
       | MulExp '/' UnaryExp { $$ = malloc(sizeof(*$$)); $$->op = '/';     $$->MulExp = $1;   $$->UnaryExp = $3; }
       | MulExp '%' UnaryExp { $$ = malloc(sizeof(*$$)); $$->op = '%';     $$->MulExp = $1;   $$->UnaryExp = $3; }
       | UnaryExp            { $$ = malloc(sizeof(*$$)); $$->op = YYEMPTY; $$->MulExp = NULL; $$->UnaryExp = $1; }
       ;

UnaryExp : '-' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->op = '-';     $$->select.UnaryExp = $2;   }
         | '+' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->op = '+';     $$->select.UnaryExp = $2;   }
         | '!' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->op = '!';     $$->select.UnaryExp = $2;   }
         | '~' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->op = '~';     $$->select.UnaryExp = $2;   }
         | '*' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->op = '*';     $$->select.UnaryExp = $2;   }
         | '&' UnaryExp     { $$ = malloc(sizeof(*$$)); $$->op = '&';     $$->select.UnaryExp = $2;   }
         | SELFADD UnaryExp { $$ = malloc(sizeof(*$$)); $$->op = SELFADD; $$->select.UnaryExp = $2;   }
         | SELFSUB UnaryExp { $$ = malloc(sizeof(*$$)); $$->op = SELFSUB; $$->select.UnaryExp = $2;   }
         | PostfixExp       { $$ = malloc(sizeof(*$$)); $$->op = YYEMPTY; $$->select.PostfixExp = $1; }
         ;

PostfixExp : PostfixExp '[' Exp ']'         { $$ = malloc(sizeof(*$$)); $$->op = '[';     $$->select.PostfixExp = $1; $$->suffix.Exp = $3;         }
           | PostfixExp '(' FuncRParams ')' { $$ = malloc(sizeof(*$$)); $$->op = '(';     $$->select.PostfixExp = $1; $$->suffix.FuncRParams = $3; }
           | PostfixExp SELFADD             { $$ = malloc(sizeof(*$$)); $$->op = SELFADD; $$->select.PostfixExp = $1; $$->suffix.Exp = NULL;       }
           | PostfixExp SELFSUB             { $$ = malloc(sizeof(*$$)); $$->op = SELFSUB; $$->select.PostfixExp = $1; $$->suffix.Exp = NULL;       }
           | PrimaryExp                     { $$ = malloc(sizeof(*$$)); $$->op = YYEMPTY; $$->select.PrimaryExp = $1; $$->suffix.Exp = NULL;       }
           ;

PrimaryExp : '(' Exp ')' { $$ = malloc(sizeof(*$$)); $$->type = '(';      $$->select.Exp = $2;    }
           | Number      { $$ = malloc(sizeof(*$$)); $$->type = $1->type; $$->select.Number = $1; }
           | ID          { $$ = malloc(sizeof(*$$)); $$->type = ID;       $$->select.ID = $1;     }
           ;

Exp : Exp ',' AssignExp { $$ = malloc(sizeof(*$$)); $$->Exp = $1;   $$->AssignExp = $3; }
    | AssignExp         { $$ = malloc(sizeof(*$$)); $$->Exp = NULL; $$->AssignExp = $1; }
    ;

Number : INTCONST { $$ = malloc(sizeof(*$$)); $$->type = INTCONST; $$->select.INTCONST = $1; }
       ;

FuncRParams : FuncRParamList { $$ = malloc(sizeof(*$$)); $$->FuncRParamList = $1;   }
            | /* *empty */   { $$ = malloc(sizeof(*$$)); $$->FuncRParamList = NULL; }
            ;

FuncRParamList : FuncRParamList ',' FuncRParam { $$ = malloc(sizeof(*$$)); $$->FuncRParamList = $1;   $$->FuncRParam = $3; }
               | FuncRParam                    { $$ = malloc(sizeof(*$$)); $$->FuncRParamList = NULL; $$->FuncRParam = $1; }
               ;

FuncRParam : AssignExp { $$ = malloc(sizeof(*$$)); $$->AssignExp = $1; }
           ;

FunctionDef : DeclarationSpecifiers Pointer ID '(' Parameters ')' Block { $$ = malloc(sizeof(*$$)); $$->DeclarationSpecifiers = $1; $$->Pointer = $2; $$->ID = $3; $$->Parameters = $5; $$->Block = $7; }
            ;

Block : '{' BlockItems '}' { $$ = malloc(sizeof(*$$)); $$->BlockItems = $2; }
      ;

BlockItems : BlockItems Declaration { $$ = malloc(sizeof(*$$)); $$->isStmt = false; $$->BlockItems = $1; $$->select.Declaration = $2; }
           | BlockItems Stmt        { $$ = malloc(sizeof(*$$)); $$->isStmt = true;  $$->BlockItems = $1; $$->select.Stmt = $2;        }
           | /* *empty */           { $$ = NULL;                                                                                      }
           ;

Stmt : IfMatchedStmt   { $$ = malloc(sizeof(*$$)); $$->isMatched = true;  $$->select.IfMatchedStmt = $1;   }
     | IfUnMatchedStmt { $$ = malloc(sizeof(*$$)); $$->isMatched = false; $$->select.IfUnMatchedStmt = $1; }
     ;

IfMatchedStmt : Block                                           { $$ = malloc(sizeof(*$$)); $$->type = '{';      $$->select.Block = $1; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | ';'                                             { $$ = malloc(sizeof(*$$)); $$->type = ';';      $$->select.Exp = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | Exp ';'                                         { $$ = malloc(sizeof(*$$)); $$->type = ';';      $$->select.Exp = $1;   $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | RETURN Exp ';'                                  { $$ = malloc(sizeof(*$$)); $$->type = RETURN;   $$->select.Exp = $2;   $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | RETURN ';'                                      { $$ = malloc(sizeof(*$$)); $$->type = RETURN;   $$->select.Exp = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | BREAK ';'                                       { $$ = malloc(sizeof(*$$)); $$->type = BREAK;    $$->select.Exp = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | CONTINUE ';'                                    { $$ = malloc(sizeof(*$$)); $$->type = CONTINUE; $$->select.Exp = NULL; $$->IfMatchedStmt_1 = NULL; $$->IfMatchedStmt_2 = NULL; }
              | IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt { $$ = malloc(sizeof(*$$)); $$->type = ELSE;     $$->select.Exp = $3;   $$->IfMatchedStmt_1 = $5;   $$->IfMatchedStmt_2 = $7;   }
              | WHILE '(' Exp ')' IfMatchedStmt                 { $$ = malloc(sizeof(*$$)); $$->type = WHILE;    $$->select.Exp = $3;   $$->IfMatchedStmt_1 = $5;   $$->IfMatchedStmt_2 = NULL; }
              ;

IfUnMatchedStmt : IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt { $$ = malloc(sizeof(*$$)); $$->type = ELSE;  $$->Exp = $3; $$->select.IfMatchedStmt = $5;   $$->IfUnMatchedStmt = $7;   }
                | IF '(' Exp ')' Stmt                               { $$ = malloc(sizeof(*$$)); $$->type = IF;    $$->Exp = $3; $$->select.Stmt = $5;            $$->IfUnMatchedStmt = NULL; }
                | WHILE '(' Exp ')' IfUnMatchedStmt                 { $$ = malloc(sizeof(*$$)); $$->type = WHILE; $$->Exp = $3; $$->select.IfUnMatchedStmt = $5; $$->IfUnMatchedStmt = NULL; }
                ;
%%
