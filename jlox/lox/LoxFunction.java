package lox;

import java.util.List;

class LoxFunction implements LoxCallable {
    private final Token name;
    private final Expr.Lambda declaration;
    private final Environment closure;

    LoxFunction(Expr.Lambda declaration, Environment closure) {
        this(declaration, closure, null);
    }

    LoxFunction(Expr.Lambda declaration, Environment closure, Token name) {
        this.declaration = declaration;
        this.closure = closure;
        this.name = name;
    }

    @Override
    public int arity() {
        return declaration.params.size();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        Environment environment = new Environment(closure);

        for (int i = 0; i < declaration.params.size(); i++) {
            environment.define(declaration.params.get(i).lexeme,
                    arguments.get(i));
        }

        try {
            interpreter.executeBlock(declaration.body, environment);
        } catch (Return returnValue) {
            return returnValue.value;
        }

        return null;
    }

    @Override
    public String toString() {
        if (name == null) {
            return "<lambda>";
        }
        return "<fn " + name.lexeme + ">";
    }
}
