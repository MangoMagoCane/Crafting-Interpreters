package lox;

class AstPrinter implements Expr.Visitor<String> {
    String print(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        return parenthesize("=", new Expr.Literal(expr.name.lexeme), expr.value);
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right);
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return parenthesize("group", expr.expression);
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitLogicalExpr(Expr.Logical expr) {
        return parenthesize(expr.operator.lexeme, expr.left, expr.right);
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return parenthesize(expr.operator.lexeme, expr.right);
    }

    @Override
    public String visitTernaryExpr(Expr.Ternary expr) {
        return parenthesize(expr.expression, new Expr.Literal("?"), expr.left, new Expr.Literal(":"), expr.right);

    }
    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        return expr.name.lexeme;
    }


    private String parenthesize(Expr... exprs) {
        return parenthesize("", exprs);
    }

    private String parenthesize(String name, Expr... exprs) {
        boolean appendSpace = true;
        StringBuilder builder = new StringBuilder();

        builder.append("(");
        if (name.isEmpty()) {
            appendSpace = false;
        } else {
            builder.append(name);
        }

        for (Expr expr : exprs) {
            if (appendSpace) {
                builder.append(" ");
            } else {
                appendSpace = true;
            }
            builder.append(expr.accept(this));
        }
        builder.append(")");

        return builder.toString();
    }
}
