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
typedef struct ParameterListNode *pParameterListNode;
typedef struct ParameterDeclarationNode *pParameterDeclarationNode;
typedef struct InitializerNode *pInitializerNode;
typedef struct InitializerListNode *pInitializerListNode;
typedef struct ExpressionNode *pExpressionNode;
typedef struct FuncRParamListNode *pFuncRParamListNode;
typedef struct BlockItemsNode *pBlockItemsNode;
typedef struct StmtNode *pStmtNode;

typedef char *pIDNode;
typedef struct CONSTNUMNode *pCONSTNUMNode;

void frontend_drop_CompUnit(pCompUnitNode CompUnit);
void frontend_drop_Declaration(pDeclarationNode Declaration);
void frontend_drop_DeclarationSpecifiers(pDeclarationSpecifiersNode DeclarationSpecifiers);
void frontend_drop_DeclarationSpecifier(pDeclarationSpecifierNode DeclarationSpecifier);
void frontend_drop_TypeSpecifier(pTypeSpecifierNode TypeSpecifier);
void frontend_drop_TypeQualifier(pTypeQualifierNode TypeQualifier);
void frontend_drop_InitDeclaratorList(pInitDeclaratorListNode InitDeclaratorList);
void frontend_drop_InitDeclarator(pInitDeclaratorNode InitDeclarator);
void frontend_drop_Declarator(pDeclaratorNode Declarator);
void frontend_drop_Pointer(pPointerNode Pointer);
void frontend_drop_TypeQualifiers(pTypeQualifiersNode TypeQualifiers);
void frontend_drop_ParameterList(pParameterListNode ParameterList);
void frontend_drop_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration);
void frontend_drop_Initializer(pInitializerNode Initializer);
void frontend_drop_InitializerList(pInitializerListNode InitializerList);
void frontend_drop_Expression(pExpressionNode Expression);
void frontend_drop_FuncRParamList(pFuncRParamListNode FuncRParamList);
void frontend_drop_BlockItems(pBlockItemsNode BlockItems);
void frontend_drop_Stmt(pStmtNode Stmt);
void frontend_drop_ID(pIDNode ID);
void frontend_drop_CONSTNUM(pCONSTNUMNode CONSTNUM);

void frontend_drop_syntaxtree(pCompUnitNode CompUnit);

struct CompUnitNode {
    pCompUnitNode CompUnit;
    pDeclarationNode Declaration;
};

struct DeclarationNode {
    bool is_func_def;
    pDeclarationSpecifiersNode DeclarationSpecifiers;
    union {
        pInitDeclaratorListNode InitDeclaratorList; // false
        pDeclaratorNode Declarator; // true
    } declarators;
    pBlockItemsNode BlockItems;
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
        spec_VOID,
        spec_UNSIGNED,
        spec_SIGNED,
        spec_LONG,
        spec_SHORT,
        spec_INT,
        spec_FLOAT,
        spec_CHAR,
    } type;
};

struct TypeQualifierNode {
    enum {
        qual_CONST,
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
    enum {
        tIDJust,
        tArrDec,
        tFunDec,
        tPointer,
    } type;
    union {
        pDeclaratorNode Declarator; // tArrDec tFunDec tPointer
        pIDNode ID; // tIDJust
    } body;
    union {
        pPointerNode Pointer; // tPointer
        pExpressionNode Expression; // tArrDec
        pParameterListNode ParameterList; // tFunDec
        void *IDsuffix; // tIDJust
    } select;
};

struct PointerNode {
    pTypeQualifiersNode TypeQualifiers;
    pPointerNode Pointer;
};

struct TypeQualifiersNode {
    pTypeQualifiersNode TypeQualifiers;
    pTypeQualifierNode TypeQualifier;
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
        pExpressionNode Expression; // false
    } select;
};

struct InitializerListNode {
    pInitializerListNode InitializerList;
    pInitializerNode Initializer;
};

struct ExpressionNode {
    enum {
        comma, assign,
        logic_or, logic_and,
        bit_or, bit_nor, bit_and,
        eq, neq,
        great, less, great_eq, less_eq,
        binary_add, binary_sub,
        binary_mul, binary_div, binary_mod,
        positive, negative, logic_not, bit_not, deref, ref, selfadd_pre, selfsub_pre,
        arrary_index, func_call, selfadd, selfsub,
        type_ID, type_CONSTNUM,
    } type;
    union {
        struct {
            pExpressionNode first;
            union {
                pExpressionNode Expression;
                pFuncRParamListNode FuncRParamList;
            } second;
        } exp;
        pIDNode ID;
        pCONSTNUMNode CONSTNUM;
    } select;
};

struct FuncRParamListNode {
    pFuncRParamListNode FuncRParamList;
    pExpressionNode Expression;
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
    enum {
        tBlockStmt,
        tExpStmt,
        tReturnStmt,
        tBreakStmt,
        tContinueStmt,
        tWhileStmt,
        tIfStmt,
    } type;
    union {
        pBlockItemsNode BlockItems; // '{'
        pExpressionNode Expression; // ';' RETURN ELSE WHILE
    } select;
    pStmtNode Stmt_1; // ELSE WHILE
    pStmtNode Stmt_2; // ELSE
};

struct CONSTNUMNode {
    enum {
        type_void, // void
        type_char, // char  signed char
        type_unsigned_char, // unsigned char
        type_short_int, // short  signed short  short int  signed short int
        type_unsigned_short_int, // unsigned short  unsigned short int
        type_int, // int  signed  signed int
        type_unsigned_int, // unsigned  unsigned int
        type_long_int, // long  signed long  long int  signed long int
        type_unsigned_long_int, // unsigned long  unsigned long int
        type_long_long_int, // long long  signed long long  long long int  signed long long int
        type_unsigned_long_long_int, // unsigned long long  unsigned long long int
    } valtype;
    union {
        char val_char;
        unsigned char val_unsigned_char;
        short int val_short_int;
        unsigned short int val_unsigned_short_int;
        int val_int;
        unsigned int val_unsigned_int;
        long int val_long_int;
        unsigned long int val_unsigned_long_int;
        long long int val_long_long_int;
        unsigned long long int val_unsigned_long_long_int;
    } val;
};

#endif