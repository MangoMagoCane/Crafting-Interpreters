#ifndef VISITOR_H
#define VISITOR_H

#include <stdio.h>
typedef enum {
    EXPR_LITERAL, EXPR_UNARY, EXPR_BINARY
} ExprType;

typedef struct {
    ExprType type;
} Expr;

typedef struct {
    Expr super;
    char value;
} ExprLiteral;

typedef struct {
    Expr super;
    Expr *op;
    Expr *right;
} ExprUnary;

typedef struct {
    Expr super;
    Expr *left;
    Expr *op;
    Expr *right;
} ExprBinary;

typedef struct {
    void *(*visitLiteral)(ExprLiteral *expr);
    void *(*visitUnary)(ExprUnary *expr);
    void *(*visitBinary)(ExprBinary *expr);
} Visitor;

static inline void accept(Expr *expr, Visitor *visitor) {
    switch (expr->type) {
    case EXPR_LITERAL: visitor->visitLiteral((ExprLiteral *)expr); break;
    case EXPR_UNARY: visitor->visitUnary((ExprUnary *)expr); break;
    case EXPR_BINARY: visitor->visitBinary((ExprBinary *)expr); break;
    }
}

#endif // VISITOR_H
