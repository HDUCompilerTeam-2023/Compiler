/*
 * SysY.y : Parser for SysY language
 */

%define parse.error verbose
%param { yyscan_t yyscanner }

%{
#include <hir.h>
#include <hir/log.h>
#define extra yyget_extra(yyscanner)
%}

%code requires{
#include <hir/syntaxtree.h>
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

       pIDNode ID;
       pCONSTNUMNode CONSTNUM;
}

%type <CompUnit> CompUnit
%destructor { frontend_drop_CompUnit($$); } CompUnit
%type <Declaration> Declaration
%destructor { frontend_drop_Declaration($$); } Declaration
%type <DeclarationSpecifiers> DeclarationSpecifiers
%destructor { frontend_drop_DeclarationSpecifiers($$); } DeclarationSpecifiers
%type <DeclarationSpecifier> DeclarationSpecifier
%destructor { frontend_drop_DeclarationSpecifier($$); } DeclarationSpecifier
%type <TypeSpecifier> TypeSpecifier
%destructor { frontend_drop_TypeSpecifier($$); } TypeSpecifier
%type <TypeQualifier> TypeQualifier
%destructor { frontend_drop_TypeQualifier($$); } TypeQualifier
%type <InitDeclaratorList> InitDeclaratorList
%destructor { frontend_drop_InitDeclaratorList($$); } InitDeclaratorList
%type <InitDeclarator> InitDeclarator
%destructor { frontend_drop_InitDeclarator($$); } InitDeclarator
%type <Declarator> Declarator
%destructor { frontend_drop_Declarator($$); } Declarator
%type <Pointer> Pointer
%destructor { frontend_drop_Pointer($$); } Pointer
%type <TypeQualifiers> TypeQualifiers
%destructor { frontend_drop_TypeQualifiers($$); } TypeQualifiers
%type <Declarator> DirectDeclarator
%destructor { frontend_drop_Declarator($$); } DirectDeclarator
%type <ParameterList> Parameters
%destructor { frontend_drop_ParameterList($$); } Parameters
%type <ParameterList> ParameterList
%destructor { frontend_drop_ParameterList($$); } ParameterList
%type <ParameterDeclaration> ParameterDeclaration
%destructor { frontend_drop_ParameterDeclaration($$); } ParameterDeclaration
%type <Initializer> Initializer
%destructor { frontend_drop_Initializer($$); } Initializer
%type <InitializerList> InitializerList
%destructor { frontend_drop_InitializerList($$); } InitializerList
%type <Expression> AssignExp
%destructor { frontend_drop_Expression($$); } AssignExp
%type <Expression> LOrExp
%destructor { frontend_drop_Expression($$); } LOrExp
%type <Expression> LAndExp
%destructor { frontend_drop_Expression($$); } LAndExp
%type <Expression> BOrExp
%destructor { frontend_drop_Expression($$); } BOrExp
%type <Expression> BNorExp
%destructor { frontend_drop_Expression($$); } BNorExp
%type <Expression> BAndExp
%destructor { frontend_drop_Expression($$); } BAndExp
%type <Expression> EqExp
%destructor { frontend_drop_Expression($$); } EqExp
%type <Expression> RelExp
%destructor { frontend_drop_Expression($$); } RelExp
%type <Expression> AddExp
%destructor { frontend_drop_Expression($$); } AddExp
%type <Expression> MulExp
%destructor { frontend_drop_Expression($$); } MulExp
%type <Expression> UnaryExp
%destructor { frontend_drop_Expression($$); } UnaryExp
%type <Expression> PostfixExp
%destructor { frontend_drop_Expression($$); } PostfixExp
%type <Expression> PrimaryExp
%destructor { frontend_drop_Expression($$); } PrimaryExp
%type <Expression> Exp
%destructor { frontend_drop_Expression($$); } Exp
%type <FuncRParamList> FuncRParams
%destructor { frontend_drop_FuncRParamList($$); } FuncRParams
%type <FuncRParamList> FuncRParamList
%destructor { frontend_drop_FuncRParamList($$); } FuncRParamList
%type <BlockItems> Block
%destructor { frontend_drop_BlockItems($$); } Block
%type <BlockItems> BlockItems
%destructor { frontend_drop_BlockItems($$); } BlockItems
%type <Stmt> Stmt
%destructor { frontend_drop_Stmt($$); } Stmt
%type <Stmt> IfMatchedStmt
%destructor { frontend_drop_Stmt($$); } IfMatchedStmt
%type <Stmt> IfUnMatchedStmt
%destructor { frontend_drop_Stmt($$); } IfUnMatchedStmt

%locations

%token UNSIGNED SIGNED
%token LONG SHORT
%token INT FLOAT DOUBLE CHAR
%token VOID
%token CONST VOLATILE
%token DO WHILE FOR BREAK CONTINUE
%token IF ELSE
%token RETURN

%token <ID> ID
%destructor { frontend_drop_ID($$); } ID
%token <CONSTNUM> CONSTNUM
%destructor { frontend_drop_CONSTNUM($$); } CONSTNUM
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
         | CompUnit error
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
              | DOUBLE   { $$ = malloc(sizeof(*$$)); $$->type = spec_DOUBLE;    }
              | CHAR     { $$ = malloc(sizeof(*$$)); $$->type = spec_CHAR;      }
              ;

TypeQualifier : CONST    { $$ = malloc(sizeof(*$$)); $$->type = qual_CONST;    }
              | VOLATILE { $$ = malloc(sizeof(*$$)); $$->type = qual_VOLATILE; }
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
                 | DirectDeclarator error
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

Stmt : IfMatchedStmt   { $$ = $1; }
     | IfUnMatchedStmt { $$ = $1; }
     ;

IfMatchedStmt : Block                                           { $$ = malloc(sizeof(*$$)); $$->type = tBlockStmt;    $$->select.BlockItems = $1;   $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | ';'                                             { $$ = malloc(sizeof(*$$)); $$->type = tExpStmt;      $$->select.Expression = NULL; $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | Exp ';'                                         { $$ = malloc(sizeof(*$$)); $$->type = tExpStmt;      $$->select.Expression = $1;   $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | RETURN Exp ';'                                  { $$ = malloc(sizeof(*$$)); $$->type = tReturnStmt;   $$->select.Expression = $2;   $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | RETURN ';'                                      { $$ = malloc(sizeof(*$$)); $$->type = tReturnStmt;   $$->select.Expression = NULL; $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | BREAK ';'                                       { $$ = malloc(sizeof(*$$)); $$->type = tBreakStmt;    $$->select.Expression = NULL; $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | CONTINUE ';'                                    { $$ = malloc(sizeof(*$$)); $$->type = tContinueStmt; $$->select.Expression = NULL; $$->Stmt_1 = NULL; $$->Stmt_2 = NULL; }
              | IF '(' Exp ')' IfMatchedStmt ELSE IfMatchedStmt { $$ = malloc(sizeof(*$$)); $$->type = tIfStmt;       $$->select.Expression = $3;   $$->Stmt_1 = $5;   $$->Stmt_2 = $7;   }
              | WHILE '(' Exp ')' IfMatchedStmt                 { $$ = malloc(sizeof(*$$)); $$->type = tWhileStmt;    $$->select.Expression = $3;   $$->Stmt_1 = $5;   $$->Stmt_2 = NULL; }
              | DO Stmt WHILE '(' Exp ')' ';'                   { $$ = malloc(sizeof(*$$)); $$->type = tDoWhileStmt;    $$->select.Expression = $5;   $$->Stmt_1 = $2;   $$->Stmt_2 = NULL; }
              ;

IfUnMatchedStmt : IF '(' Exp ')' IfMatchedStmt ELSE IfUnMatchedStmt { $$ = malloc(sizeof(*$$)); $$->type = tIfStmt;    $$->select.Expression = $3; $$->Stmt_1 = $5; $$->Stmt_2 = $7;   }
                | IF '(' Exp ')' Stmt                               { $$ = malloc(sizeof(*$$)); $$->type = tIfStmt;    $$->select.Expression = $3; $$->Stmt_1 = $5; $$->Stmt_2 = NULL; }
                | WHILE '(' Exp ')' IfUnMatchedStmt                 { $$ = malloc(sizeof(*$$)); $$->type = tWhileStmt; $$->select.Expression = $3; $$->Stmt_1 = $5; $$->Stmt_2 = NULL; }
                ;
%%
