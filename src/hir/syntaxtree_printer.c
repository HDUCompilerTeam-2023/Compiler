#include <hir.h>
#include <hir/syntaxtree_printer.h>

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
void frontend_print_ParameterList(pParameterListNode ParameterList);
void frontend_print_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration);
void frontend_print_Initializer(pInitializerNode Initializer);
void frontend_print_InitializerList(pInitializerListNode InitializerList);
void frontend_print_Expression(pExpressionNode Expression);
void frontend_print_FuncRParamList(pFuncRParamListNode FuncRParamList);
void frontend_print_BlockItems(pBlockItemsNode BlockItems, int depth);
void frontend_print_Stmt(pStmtNode Stmt, int depth);
void frontend_print_ID(pIDNode ID);
void frontend_print_CONSTNUM(pCONSTNUMNode CONSTNUM);

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
    if (Declaration->is_func_def) {
        frontend_print_DeclarationSpecifiers(Declaration->DeclarationSpecifiers);
        printf(" ");
        frontend_print_Declarator(Declaration->declarators.Declarator);
        printf("\n");
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("{\n");
        frontend_print_BlockItems(Declaration->BlockItems, depth + 1);
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("}\n");
    }
    else {
        frontend_print_InitDeclaratorList(Declaration->DeclarationSpecifiers, Declaration->declarators.InitDeclaratorList, depth);
    }
}

void frontend_print_DeclarationSpecifiers(pDeclarationSpecifiersNode DeclarationSpecifiers) {
    if (DeclarationSpecifiers->DeclarationSpecifiers) {
        frontend_print_DeclarationSpecifiers(DeclarationSpecifiers->DeclarationSpecifiers);
        printf(" ");
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
    case spec_VOID:
        printf("void");
        break;
    case spec_UNSIGNED:
        printf("unsigned");
        break;
    case spec_SIGNED:
        printf("signed");
        break;
    case spec_LONG:
        printf("long");
        break;
    case spec_SHORT:
        printf("short");
        break;
    case spec_INT:
        printf("int");
        break;
    case spec_FLOAT:
        printf("float");
        break;
    case spec_DOUBLE:
        printf("double");
        break;
    case spec_CHAR:
        printf("char");
        break;
    }
}

void frontend_print_TypeQualifier(pTypeQualifierNode TypeQualifier) {
    switch (TypeQualifier->type) {
    case qual_CONST:
        printf("const");
        break;
    case qual_VOLATILE:
        printf("volatile");
        break;
    }
}

void frontend_print_InitDeclaratorList(pDeclarationSpecifiersNode DeclarationSpecifiers, pInitDeclaratorListNode InitDeclaratorList, int depth) {
    if (InitDeclaratorList->InitDeclaratorList) {
        frontend_print_InitDeclaratorList(DeclarationSpecifiers, InitDeclaratorList->InitDeclaratorList, depth);
    }
    for (int i = 0; i < depth; ++i) printf("    ");
    frontend_print_DeclarationSpecifiers(DeclarationSpecifiers);
    printf(" ");
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
    switch (Declarator->type) {
    case tPointer:
        if (Declarator->select.Pointer) {
            frontend_print_Pointer(Declarator->select.Pointer);
        }
        frontend_print_Declarator(Declarator->body.Declarator);
        break;
    case tArrDec:
        if (Declarator->body.Declarator->type == tPointer) printf("(");
        frontend_print_Declarator(Declarator->body.Declarator);
        if (Declarator->body.Declarator->type == tPointer) printf(")");
        printf("[");
        if (Declarator->select.Expression) {
            frontend_print_Expression(Declarator->select.Expression);
        }
        printf("]");
        break;
    case tFunDec:
        if (Declarator->body.Declarator->type == tPointer) printf("(");
        frontend_print_Declarator(Declarator->body.Declarator);
        if (Declarator->body.Declarator->type == tPointer) printf(")");
        printf("(");
        if (Declarator->select.ParameterList) {
            frontend_print_ParameterList(Declarator->select.ParameterList);
        }
        printf(")");
        break;
    case tIDJust:
        frontend_print_ID(Declarator->body.ID);
        assert(!Declarator->select.IDsuffix);
        break;
    }
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
    printf(" ");
}

void frontend_print_ParameterList(pParameterListNode ParameterList) {
    if (!ParameterList) return;
    if (ParameterList->ParameterList) {
        frontend_print_ParameterList(ParameterList->ParameterList);
        printf(", ");
    }
    frontend_print_ParameterDeclaration(ParameterList->ParameterDeclaration);
}

void frontend_print_ParameterDeclaration(pParameterDeclarationNode ParameterDeclaration) {
    frontend_print_DeclarationSpecifiers(ParameterDeclaration->DeclarationSpecifiers);
    if (ParameterDeclaration->Declarator) {
        printf(" ");
        frontend_print_Declarator(ParameterDeclaration->Declarator);
    }
}

void frontend_print_Initializer(pInitializerNode Initializer) {
    if (Initializer->isList) {
        printf("{");
        if (Initializer->select.InitializerList)
            frontend_print_InitializerList(Initializer->select.InitializerList);
        printf("}");
    }
    else {
        frontend_print_Expression(Initializer->select.Expression);
    }
}

void frontend_print_InitializerList(pInitializerListNode InitializerList) {
    if (InitializerList->InitializerList) {
        frontend_print_InitializerList(InitializerList->InitializerList);
        printf(", ");
    }
    frontend_print_Initializer(InitializerList->Initializer);
}

void frontend_print_Expression(pExpressionNode Expression) {
    assert(Expression);
    switch (Expression->type) {
    case comma:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf(", ");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case assign:
        frontend_print_Expression(Expression->select.exp.first);
        printf(" = ");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        break;
    case logic_or:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("||");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case logic_and:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("&&");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case bit_or:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("|");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case bit_nor:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("^");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case bit_and:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("&");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case eq:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("==");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case neq:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("!=");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case great:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf(">");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case less:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("<");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case great_eq:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf(">=");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case less_eq:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("<=");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case binary_add:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("+");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case binary_sub:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("-");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case binary_mul:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("*");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case binary_div:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("/");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case binary_mod:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("%%");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf(")");
        break;
    case positive:
        printf("(");
        printf("+");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case negative:
        printf("(");
        printf("-");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case logic_not:
        printf("(");
        printf("!");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case bit_not:
        printf("(");
        printf("~");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case deref:
        printf("(");
        printf("*");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case ref:
        printf("(");
        printf("&");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case selfadd_pre:
        printf("(");
        printf("++");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case selfsub_pre:
        printf("(");
        printf("--");
        frontend_print_Expression(Expression->select.exp.first);
        printf(")");
        break;
    case arrary_index:
        frontend_print_Expression(Expression->select.exp.first);
        printf("[");
        frontend_print_Expression(Expression->select.exp.second.Expression);
        printf("]");
        break;
    case func_call:
        frontend_print_Expression(Expression->select.exp.first);
        printf("(");
        frontend_print_FuncRParamList(Expression->select.exp.second.FuncRParamList);
        printf(")");
        break;
    case selfadd:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("++");
        printf(")");
        break;
    case selfsub:
        printf("(");
        frontend_print_Expression(Expression->select.exp.first);
        printf("--");
        printf(")");
        break;
    case type_ID:
        frontend_print_ID(Expression->select.ID);
        break;
    case type_CONSTNUM:
        frontend_print_CONSTNUM(Expression->select.CONSTNUM);
        break;
    }
}

void frontend_print_FuncRParamList(pFuncRParamListNode FuncRParamList) {
    if (!FuncRParamList) return;
    if (FuncRParamList->FuncRParamList) {
        frontend_print_FuncRParamList(FuncRParamList->FuncRParamList);
        printf(", ");
    }
    frontend_print_Expression(FuncRParamList->Expression);
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
    switch (Stmt->type) {
    case tBlockStmt:
        for (int i = 0; i < depth - 1; ++i) printf("    ");
        printf("{\n");
        frontend_print_BlockItems(Stmt->select.BlockItems, depth);
        for (int i = 0; i < depth - 1; ++i) printf("    ");
        printf("}\n");
        break;
    case tExpStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        if(Stmt->select.Expression)
            frontend_print_Expression(Stmt->select.Expression);
        printf(";\n");
        break;
    case tReturnStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("return");
        if (Stmt->select.Expression) {
            printf(" ");
            frontend_print_Expression(Stmt->select.Expression);
        }
        printf(";\n");
        break;
    case tIfStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("if (");
        frontend_print_Expression(Stmt->select.Expression);
        printf(")\n");
        frontend_print_Stmt(Stmt->Stmt_1, depth + 1);
        if (Stmt->Stmt_2) {
            for (int i = 0; i < depth; ++i) printf("    ");
            printf("else\n");
            frontend_print_Stmt(Stmt->Stmt_2, depth + 1);
        }
        break;
    case tDoWhileStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("do\n");
        frontend_print_Stmt(Stmt->Stmt_1, depth + 1);
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("while (");
        frontend_print_Expression(Stmt->select.Expression);
        printf(");\n");
        break;
    case tWhileStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("while (");
        frontend_print_Expression(Stmt->select.Expression);
        printf(")\n");
        frontend_print_Stmt(Stmt->Stmt_1, depth + 1);
        break;
    case tBreakStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("break;\n");
        break;
    case tContinueStmt:
        for (int i = 0; i < depth; ++i) printf("    ");
        printf("continue;\n");
        break;
    }
}

void frontend_print_ID(pIDNode ID) {
    printf("%s", ID);
}

void frontend_print_CONSTNUM(pCONSTNUMNode CONSTNUM) {
    switch (CONSTNUM->valtype) {
    case type_int:
        printf("%d", CONSTNUM->val.val_int);
        break;
    case type_unsigned_int:
        printf("%u", CONSTNUM->val.val_unsigned_int);
        break;
    case type_long_int:
        printf("%ld", CONSTNUM->val.val_long_int);
        break;
    case type_unsigned_long_int:
        printf("%lu", CONSTNUM->val.val_unsigned_long_int);
        break;
    case type_long_long_int:
        printf("%lld", CONSTNUM->val.val_long_long_int);
        break;
    case type_unsigned_long_long_int:
        printf("%llu", CONSTNUM->val.val_unsigned_long_long_int);
        break;
    default:
        break;
    }
}
