#include <hir.h>
#include <hir/syntaxtree.h>

#include <stdlib.h>

#define YYEMPTY -2 // define in bison output .tab.c

void frontend_drop_syntaxtree(pCompUnitNode CompUnit) {
    frontend_drop_CompUnit(CompUnit);
}

void frontend_drop_CompUnit(pCompUnitNode CompUnit) {
    if (!CompUnit)
        return;
    frontend_drop_CompUnit(CompUnit->CompUnit);
    frontend_drop_Declaration(CompUnit->Declaration);
    free(CompUnit);
}

void frontend_drop_Declaration(pDeclarationNode Declaration) {
    frontend_drop_DeclarationSpecifiers(Declaration->DeclarationSpecifiers);
    if (Declaration->is_func_def) {
        frontend_drop_Declarator(Declaration->declarators.Declarator);
        frontend_drop_BlockItems(Declaration->BlockItems);
    }
    else {
        frontend_drop_InitDeclaratorList(Declaration->declarators.InitDeclaratorList);
    }
    free(Declaration);
}

void frontend_drop_DeclarationSpecifiers(pDeclarationSpecifiersNode DeclarationSpecifiers) {
    if (DeclarationSpecifiers->DeclarationSpecifiers) {
        frontend_drop_DeclarationSpecifiers(DeclarationSpecifiers->DeclarationSpecifiers);
    }
    frontend_drop_DeclarationSpecifier(DeclarationSpecifiers->DeclarationSpecifier);
    free(DeclarationSpecifiers);
}

void frontend_drop_DeclarationSpecifier(pDeclarationSpecifierNode DeclarationSpecifier) {
    if (DeclarationSpecifier->type == tTypeQualifier) {
        frontend_drop_TypeQualifier(DeclarationSpecifier->select.TypeQualifier);
    }
    else if (DeclarationSpecifier->type == tTypeSpecifier) {
        frontend_drop_TypeSpecifier(DeclarationSpecifier->select.TypeSpecifier);
    }
    free(DeclarationSpecifier);
}

void frontend_drop_TypeSpecifier(pTypeSpecifierNode TypeSpecifier) {
    free(TypeSpecifier);
}

void frontend_drop_TypeQualifier(pTypeQualifierNode TypeQualifier) {
    free(TypeQualifier);
}

void frontend_drop_InitDeclaratorList(pInitDeclaratorListNode InitDeclaratorList) {
    if (InitDeclaratorList->InitDeclaratorList) {
        frontend_drop_InitDeclaratorList(InitDeclaratorList->InitDeclaratorList);
    }
    frontend_drop_InitDeclarator(InitDeclaratorList->InitDeclarator);
    free(InitDeclaratorList);
}

void frontend_drop_InitDeclarator(pInitDeclaratorNode InitDeclarator) {
    frontend_drop_Declarator(InitDeclarator->Declarator);
    if(InitDeclarator->Initializer) {
        frontend_drop_Initializer(InitDeclarator->Initializer);
    }
    free(InitDeclarator);
}

void frontend_drop_Declarator(pDeclaratorNode Declarator) {
    switch (Declarator->type) {
    case tPointer:
        if (Declarator->select.Pointer) {
            frontend_drop_Pointer(Declarator->select.Pointer);
        }
        frontend_drop_Declarator(Declarator->body.Declarator);
        break;
    case tArrDec:
        frontend_drop_Declarator(Declarator->body.Declarator);
        if (Declarator->select.Expression) {
            frontend_drop_Expression(Declarator->select.Expression);
        }
        break;
    case tFunDec:
        frontend_drop_Declarator(Declarator->body.Declarator);
        if (Declarator->select.ParameterList) {
            frontend_drop_ParameterList(Declarator->select.ParameterList);
        }
        break;
    case tIDJust:
        frontend_drop_ID(Declarator->body.ID);
        assert(!Declarator->select.IDsuffix);
        break;
    }
    free(Declarator);
}

void frontend_drop_Pointer(pPointerNode Pointer) {
    if(!Pointer)
        return;
    frontend_drop_TypeQualifiers(Pointer->TypeQualifiers);
    frontend_drop_Pointer(Pointer->Pointer);
    free(Pointer);
}

void frontend_drop_TypeQualifiers(pTypeQualifiersNode TypeQualifiers) {
    if(!TypeQualifiers)
        return;
    frontend_drop_TypeQualifiers(TypeQualifiers->TypeQualifiers);
    frontend_drop_TypeQualifier(TypeQualifiers->TypeQualifier);
    free(TypeQualifiers);
}

void frontend_drop_ParameterList(pParameterListNode ParameterList) {
    if (!ParameterList) return;
    if (ParameterList->ParameterList) {
        frontend_drop_ParameterList(ParameterList->ParameterList);
    }
    frontend_drop_ParameterDeclaration(ParameterList->ParameterDeclaration);
    free(ParameterList);
}

void frontend_drop_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration) {
    frontend_drop_DeclarationSpecifiers(ParameterDeclaration->DeclarationSpecifiers);
    if (ParameterDeclaration->Declarator) frontend_drop_Declarator(ParameterDeclaration->Declarator);
    free(ParameterDeclaration);
}

void frontend_drop_Initializer(pInitializerNode Initializer) {
    if (Initializer->isList) {
        if (Initializer->select.InitializerList)
            frontend_drop_InitializerList(Initializer->select.InitializerList);
    }
    else {
        frontend_drop_Expression(Initializer->select.Expression);
    }
    free(Initializer);
}

void frontend_drop_InitializerList(pInitializerListNode InitializerList) {
    if (InitializerList->InitializerList) {
        frontend_drop_InitializerList(InitializerList->InitializerList);
    }
    frontend_drop_Initializer(InitializerList->Initializer);
    free(InitializerList);
}

void frontend_drop_Expression(pExpressionNode Expression) {
    assert(Expression);
    switch (Expression->type) {
    case type_ID:
        frontend_drop_ID(Expression->select.ID);
        break;
    case type_CONSTNUM:
        frontend_drop_CONSTNUM(Expression->select.CONSTNUM);
        break;
    case func_call:
        frontend_drop_Expression(Expression->select.exp.first);
        frontend_drop_FuncRParamList(Expression->select.exp.second.FuncRParamList);
        break;
    case positive:
    case negative:
    case logic_not:
    case bit_not:
    case deref:
    case ref:
    case selfadd_pre:
    case selfsub_pre:
    case selfadd:
    case selfsub:
        frontend_drop_Expression(Expression->select.exp.first);
        break;
    default:
        frontend_drop_Expression(Expression->select.exp.first);
        frontend_drop_Expression(Expression->select.exp.second.Expression);
        break;
    }
    free(Expression);
}

void frontend_drop_FuncRParamList(pFuncRParamListNode FuncRParamList) {
    if (!FuncRParamList) return;
    if (FuncRParamList->FuncRParamList) {
        frontend_drop_FuncRParamList(FuncRParamList->FuncRParamList);
    }
    frontend_drop_Expression(FuncRParamList->Expression);
    free(FuncRParamList);
}

void frontend_drop_BlockItems(pBlockItemsNode BlockItems) {
    if (!BlockItems)
        return;
    frontend_drop_BlockItems(BlockItems->BlockItems);
    if (BlockItems->isStmt) {
        frontend_drop_Stmt(BlockItems->select.Stmt);
    }
    else {
        frontend_drop_Declaration(BlockItems->select.Declaration);
    }
    free(BlockItems);
}

void frontend_drop_Stmt(pStmtNode Stmt) {
    switch (Stmt->type) {
    case tBlockStmt:
        frontend_drop_BlockItems(Stmt->select.BlockItems);
        break;
    case tExpStmt:
    case tReturnStmt:
        if(Stmt->select.Expression)
            frontend_drop_Expression(Stmt->select.Expression);
        break;
    case tIfStmt:
        if (Stmt->Stmt_2)
            frontend_drop_Stmt(Stmt->Stmt_2);
    case tDoWhileStmt:
    case tWhileStmt:
        frontend_drop_Expression(Stmt->select.Expression);
        frontend_drop_Stmt(Stmt->Stmt_1);
    case tBreakStmt:
    case tContinueStmt:
        break;
    }
    free(Stmt);
}

void frontend_drop_ID(pIDNode ID) {
    free(ID);
}

void frontend_drop_CONSTNUM(pCONSTNUMNode CONSTNUM) {
    free(CONSTNUM);
}
