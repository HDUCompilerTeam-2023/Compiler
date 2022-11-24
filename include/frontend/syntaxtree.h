#ifndef __FRONTEND_SYNTAXTREE__
#define __FRONTEND_SYNTAXTREE__

#include <util.h>

typedef struct CompUnitNode *pCompUnitNode;
typedef struct DeclarationNode *pDeclarationNode;
typedef struct DeclarationSpecifiersNode *pDeclarationSpecifiersNode;
typedef struct DeclarationSpecifierNode *pDeclarationSpecifierNode;
typedef struct TypeSpecifierNode *pTypeSpecifierNode;
typedef struct TypeQualifierNode *pTypeQualifierNode;
typedef struct InitDeclaratorListNode *pInitDeclaratorListNode;
typedef struct InitDeclaratorNode *pInitDeclaratorNode;
typedef struct DeclaratorNode *pDeclaratorNode;
typedef struct PointerNode *pPointerNode;
typedef struct TypeQualifiersNode *pTypeQualifiersNode;
typedef struct DirectDeclaratorNode *pDirectDeclaratorNode;
typedef struct VarDirectDeclaratorNode *pVarDirectDeclaratorNode;
typedef struct FuncDirectDeclaratorNode *pFuncDirectDeclaratorNode;
typedef struct ParametersNode *pParametersNode;
typedef struct ParameterListNode *pParameterListNode;
typedef struct ParameterDeclarationNode *pParameterDeclarationNode;
typedef struct InitializerNode *pInitializerNode;
typedef struct InitializerListNode *pInitializerListNode;
typedef struct AssignExpNode *pAssignExpNode;
typedef struct LOrExpNode *pLOrExpNode;
typedef struct LAndExpNode *pLAndExpNode;
typedef struct BOrExpNode *pBOrExpNode;
typedef struct BNorExpNode *pBNorExpNode;
typedef struct BAndExpNode *pBAndExpNode;
typedef struct EqExpNode *pEqExpNode;
typedef struct RelExpNode *pRelExpNode;
typedef struct AddExpNode *pAddExpNode;
typedef struct MulExpNode *pMulExpNode;
typedef struct UnaryExpNode *pUnaryExpNode;
typedef struct PostfixExpNode *pPostfixExpNode;
typedef struct PrimaryExpNode *pPrimaryExpNode;
typedef struct ExpNode *pExpNode;
typedef struct NumberNode *pNumberNode;
typedef struct FuncRParamsNode *pFuncRParamsNode;
typedef struct FuncRParamListNode *pFuncRParamListNode;
typedef struct FuncRParamNode *pFuncRParamNode;
typedef struct FunctionDefNode *pFunctionDefNode;
typedef struct BlockNode *pBlockNode;
typedef struct BlockItemsNode *pBlockItemsNode;
typedef struct StmtNode *pStmtNode;
typedef struct IfMatchedStmtNode *pIfMatchedStmtNode;
typedef struct IfUnMatchedStmtNode *pIfUnMatchedStmtNode;

typedef struct IDNode *pIDNode;
typedef struct INTCONSTNode *pINTCONSTNode;

struct CompUnitNode {
    enum {
        tDeclaration,
        tFunctionDef,
    } type;
    pCompUnitNode CompUnit;
    union {
        pDeclarationNode Declaration;
        pFunctionDefNode FunctionDef;
    } select;
};

struct DeclarationNode {
  pDeclarationSpecifiersNode DeclarationSpecifiers;
  pInitDeclaratorListNode InitDeclaratorList;
};

struct DeclarationSpecifiersNode {
  pDeclarationSpecifiersNode DeclarationSpecifiers;
  pDeclarationSpecifierNode DeclarationSpecifier;
};

struct DeclarationSpecifierNode {
    enum {
        tTypeSpecifier,
        tTypeQualifier,
    } type;
    union{
        pTypeSpecifierNode TypeSpecifier;
        pTypeQualifierNode TypeQualifier;
    } select;
};

struct TypeSpecifierNode {
    enum {
        tVOID,
        tINT,
        tFLOAT,
    } type;
};

struct TypeQualifierNode {
    enum {
        tCONST,
    } type;
};

struct InitDeclaratorListNode {
    pInitDeclaratorListNode InitDeclaratorList;
    pInitDeclaratorNode InitDeclarator;
};

struct InitDeclaratorNode {
    pDeclaratorNode Declarator;
    pInitializerNode Initializer;
};

struct DeclaratorNode {
    pPointerNode Pointer;
    pDirectDeclaratorNode DirectDeclarator;
};

struct PointerNode {
    pTypeQualifiersNode TypeQualifiers;
    pPointerNode Pointer;
};

struct TypeQualifiersNode {
    pTypeQualifiersNode TypeQualifiers;
    pTypeQualifierNode TypeQualifier;
};

struct DirectDeclaratorNode {
    enum {
        tVarDirectDeclarator,
        tFuncDirectDeclarator,
    } type;
    union {
        pVarDirectDeclaratorNode VarDirectDeclarator;
        pFuncDirectDeclaratorNode FuncDirectDeclarator;
    } select;
};

struct VarDirectDeclaratorNode {
    int op;
    union {
        pVarDirectDeclaratorNode VarDirectDeclarator; // '['
        pDeclaratorNode Declarator; // '('
        pIDNode ID; // ID
    } select;
    pAssignExpNode AssignExp; // '['
};

struct FuncDirectDeclaratorNode {
    bool SimpleFunc;
    union {
        pDeclaratorNode Declarator; // false
        pIDNode ID; // true
    } select;
    pParametersNode Parameters;
};

struct ParametersNode {
    pParameterListNode ParameterList;
};

struct ParameterListNode {
    pParameterListNode ParameterList;
    pParameterDeclarationNode ParameterDeclaration;
};

struct ParameterDeclarationNode {
  pDeclarationSpecifiersNode DeclarationSpecifiers;
  pDeclaratorNode Declarator;
};

struct InitializerNode {
    bool isList;
    union {
        pInitializerListNode InitializerList; // true
        pAssignExpNode AssignExp; // false
    } select;
};

struct InitializerListNode {
    pInitializerListNode InitializerList;
    pInitializerNode Initializer;
};

struct AssignExpNode {
    union {
        pUnaryExpNode UnaryExp; // NOT NULL
        pLOrExpNode LOrExp; // NULL
    } select;
    pAssignExpNode AssignExp;
};

struct LOrExpNode {
    pLOrExpNode LOrExp;
    pLAndExpNode LAndExp;
};

struct LAndExpNode {
    pLAndExpNode LAndExp;
    pBOrExpNode BOrExp;
};

struct BOrExpNode {
    pBOrExpNode BOrExp;
    pBNorExpNode BNorExp;
};

struct BNorExpNode {
    pBNorExpNode BNorExp;
    pBAndExpNode BAndExp;
};

struct BAndExpNode {
    pBAndExpNode BAndExp;
    pEqExpNode EqExp;
};

struct EqExpNode {
    int op;
    pEqExpNode EqExp;
    pRelExpNode RelExp;
};

struct RelExpNode {
    int op;
    pRelExpNode RelExp;
    pAddExpNode AddExp;
};

struct AddExpNode {
    int op;
    pAddExpNode AddExp;
    pMulExpNode MulExp;
};

struct MulExpNode {
    int op;
    pMulExpNode MulExp; 
    pUnaryExpNode UnaryExp;
};

struct UnaryExpNode {
    int op;
    union {
        pUnaryExpNode UnaryExp; // '+' '-' '!' '~' SELFADD SELFSUB
        pPostfixExpNode PostfixExp; // YYEMPTY
    } select;
};

struct PostfixExpNode {
    int op;
    union {
        pPostfixExpNode PostfixExp; // '[' '(' SELFADD SELFSUB
        pPrimaryExpNode PrimaryExp; // YYEMPTY
    } select;
    union {
        pExpNode Exp; // '['
        pFuncRParamsNode FuncRParams; // '('
    } suffix;
};

struct PrimaryExpNode {
    int type;
    union {
        pExpNode Exp; // '('
        pNumberNode Number; // INTCONST
        pIDNode ID; // ID
    } select;
};

struct ExpNode {
    pExpNode Exp;
    pAssignExpNode AssignExp;
};

struct NumberNode {
    int type;
    union {
        pINTCONSTNode INTCONST; // INTCONST
    } select;
};

struct FuncRParamsNode {
    pFuncRParamListNode FuncRParamList;
};

struct FuncRParamListNode {
    pFuncRParamListNode FuncRParamList;
    pFuncRParamNode FuncRParam;
};

struct FuncRParamNode {
    pAssignExpNode AssignExp;
};

struct FunctionDefNode {
    pDeclarationSpecifiersNode DeclarationSpecifiers;
    pPointerNode Pointer;
    pIDNode ID;
    pParametersNode Parameters;
    pBlockNode Block;
};

struct BlockNode {
    pBlockItemsNode BlockItems;
};

struct BlockItemsNode {
    bool isStmt;
    pBlockItemsNode BlockItems;
    union {
        pStmtNode Stmt; // true
        pDeclarationNode Declaration; // false
    } select;
};

struct StmtNode {
    bool isMatched;
    union {
        pIfMatchedStmtNode IfMatchedStmt;
        pIfUnMatchedStmtNode IfUnMatchedStmt;
    } select;
};

struct IfMatchedStmtNode {
    int type;
    union {
        pBlockNode Block; // '{'
        pExpNode Exp; // ';' RETURN ELSE WHILE
    } select;
    pIfMatchedStmtNode IfMatchedStmt_1; // ELSE WHILE
    pIfMatchedStmtNode IfMatchedStmt_2; // ELSE
};

struct IfUnMatchedStmtNode {
    int type;
    pExpNode Exp;
    union {
        pIfMatchedStmtNode IfMatchedStmt; // ELSE
        pStmtNode Stmt; // IF
        pIfUnMatchedStmtNode IfUnMatchedStmt; // WHILE
    } select;
    pIfUnMatchedStmtNode IfUnMatchedStmt; // ELSE
};

struct IDNode {
    char *str;
};

struct INTCONSTNode {
    enum {
        type_int,
    } type;
    union {
        int val_int;
    } val;
};

#endif