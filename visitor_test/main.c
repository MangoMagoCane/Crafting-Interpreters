#include <stddef.h>
#include <stdlib.h>

#include "visitor.h"

#include "sexprVisitor.c"
// #include "printVisitor.c"

Expr *makeLiteral(char value) {
    ExprLiteral *expr = malloc(sizeof (ExprLiteral));
    expr->super.type = EXPR_LITERAL;
    expr->value = value;
    return (Expr *)expr;
}

Expr *makeUnary(Expr *op, Expr *right) {
    ExprUnary *expr = malloc(sizeof (ExprUnary));
    expr->super.type = EXPR_UNARY;
    expr->op = op;
    expr->right = right;
    return (Expr *)expr;
}

Expr *makeBinary(Expr *left, Expr *op, Expr *right) {
    ExprBinary*expr = malloc(sizeof (ExprBinary));
    expr->super.type = EXPR_BINARY;
    expr->left = left;
    expr->op = op;
    expr->right = right;
    return (Expr *)expr;
}

int main() {
    // 2 * -3 + 4
    // (+ (* 2 (- 3)) 4)
    Expr *example = makeBinary(
        makeBinary(
            makeLiteral('2'),
            makeLiteral('*'),
            makeUnary(
                makeLiteral('-'),
                makeLiteral('3')
            )
        ),
        makeLiteral('+'),
        makeLiteral('4')
    );

    accept(example, &printVisitor);
    printf("\n");
}

