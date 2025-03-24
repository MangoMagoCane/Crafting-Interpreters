package lox;

import java.util.HashMap;
import java.util.Map;

class Environment {
    class Variable {
        public Object value;
        public boolean assignedTo;

        Variable(Object object) {
            this(object, false);
        }

        Variable(Object object, boolean assigned) {
            value = object;
            assignedTo = assigned;
        }
    }

    final Environment enclosing;
    private final Map<String, Variable> values = new HashMap<>();


    Environment() {
        enclosing = null;
    }

    Environment(Environment enclosing) {
        this.enclosing = enclosing;
    }

    public void define(String name, Object value) {
        define(name, value, true);
    }

    public void define(String name, Object value, boolean initialized) {
        values.put(name, new Variable(value, initialized));
    }

    public Object get(Token name) {
        if (values.containsKey(name.lexeme)) {
            Variable variable = values.get(name.lexeme);
            if (variable.assignedTo) return variable.value;
            throw new RuntimeError(name,
                    "Unassigned variable '" + name.lexeme + "' accessed.");
        }

        if (enclosing != null) return enclosing.get(name);

        throw new RuntimeError(name,
                "Undefined variable '" + name.lexeme + "'");
    }

    public void assign(Token name, Object value) {
        if (values.containsKey(name.lexeme)) {
            Variable variable = values.get(name.lexeme);
            variable.value = value;
            variable.assignedTo = true;
            return;
        }

        if (enclosing != null) {
            enclosing.assign(name, value);
            return;
        }

        throw new RuntimeError(name,
                "Undefined variable '" + name.lexeme + "'");
    }

    private void printTable() {
        System.out.println("env:");
        values.forEach((key, value) -> System.out.println("  " + key + " " + value));
    }
}
