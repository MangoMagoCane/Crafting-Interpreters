#include <stdio.h>

#include "visitor.h"

void *printLiteral(ExprLiteral *expr);
void *printBinary(ExprBinary *expr);
void *printUnary(ExprUnary *expr);

Visitor printVisitor = {
    *printLiteral,
    *printUnary,
    *printBinary
};

void *printLiteral(ExprLiteral *expr) {
    printf("%c", expr->value);
    return NULL;
}

void *printUnary(ExprUnary *expr) {
    accept(expr->op, &printVisitor);
    accept(expr->right, &printVisitor);
    return NULL;
}

void *printBinary(ExprBinary *expr) {
    accept(expr->left, &printVisitor);
    printf(" ");
    accept(expr->op, &printVisitor);
    printf(" ");
    accept(expr->right, &printVisitor);
    return NULL;
}
