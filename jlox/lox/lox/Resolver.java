package lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void> {
    private enum FunctionType {
        NONE, FUNCTION
    }

    private enum LoopType {
        NONE, WHILE
    }

    private class ScopeData {
        Token name;
        Boolean defined = false;
        Boolean referenced = false;

        ScopeData(Token name) {
            this.name = name;
        }

        public String toString() {
            return "defined: " + defined + " " + "referenced: " + referenced;
        }
    }

    private final Interpreter interpreter;
    private final Stack<Map<String, ScopeData>> scopes = new Stack<>();
    private FunctionType currentFunction = FunctionType.NONE;
    private LoopType currentLoop = LoopType.NONE;

    Resolver(Interpreter interpreter) {
        this.interpreter = interpreter;
    }

    void resolve(List<Stmt> statements) {
        for (Stmt statement : statements) {
            resolve(statement);
        }
    }

    void resolve(Expr expr) {
        expr.accept(this);
    }

    @Override
    public Void visitAssignExpr(Expr.Assign expr) {
        resolve(expr.value);
        resolveLocal(expr, expr.name);
        return null;
    }

    @Override
    public Void visitBinaryExpr(Expr.Binary expr) {
        resolve(expr.left);
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitCallExpr(Expr.Call expr) {
        resolve(expr.callee);
        for (Expr argument : expr.arguments) {
            resolve(argument);
        }
        return null;
    }

    @Override
    public Void visitLambdaExpr(Expr.Lambda expr) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = FunctionType.FUNCTION;

        beginScope();
        for (Token param : expr.params) {
            declare(param);
            define(param);
        }
        resolve(expr.body);
        endScope();

        currentFunction = enclosingFunction;
        return null;
    }

    @Override
    public Void visitGroupingExpr(Expr.Grouping expr) {
        resolve(expr.expression);
        return null;
    }

    @Override
    public Void visitLiteralExpr(Expr.Literal expr) {
        return null;
    }

    @Override
    public Void visitLogicalExpr(Expr.Logical expr) {
        resolve(expr.left);
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitUnaryExpr(Expr.Unary expr) {
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitTernaryExpr(Expr.Ternary expr) {
        resolve(expr.expression);
        resolve(expr.left);
        resolve(expr.right);
        return null;
    }

    @Override
    public Void visitVariableExpr(Expr.Variable expr) {
        ScopeData data = scopes.peek().get(expr.name.lexeme);
        if (!scopes.isEmpty() && data != null) {
            // System.out.println(expr.name.lexeme + " " + expr.name.line + " " + data);
            if (data.defined == Boolean.FALSE) {
                Lox.error(expr.name,
                        "Can't read local variable in its own initializer.");
            }
        }

        resolveLocal(expr, expr.name);
        return null;
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        beginScope();
        resolve(stmt.statements);
        endScope();
        return null;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        resolve(stmt.expression);
        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt) {
        declare(stmt.name);
        define(stmt.name);
        resolve(stmt.lambda);
        return null;
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        declare(stmt.name);
        if (stmt.initializer != null) {
            resolve(stmt.initializer);
        }
        define(stmt.name);
        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        LoopType enclosingLoop = currentLoop;
        currentLoop = LoopType.WHILE;

        resolve(stmt.condition);
        resolve(stmt.body);

        currentLoop = enclosingLoop;
        return null;
    }


    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        resolve(stmt.condition);
        resolve(stmt.thenBranch);
        if (stmt.elseBranch != null) resolve(stmt.elseBranch);
        return null;
    }

    @Override
    public Void visitLoopControlStmt(Stmt.LoopControl stmt) {
        if (currentLoop == LoopType.NONE) {
            Lox.error(stmt.keyword,
                    "Can't execute a loop control statement outsie of a loop.");
        }
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        if (currentFunction == FunctionType.NONE) {
            Lox.error(stmt.keyword, "Can't return from top-level code.");
        }

        if (stmt.value != null) {
            resolve(stmt.value);
        }
        return null;
    }

    @Override
    public Void visitPrintSexprStmt(Stmt.PrintSexpr stmt) {
        resolve(stmt.expression);
        return null;
    }

    private void resolve(Stmt stmt) {
        stmt.accept(this);
    }

    // private void resolveFunction() { // TODO might be able to remove?
    // private void resolveLambda(Expr.Lambda lambda) {
    //     beginScope();
    //     for (Token param : lambda.params) {
    //         declare(param);
    //         define(param);
    //     }
    //     resolve(lambda.body);
    //     endScope();
    // }

    private void beginScope() {
        scopes.push(new HashMap<String, ScopeData>());
    }

    private void endScope() {
        Map<String, ScopeData> scope = scopes.pop();

        scope.forEach((k, v) -> {
            if (!v.referenced) {
                Lox.warning(v.name, "Variable is unused.");
            }
        });
    }

    private void declare(Token name) {
        if (scopes.isEmpty()) return;
        Map<String, ScopeData> scope = scopes.peek();
        if (scope.containsKey(name.lexeme)) {
            Lox.error(name,
                    "Already a variable with this name in this scope.");
        }
        scope.put(name.lexeme, new ScopeData(name));
    }

    private void define(Token name) {
        if (scopes.isEmpty()) return;
        scopes.peek().get(name.lexeme).defined = true;
    }

    private void resolveLocal(Expr expr, Token name) {
        int scopesSize = scopes.size();

        for (int i = scopesSize - 1; i >= 0; i--) {
            ScopeData data = scopes.get(i).get(name.lexeme);
            if (data != null) {
                data.referenced = true;
                interpreter.resolve(expr, scopesSize - 1 - i);
                return;
            }
        }
    }
}
