package lox;

class RpnPrinter implements Expr.Visitor<String> {
    String print(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return polishize(expr.operator.lexeme, expr.left, expr.right);
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return polishize("group", expr.expression);
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.value == null) return "nil";
        return expr.value.toString();
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return polishize(expr.operator.lexeme, expr.right);
    }

    private String polishize(String name, Expr... exprs) {
        StringBuilder builder = new StringBuilder();

        for (Expr expr : exprs) {
            builder.append(expr.accept(this)).append(" ");
        }
        builder.append(name);

        return builder.toString();
    }
}
