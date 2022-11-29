#include <frontend/syntaxtree.h>
#include <parser.h>

#include <stdlib.h>

#define YYEMPTY -2 // define in bison output .tab.c

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
void frontend_drop_DirectDeclarator(pDirectDeclaratorNode DirectDeclarator);
void frontend_drop_Parameters(pParametersNode Parameters);
void frontend_drop_ParameterList(pParameterListNode ParameterList);
void frontend_drop_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration);
void frontend_drop_Initializer(pInitializerNode Initializer);
void frontend_drop_InitializerList(pInitializerListNode InitializerList);
void frontend_drop_AssignExp(pAssignExpNode AssignExp);
void frontend_drop_LOrExp(pLOrExpNode LOrExp);
void frontend_drop_LAndExp(pLAndExpNode LAndExp);
void frontend_drop_BOrExp(pBOrExpNode BOrExp);
void frontend_drop_BNorExp(pBNorExpNode BNorExp);
void frontend_drop_BAndExp(pBAndExpNode BAndExp);
void frontend_drop_EqExp(pEqExpNode EqExp);
void frontend_drop_RelExp(pRelExpNode RelExp);
void frontend_drop_AddExp(pAddExpNode AddExp);
void frontend_drop_MulExp(pMulExpNode MulExp);
void frontend_drop_UnaryExp(pUnaryExpNode UnaryExp);
void frontend_drop_PostfixExp(pPostfixExpNode PostfixExp);
void frontend_drop_PrimaryExp(pPrimaryExpNode PrimaryExp);
void frontend_drop_Exp(pExpNode Exp);
void frontend_drop_Number(pNumberNode Number);
void frontend_drop_FuncRParams(pFuncRParamsNode FuncRParams);
void frontend_drop_FuncRParamList(pFuncRParamListNode FuncRParamList);
void frontend_drop_FuncRParam(pFuncRParamNode FuncRParam);
void frontend_drop_Block(pBlockNode Block);
void frontend_drop_BlockItems(pBlockItemsNode BlockItems);
void frontend_drop_Stmt(pStmtNode Stmt);
void frontend_drop_IfMatchedStmt(pIfMatchedStmtNode IfMatchedStmt);
void frontend_drop_IfUnMatchedStmt(pIfUnMatchedStmtNode IfUnMatchedStmt);
void frontend_drop_ID(pIDNode ID);
void frontend_drop_INTCONST(pINTCONSTNode INTCONST);

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
    if (Declaration->Block) {
        frontend_drop_Declarator(Declaration->declarators.Declarator);
        frontend_drop_Block(Declaration->Block);
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
    frontend_drop_Pointer(Declarator->Pointer);
    frontend_drop_DirectDeclarator(Declarator->DirectDeclarator);
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

void frontend_drop_DirectDeclarator(pDirectDeclaratorNode DirectDeclarator) {
    switch (DirectDeclarator->op) {
    case tRecurr:
        frontend_drop_Declarator(DirectDeclarator->select.Declarator);
        break;
    case tIDJust:
        frontend_drop_ID(DirectDeclarator->select.ID);
        break;
    case tArrDec:
        frontend_drop_DirectDeclarator(DirectDeclarator->select.DirectDeclarator);
        if (DirectDeclarator->AssignExp) {
            frontend_drop_AssignExp(DirectDeclarator->AssignExp);
        }
        break;
    case tFunDec:
        frontend_drop_DirectDeclarator(DirectDeclarator->select.DirectDeclarator);
        if (DirectDeclarator->Parameters) {
            frontend_drop_Parameters(DirectDeclarator->Parameters);
        }
        break;
    }
    free(DirectDeclarator);
}

void frontend_drop_Parameters(pParametersNode Parameters) {
    if (Parameters->ParameterList)
        frontend_drop_ParameterList(Parameters->ParameterList);
    free(Parameters);
}

void frontend_drop_ParameterList(pParameterListNode ParameterList) {
    if (ParameterList->ParameterList) {
        frontend_drop_ParameterList(ParameterList->ParameterList);
    }
    frontend_drop_ParameterDeclaration(ParameterList->ParameterDeclaration);
    free(ParameterList);
}

void frontend_drop_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration) {
    frontend_drop_DeclarationSpecifiers(ParameterDeclaration->DeclarationSpecifiers);
    frontend_drop_Declarator(ParameterDeclaration->Declarator);
    free(ParameterDeclaration);
}

void frontend_drop_Initializer(pInitializerNode Initializer) {
    if (Initializer->isList) {
        if (Initializer->select.InitializerList)
            frontend_drop_InitializerList(Initializer->select.InitializerList);
    }
    else {
        frontend_drop_AssignExp(Initializer->select.AssignExp);
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

void frontend_drop_AssignExp(pAssignExpNode AssignExp) {
    if (AssignExp->AssignExp) {
        frontend_drop_UnaryExp(AssignExp->select.UnaryExp);
        frontend_drop_AssignExp(AssignExp->AssignExp);
    }
    else {
        frontend_drop_LOrExp(AssignExp->select.LOrExp);
    }
    free(AssignExp);
}

void frontend_drop_LOrExp(pLOrExpNode LOrExp) {
    if (LOrExp->LOrExp) {
        frontend_drop_LOrExp(LOrExp->LOrExp);
        frontend_drop_LAndExp(LOrExp->LAndExp);
    }
    else {
        frontend_drop_LAndExp(LOrExp->LAndExp);
    }
    free(LOrExp);
}

void frontend_drop_LAndExp(pLAndExpNode LAndExp) {
    if (LAndExp->LAndExp) {
        frontend_drop_LAndExp(LAndExp->LAndExp);
        frontend_drop_BOrExp(LAndExp->BOrExp);
    }
    else {
        frontend_drop_BOrExp(LAndExp->BOrExp);
    }
    free(LAndExp);
}

void frontend_drop_BOrExp(pBOrExpNode BOrExp) {
    if (BOrExp->BOrExp) {
        frontend_drop_BOrExp(BOrExp->BOrExp);
        frontend_drop_BNorExp(BOrExp->BNorExp);
    }
    else {
        frontend_drop_BNorExp(BOrExp->BNorExp);
    }
    free(BOrExp);
}

void frontend_drop_BNorExp(pBNorExpNode BNorExp) {
    if (BNorExp->BNorExp) {
        frontend_drop_BNorExp(BNorExp->BNorExp);
        frontend_drop_BAndExp(BNorExp->BAndExp);
    }
    else {
        frontend_drop_BAndExp(BNorExp->BAndExp);
    }
    free(BNorExp);
}

void frontend_drop_BAndExp(pBAndExpNode BAndExp) {
    if (BAndExp->BAndExp) {
        frontend_drop_BAndExp(BAndExp->BAndExp);
        frontend_drop_EqExp(BAndExp->EqExp);
    }
    else {
        frontend_drop_EqExp(BAndExp->EqExp);
    }
    free(BAndExp);
}

void frontend_drop_EqExp(pEqExpNode EqExp) {
    if (EqExp->EqExp) {
        frontend_drop_EqExp(EqExp->EqExp);
        frontend_drop_RelExp(EqExp->RelExp);
    }
    else {
        frontend_drop_RelExp(EqExp->RelExp);
    }
    free(EqExp);
}

void frontend_drop_RelExp(pRelExpNode RelExp) {
    if (RelExp->RelExp) {
        frontend_drop_RelExp(RelExp->RelExp);
        frontend_drop_AddExp(RelExp->AddExp);
    }
    else {
        frontend_drop_AddExp(RelExp->AddExp);
    }
    free(RelExp);
}

void frontend_drop_AddExp(pAddExpNode AddExp) {
    if (AddExp->AddExp) {
        frontend_drop_AddExp(AddExp->AddExp);
        frontend_drop_MulExp(AddExp->MulExp);
    }
    else {
        frontend_drop_MulExp(AddExp->MulExp);
    }
    free(AddExp);
}

void frontend_drop_MulExp(pMulExpNode MulExp) {
    if (MulExp->MulExp) {
        frontend_drop_MulExp(MulExp->MulExp);
        frontend_drop_UnaryExp(MulExp->UnaryExp);
    }
    else {
        frontend_drop_UnaryExp(MulExp->UnaryExp);
    }
    free(MulExp);
}

void frontend_drop_UnaryExp(pUnaryExpNode UnaryExp) {
    switch (UnaryExp->op) {
    case '-':
    case '+':
    case '!':
    case '~':
    case '*':
    case '&':
    case SELFADD:
    case SELFSUB:
        frontend_drop_UnaryExp(UnaryExp->select.UnaryExp);
        break;
    case YYEMPTY:
        frontend_drop_PostfixExp(UnaryExp->select.PostfixExp);
        break;
    }
    free(UnaryExp);
}

void frontend_drop_PostfixExp(pPostfixExpNode PostfixExp) {
    switch (PostfixExp->op) {
    case '[':
        frontend_drop_PostfixExp(PostfixExp->select.PostfixExp);
        frontend_drop_Exp(PostfixExp->suffix.Exp);
        break;
    case '(':
        frontend_drop_PostfixExp(PostfixExp->select.PostfixExp);
        frontend_drop_FuncRParams(PostfixExp->suffix.FuncRParams);
        break;
    case SELFADD:
    case SELFSUB:
        frontend_drop_PostfixExp(PostfixExp->select.PostfixExp);
        break;
    case YYEMPTY:
        frontend_drop_PrimaryExp(PostfixExp->select.PrimaryExp);
        break;
    }
    free(PostfixExp);
}

void frontend_drop_PrimaryExp(pPrimaryExpNode PrimaryExp) {
    switch (PrimaryExp->type) {
    case '(':
        frontend_drop_Exp(PrimaryExp->select.Exp);
        break;
    case INTCONST:
        frontend_drop_Number(PrimaryExp->select.Number);
        break;
    case ID:
        frontend_drop_ID(PrimaryExp->select.ID);
        break;
    }
    free(PrimaryExp);
}

void frontend_drop_Exp(pExpNode Exp) {
    if (Exp->Exp) {
        frontend_drop_Exp(Exp->Exp);
        frontend_drop_AssignExp(Exp->AssignExp);
    }
    else {
        frontend_drop_AssignExp(Exp->AssignExp);
    }
    free(Exp);
}

void frontend_drop_Number(pNumberNode Number) {
    switch (Number->type) {
    case INTCONST:
        frontend_drop_INTCONST(Number->select.INTCONST);
        break;
    }
    free(Number);
}

void frontend_drop_FuncRParams(pFuncRParamsNode FuncRParams) {
    if (FuncRParams->FuncRParamList)
        frontend_drop_FuncRParamList(FuncRParams->FuncRParamList);
    free(FuncRParams);
}

void frontend_drop_FuncRParamList(pFuncRParamListNode FuncRParamList) {
    if (FuncRParamList->FuncRParamList) {
        frontend_drop_FuncRParamList(FuncRParamList->FuncRParamList);
    }
    frontend_drop_FuncRParam(FuncRParamList->FuncRParam);
    free(FuncRParamList);
}

void frontend_drop_FuncRParam(pFuncRParamNode FuncRParam) {
    frontend_drop_AssignExp(FuncRParam->AssignExp);
    free(FuncRParam);
}

void frontend_drop_Block(pBlockNode Block) {
    frontend_drop_BlockItems(Block->BlockItems);
    free(Block);
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
    if (Stmt->isMatched) {
        frontend_drop_IfMatchedStmt(Stmt->select.IfMatchedStmt);
    }
    else {
        frontend_drop_IfUnMatchedStmt(Stmt->select.IfUnMatchedStmt);
    }
    free(Stmt);
}

void frontend_drop_IfMatchedStmt(pIfMatchedStmtNode IfMatchedStmt) {
    switch (IfMatchedStmt->type) {
    case '{':
        frontend_drop_Block(IfMatchedStmt->select.Block);
        break;
    case ';':
    case RETURN:
        if (IfMatchedStmt->select.Exp){
            frontend_drop_Exp(IfMatchedStmt->select.Exp);
        }
    case BREAK:
    case CONTINUE:
        break;
    case ELSE:
        frontend_drop_Exp(IfMatchedStmt->select.Exp);
        frontend_drop_IfMatchedStmt(IfMatchedStmt->IfMatchedStmt_1);
        frontend_drop_IfMatchedStmt(IfMatchedStmt->IfMatchedStmt_2);
        break;
    case WHILE:
        frontend_drop_Exp(IfMatchedStmt->select.Exp);
        frontend_drop_IfMatchedStmt(IfMatchedStmt->IfMatchedStmt_1);
        break;
    }
    free(IfMatchedStmt);
}

void frontend_drop_IfUnMatchedStmt(pIfUnMatchedStmtNode IfUnMatchedStmt) {
    switch (IfUnMatchedStmt->type) {
    case IF:
        frontend_drop_Exp(IfUnMatchedStmt->Exp);
        frontend_drop_Stmt(IfUnMatchedStmt->select.Stmt);
        break;
    case ELSE:
        frontend_drop_Exp(IfUnMatchedStmt->Exp);
        frontend_drop_IfMatchedStmt(IfUnMatchedStmt->select.IfMatchedStmt);
        frontend_drop_IfUnMatchedStmt(IfUnMatchedStmt->IfUnMatchedStmt);
        break;
    case WHILE:
        frontend_drop_Exp(IfUnMatchedStmt->Exp);
        frontend_drop_IfUnMatchedStmt(IfUnMatchedStmt->select.IfUnMatchedStmt);
        break;
    }
    free(IfUnMatchedStmt);
}

void frontend_drop_ID(pIDNode ID) {
    free(ID->str);
    free(ID);
}

void frontend_drop_INTCONST(pINTCONSTNode INTCONST) {
    free(INTCONST);
}
