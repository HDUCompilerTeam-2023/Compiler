#include <frontend/syntaxtree_printer.h>
#include <parser.h>
#include <lexer.h>

#include <stdio.h>

#define YYEMPTY -2 // define in bison output .tab.c

void frontend_print_CompUnit(pCompUnitNode CompUnit);
void frontend_print_Declaration(pDeclarationNode Declaration, int depth);
void frontend_print_DeclarationSpecifiers(pDeclarationSpecifiersNode DeclarationSpecifiers);
void frontend_print_DeclarationSpecifier(pDeclarationSpecifierNode DeclarationSpecifier);
void frontend_print_TypeSpecifier(pTypeSpecifierNode TypeSpecifier);
void frontend_print_TypeQualifier(pTypeQualifierNode TypeQualifier);
void frontend_print_InitDeclaratorList(pDeclarationSpecifiersNode DeclarationSpecifiers, pInitDeclaratorListNode InitDeclaratorList, int depth);
void frontend_print_InitDeclarator(pInitDeclaratorNode InitDeclarator);
void frontend_print_Declarator(pDeclaratorNode Declarator);
void frontend_print_Pointer(pPointerNode Pointer);
void frontend_print_TypeQualifiers(pTypeQualifiersNode TypeQualifiers);
void frontend_print_DirectDeclarator(pDirectDeclaratorNode DirectDeclarator);
void frontend_print_Parameters(pParametersNode Parameters);
void frontend_print_ParameterList(pParameterListNode ParameterList);
void frontend_print_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration);
void frontend_print_Initializer(pInitializerNode Initializer);
void frontend_print_InitializerList(pInitializerListNode InitializerList);
void frontend_print_AssignExp(pAssignExpNode AssignExp);
void frontend_print_LOrExp(pLOrExpNode LOrExp);
void frontend_print_LAndExp(pLAndExpNode LAndExp);
void frontend_print_BOrExp(pBOrExpNode BOrExp);
void frontend_print_BNorExp(pBNorExpNode BNorExp);
void frontend_print_BAndExp(pBAndExpNode BAndExp);
void frontend_print_EqExp(pEqExpNode EqExp);
void frontend_print_RelExp(pRelExpNode RelExp);
void frontend_print_AddExp(pAddExpNode AddExp);
void frontend_print_MulExp(pMulExpNode MulExp);
void frontend_print_UnaryExp(pUnaryExpNode UnaryExp);
void frontend_print_PostfixExp(pPostfixExpNode PostfixExp);
void frontend_print_PrimaryExp(pPrimaryExpNode PrimaryExp);
void frontend_print_Exp(pExpNode Exp);
void frontend_print_Number(pNumberNode Number);
void frontend_print_FuncRParams(pFuncRParamsNode FuncRParams);
void frontend_print_FuncRParamList(pFuncRParamListNode FuncRParamList);
void frontend_print_FuncRParam(pFuncRParamNode FuncRParam);
void frontend_print_Block(pBlockNode Block, int depth);
void frontend_print_BlockItems(pBlockItemsNode BlockItems, int depth);
void frontend_print_Stmt(pStmtNode Stmt, int depth);
void frontend_print_IfMatchedStmt(pIfMatchedStmtNode IfMatchedStmt, int depth);
void frontend_print_IfUnMatchedStmt(pIfUnMatchedStmtNode IfUnMatchedStmt, int depth);
void frontend_print_ID(pIDNode ID);
void frontend_print_INTCONST(pINTCONSTNode INTCONST);

void frontend_print_syntaxtree(pCompUnitNode CompUnit) {
    frontend_print_CompUnit(CompUnit);
}

void frontend_print_CompUnit(pCompUnitNode CompUnit) {
    if (!CompUnit)
        return;
    frontend_print_CompUnit(CompUnit->CompUnit);
    frontend_print_Declaration(CompUnit->Declaration, 0);
}

void frontend_print_Declaration(pDeclarationNode Declaration, int depth) {
    if (Declaration->Block) {
        frontend_print_DeclarationSpecifiers(Declaration->DeclarationSpecifiers);
        frontend_print_Declarator(Declaration->declarators.Declarator);
        printf("\n");
        frontend_print_Block(Declaration->Block, depth + 1);
    }
    else
        frontend_print_InitDeclaratorList(Declaration->DeclarationSpecifiers, Declaration->declarators.InitDeclaratorList, depth);
}

void frontend_print_DeclarationSpecifiers(pDeclarationSpecifiersNode DeclarationSpecifiers) {
    if (DeclarationSpecifiers->DeclarationSpecifiers) {
        frontend_print_DeclarationSpecifiers(DeclarationSpecifiers->DeclarationSpecifiers);
    }
    frontend_print_DeclarationSpecifier(DeclarationSpecifiers->DeclarationSpecifier);
}

void frontend_print_DeclarationSpecifier(pDeclarationSpecifierNode DeclarationSpecifier) {
    if (DeclarationSpecifier->type == tTypeQualifier) {
        frontend_print_TypeQualifier(DeclarationSpecifier->select.TypeQualifier);
    }
    else if (DeclarationSpecifier->type == tTypeSpecifier) {
        frontend_print_TypeSpecifier(DeclarationSpecifier->select.TypeSpecifier);
    }
}

void frontend_print_TypeSpecifier(pTypeSpecifierNode TypeSpecifier) {
    switch (TypeSpecifier->type) {
    case tVOID:
        printf("void");
        break;
    case tUNSIGNED:
        printf("unsigned");
        break;
    case tSIGNED:
        printf("signed");
        break;
    case tLONG:
        printf("long");
        break;
    case tSHORT:
        printf("short");
        break;
    case tINT:
        printf("int");
        break;
    case tFLOAT:
        printf("float");
        break;
    case tCHAR:
        printf("char");
        break;
    }
    printf(" ");
}

void frontend_print_TypeQualifier(pTypeQualifierNode TypeQualifier) {
    switch (TypeQualifier->type) {
    case tCONST:
        printf("const");
        break;
    }
    printf(" ");
}

void frontend_print_InitDeclaratorList(pDeclarationSpecifiersNode DeclarationSpecifiers, pInitDeclaratorListNode InitDeclaratorList, int depth) {
    if (InitDeclaratorList->InitDeclaratorList) {
        frontend_print_InitDeclaratorList(DeclarationSpecifiers, InitDeclaratorList->InitDeclaratorList, depth);
    }
    for (int i = 0; i < depth; ++i) printf("    ");
    frontend_print_DeclarationSpecifiers(DeclarationSpecifiers);
    frontend_print_InitDeclarator(InitDeclaratorList->InitDeclarator);
    printf(";\n");
}

void frontend_print_InitDeclarator(pInitDeclaratorNode InitDeclarator) {
    frontend_print_Declarator(InitDeclarator->Declarator);
    if(InitDeclarator->Initializer) {
        printf(" = ");
        frontend_print_Initializer(InitDeclarator->Initializer);
    }
}

void frontend_print_Declarator(pDeclaratorNode Declarator) {
    frontend_print_Pointer(Declarator->Pointer);
    frontend_print_DirectDeclarator(Declarator->DirectDeclarator);
}

void frontend_print_Pointer(pPointerNode Pointer) {
    if(!Pointer)
        return;
    printf("*");
    frontend_print_TypeQualifiers(Pointer->TypeQualifiers);
    frontend_print_Pointer(Pointer->Pointer);
}

void frontend_print_TypeQualifiers(pTypeQualifiersNode TypeQualifiers) {
    if(!TypeQualifiers)
        return;
    frontend_print_TypeQualifiers(TypeQualifiers->TypeQualifiers);
    frontend_print_TypeQualifier(TypeQualifiers->TypeQualifier);
}

void frontend_print_DirectDeclarator(pDirectDeclaratorNode DirectDeclarator) {
    switch (DirectDeclarator->op) {
    case tRecurr:
        printf("(");
        frontend_print_Declarator(DirectDeclarator->select.Declarator);
        printf(")");
        break;
    case tIDJust:
        frontend_print_ID(DirectDeclarator->select.ID);
        break;
    case tArrDec:
        frontend_print_DirectDeclarator(DirectDeclarator->select.DirectDeclarator);
        printf("[");
        if (DirectDeclarator->AssignExp) {
            frontend_print_AssignExp(DirectDeclarator->AssignExp);
        }
        printf("]");
        break;
    case tFunDec:
        frontend_print_DirectDeclarator(DirectDeclarator->select.DirectDeclarator);
        printf("(");
        if (DirectDeclarator->Parameters) {
            frontend_print_Parameters(DirectDeclarator->Parameters);
        }
        printf(")");
        break;
    }
}

void frontend_print_Parameters(pParametersNode Parameters) {
    if (Parameters->ParameterList)
        frontend_print_ParameterList(Parameters->ParameterList);
}

void frontend_print_ParameterList(pParameterListNode ParameterList) {
    if (ParameterList->ParameterList) {
        frontend_print_ParameterList(ParameterList->ParameterList);
        printf(", ");
    }
    frontend_print_ParameterDeclaration(ParameterList->ParameterDeclaration);
}

void frontend_print_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration) {
    frontend_print_DeclarationSpecifiers(ParameterDeclaration->DeclarationSpecifiers);
    frontend_print_Declarator(ParameterDeclaration->Declarator);
}

void frontend_print_Initializer(pInitializerNode Initializer) {
    if (Initializer->isList) {
        printf("{");
        if (Initializer->select.InitializerList)
            frontend_print_InitializerList(Initializer->select.InitializerList);
        printf("}");
    }
    else {
        frontend_print_AssignExp(Initializer->select.AssignExp);
    }
}

void frontend_print_InitializerList(pInitializerListNode InitializerList) {
    if (InitializerList->InitializerList) {
        frontend_print_InitializerList(InitializerList->InitializerList);
        printf(", ");
    }
    frontend_print_Initializer(InitializerList->Initializer);
}

void frontend_print_AssignExp(pAssignExpNode AssignExp) {
    if (AssignExp->AssignExp) {
        frontend_print_UnaryExp(AssignExp->select.UnaryExp);
        printf(" = ");
        frontend_print_AssignExp(AssignExp->AssignExp);
    }
    else {
        frontend_print_LOrExp(AssignExp->select.LOrExp);
    }
}

void frontend_print_LOrExp(pLOrExpNode LOrExp) {
    if (LOrExp->LOrExp) {
        printf("(");
        frontend_print_LOrExp(LOrExp->LOrExp);
        printf("||");
        frontend_print_LAndExp(LOrExp->LAndExp);
        printf(")");
    }
    else {
        frontend_print_LAndExp(LOrExp->LAndExp);
    }
}

void frontend_print_LAndExp(pLAndExpNode LAndExp) {
    if (LAndExp->LAndExp) {
        printf("(");
        frontend_print_LAndExp(LAndExp->LAndExp);
        printf("&&");
        frontend_print_BOrExp(LAndExp->BOrExp);
        printf(")");
    }
    else {
        frontend_print_BOrExp(LAndExp->BOrExp);
    }
}

void frontend_print_BOrExp(pBOrExpNode BOrExp) {
    if (BOrExp->BOrExp) {
        printf("(");
        frontend_print_BOrExp(BOrExp->BOrExp);
        printf("|");
        frontend_print_BNorExp(BOrExp->BNorExp);
        printf(")");
    }
    else {
        frontend_print_BNorExp(BOrExp->BNorExp);
    }
}

void frontend_print_BNorExp(pBNorExpNode BNorExp) {
    if (BNorExp->BNorExp) {
        printf("(");
        frontend_print_BNorExp(BNorExp->BNorExp);
        printf("^");
        frontend_print_BAndExp(BNorExp->BAndExp);
        printf(")");
    }
    else {
        frontend_print_BAndExp(BNorExp->BAndExp);
    }
}

void frontend_print_BAndExp(pBAndExpNode BAndExp) {
    if (BAndExp->BAndExp) {
        printf("(");
        frontend_print_BAndExp(BAndExp->BAndExp);
        printf("&");
        frontend_print_EqExp(BAndExp->EqExp);
        printf(")");
    }
    else {
        frontend_print_EqExp(BAndExp->EqExp);
    }
}

void frontend_print_EqExp(pEqExpNode EqExp) {
    if (EqExp->EqExp) {
        printf("(");
        frontend_print_EqExp(EqExp->EqExp);
        switch (EqExp->op) {
        case EQ:
            printf("==");
            break;
        case NEQ:
            printf("!=");
            break;
        }
        frontend_print_RelExp(EqExp->RelExp);
        printf(")");
    }
    else {
        frontend_print_RelExp(EqExp->RelExp);
    }
}

void frontend_print_RelExp(pRelExpNode RelExp) {
    if (RelExp->RelExp) {
        printf("(");
        frontend_print_RelExp(RelExp->RelExp);
        switch (RelExp->op) {
        case '<':
            printf("<");
            break;
        case '>':
            printf(">");
            break;
        case LE:
            printf("<=");
            break;
        case GE:
            printf(">=");
            break;
        }
        frontend_print_AddExp(RelExp->AddExp);
        printf(")");
    }
    else {
        frontend_print_AddExp(RelExp->AddExp);
    }
}

void frontend_print_AddExp(pAddExpNode AddExp) {
    if (AddExp->AddExp) {
        printf("(");
        frontend_print_AddExp(AddExp->AddExp);
        printf("%c", AddExp->op);
        frontend_print_MulExp(AddExp->MulExp);
        printf(")");
    }
    else {
        frontend_print_MulExp(AddExp->MulExp);
    }
}

void frontend_print_MulExp(pMulExpNode MulExp) {
    if (MulExp->MulExp) {
        printf("(");
        frontend_print_MulExp(MulExp->MulExp);
        printf("%c", MulExp->op);
        frontend_print_UnaryExp(MulExp->UnaryExp);
        printf(")");
    }
    else {
        frontend_print_UnaryExp(MulExp->UnaryExp);
    }
}

void frontend_print_UnaryExp(pUnaryExpNode UnaryExp) {
    if (UnaryExp->op != YYEMPTY) {
        printf("(");
    }
    switch (UnaryExp->op) {
    case '-':
    case '+':
    case '!':
    case '~':
    case '*':
    case '&':
        printf("%c", UnaryExp->op);
        frontend_print_UnaryExp(UnaryExp->select.UnaryExp);
        break;
    case SELFADD:
        printf("++");
        frontend_print_UnaryExp(UnaryExp->select.UnaryExp);
        break;
    case SELFSUB:
        printf("--");
        frontend_print_UnaryExp(UnaryExp->select.UnaryExp);
        break;
    case YYEMPTY:
        frontend_print_PostfixExp(UnaryExp->select.PostfixExp);
        break;
    }
    if (UnaryExp->op != YYEMPTY) {
        printf(")");
    }
}

void frontend_print_PostfixExp(pPostfixExpNode PostfixExp) {
    if (PostfixExp->op == SELFADD || PostfixExp->op == SELFSUB) {
        printf("(");
    }
    switch (PostfixExp->op) {
    case '[':
        frontend_print_PostfixExp(PostfixExp->select.PostfixExp);
        printf("[");
        frontend_print_Exp(PostfixExp->suffix.Exp);
        printf("]");
        break;
    case '(':
        frontend_print_PostfixExp(PostfixExp->select.PostfixExp);
        printf("(");
        frontend_print_FuncRParams(PostfixExp->suffix.FuncRParams);
        printf(")");
        break;
    case SELFADD:
        frontend_print_PostfixExp(PostfixExp->select.PostfixExp);
        printf("++");
        break;
    case SELFSUB:
        frontend_print_PostfixExp(PostfixExp->select.PostfixExp);
        printf("--");
        break;
    case YYEMPTY:
        frontend_print_PrimaryExp(PostfixExp->select.PrimaryExp);
        break;
    }
    if (PostfixExp->op == SELFADD || PostfixExp->op == SELFSUB) {
        printf(")");
    }
}

void frontend_print_PrimaryExp(pPrimaryExpNode PrimaryExp) {
    switch (PrimaryExp->type) {
    case '(':
        frontend_print_Exp(PrimaryExp->select.Exp);
        break;
    case INTCONST:
        frontend_print_Number(PrimaryExp->select.Number);
        break;
    case ID:
        frontend_print_ID(PrimaryExp->select.ID);
        break;
    }
}

void frontend_print_Exp(pExpNode Exp) {
    if (Exp->Exp) {
        printf("(");
        frontend_print_Exp(Exp->Exp);
        printf(", ");
        frontend_print_AssignExp(Exp->AssignExp);
        printf(")");
    }
    else {
        frontend_print_AssignExp(Exp->AssignExp);
    }
}

void frontend_print_Number(pNumberNode Number) {
    switch (Number->type) {
    case INTCONST:
        frontend_print_INTCONST(Number->select.INTCONST);
        break;
    }
}

void frontend_print_FuncRParams(pFuncRParamsNode FuncRParams) {
    if (FuncRParams->FuncRParamList)
        frontend_print_FuncRParamList(FuncRParams->FuncRParamList);
}

void frontend_print_FuncRParamList(pFuncRParamListNode FuncRParamList) {
    if (FuncRParamList->FuncRParamList) {
        frontend_print_FuncRParamList(FuncRParamList->FuncRParamList);
        printf(", ");
    }
    frontend_print_FuncRParam(FuncRParamList->FuncRParam);
}

void frontend_print_FuncRParam(pFuncRParamNode FuncRParam) {
    frontend_print_AssignExp(FuncRParam->AssignExp);
}

void frontend_print_Block(pBlockNode Block, int depth) {
    for (int i = 0; i < depth - 1; ++i) printf("    ");
    printf("{\n");
    frontend_print_BlockItems(Block->BlockItems, depth);
    for (int i = 0; i < depth - 1; ++i) printf("    ");
    printf("}\n");
}

void frontend_print_BlockItems(pBlockItemsNode BlockItems, int depth) {
    if (!BlockItems)
        return;
    frontend_print_BlockItems(BlockItems->BlockItems, depth);
    if (BlockItems->isStmt) {
        frontend_print_Stmt(BlockItems->select.Stmt, depth);
    }
    else {
        frontend_print_Declaration(BlockItems->select.Declaration, depth);
    }
}

void frontend_print_Stmt(pStmtNode Stmt, int depth) {
    if (Stmt->isMatched) {
        frontend_print_IfMatchedStmt(Stmt->select.IfMatchedStmt, depth);
    }
    else {
        frontend_print_IfUnMatchedStmt(Stmt->select.IfUnMatchedStmt, depth);
    }
}

void frontend_print_IfMatchedStmt(pIfMatchedStmtNode IfMatchedStmt, int depth) {
    if (IfMatchedStmt->type != '{')
        for (int i = 0; i < depth; ++i) printf("    ");
    switch (IfMatchedStmt->type) {
    case '{':
        frontend_print_Block(IfMatchedStmt->select.Block, depth);
        break;
    case ';':
        if (IfMatchedStmt->select.Exp)
            frontend_print_Exp(IfMatchedStmt->select.Exp);
        printf(";\n");
        break;
    case RETURN:
        printf("return");
        if (IfMatchedStmt->select.Exp){
            printf(" ");
            frontend_print_Exp(IfMatchedStmt->select.Exp);
        }
        printf(";\n");
        break;
    case BREAK:
        printf("break;\n");
        break;
    case CONTINUE:
        printf("continue;\n");
        break;
    case ELSE:
        printf("if (");
        frontend_print_Exp(IfMatchedStmt->select.Exp);
        printf(")\n");
        frontend_print_IfMatchedStmt(IfMatchedStmt->IfMatchedStmt_1, depth + 1);
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("else\n");
        frontend_print_IfMatchedStmt(IfMatchedStmt->IfMatchedStmt_2, depth + 1);
        break;
    case WHILE:
        printf("while (");
        frontend_print_Exp(IfMatchedStmt->select.Exp);
        printf(")\n");
        frontend_print_IfMatchedStmt(IfMatchedStmt->IfMatchedStmt_1, depth + 1);
        break;
    }
}

void frontend_print_IfUnMatchedStmt(pIfUnMatchedStmtNode IfUnMatchedStmt, int depth) {
    for (int i = 0; i < depth; ++i) printf("    ");
    switch (IfUnMatchedStmt->type) {
    case IF:
        printf("if (");
        frontend_print_Exp(IfUnMatchedStmt->Exp);
        printf(")\n");
        frontend_print_Stmt(IfUnMatchedStmt->select.Stmt, depth + 1);
        break;
    case ELSE:
        printf("if (");
        frontend_print_Exp(IfUnMatchedStmt->Exp);
        printf(")\n");
        frontend_print_IfMatchedStmt(IfUnMatchedStmt->select.IfMatchedStmt, depth + 1);
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("else\n");
        frontend_print_IfUnMatchedStmt(IfUnMatchedStmt->IfUnMatchedStmt, depth + 1);
        break;
    case WHILE:
        printf("while (");
        frontend_print_Exp(IfUnMatchedStmt->Exp);
        printf(")\n");
        frontend_print_IfUnMatchedStmt(IfUnMatchedStmt->select.IfUnMatchedStmt, depth);
        break;
    }
}

void frontend_print_ID(pIDNode ID) {
    printf("%s", ID->str);
}

void frontend_print_INTCONST(pINTCONSTNode INTCONST) {
    switch (INTCONST->type) {
    case type_int:
        printf("%d", INTCONST->val.val_int);
        break;
    case type_unsigned_int:
        printf("%u", INTCONST->val.val_unsigned_int);
        break;
    case type_long_int:
        printf("%ld", INTCONST->val.val_long_int);
        break;
    case type_unsigned_long_int:
        printf("%lu", INTCONST->val.val_unsigned_long_int);
        break;
    case type_long_long_int:
        printf("%lld", INTCONST->val.val_long_long_int);
        break;
    case type_unsigned_long_long_int:
        printf("%llu", INTCONST->val.val_unsigned_long_long_int);
        break;
    }
}
