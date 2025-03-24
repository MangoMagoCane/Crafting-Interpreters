package lox;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static lox.TokenType.*;

class Parser {
    private static class ParseError extends RuntimeException {}

    private final List<Token> tokens;
    private int current = 0;

    Parser(List<Token> tokens) {
        this.tokens = tokens;
    }

    public void reset() {
        rewind(0);
    }

    public List<Stmt> parse() {
        List<Stmt> statements = new ArrayList<>();
        while (!isAtEnd()) {
            statements.add(declaration());
        }

        return statements;
    }

    public Expr parseExpression() {
        try {
            Expr value = expression();
            return isAtEnd() ? value : null;
        } catch (ParseError error) {
            return null;
        }
    }

    private Stmt declaration() {
        try {
            if (match(FUN)) return function("function");
            if (match(VAR)) return varDeclaration();
            return statement();
        } catch (ParseError error) {
            synchronize();
            return null;
        }
    }

    private Stmt statement() {
        if (match(RETURN)) return returnStatement();
        if (match(FOR)) return forStatement();
        if (match(WHILE)) return whileStatement();
        if (match(IF)) return ifStatement();
        // if (match(PRINT)) return printStatement();
        if (match(PRINT_SEXPR)) return printSexprStatement();
        if (match(LEFT_BRACE)) return new Stmt.Block(block());

        return expressionStatement();
    }

    private Stmt returnStatement() {
        Token keyword = previous();
        Expr value = null;

        if (!check(SEMICOLON)) {
            value = expression();
        }
        consume(SEMICOLON, "Exect ';' after return value.");

        return new Stmt.Return(keyword, value);
    }

    private Stmt forStatement() {
        consume(LEFT_PAREN, "Expect '(' after 'for'.");

        Stmt initializer;
        if (match(SEMICOLON)) {
            initializer = null;
        } else if (match(VAR)) {
            initializer = varDeclaration();
        } else {
            initializer = expressionStatement();
        }

        Expr condition = null;
        if (!check(SEMICOLON)) {
            condition = expression();
        }
        consume(SEMICOLON, "Expect ';' after loop condition.");

        Expr increment = null;
        if (!check(RIGHT_PAREN)) {
            increment = expression();
        }
        consume(RIGHT_PAREN, "Expect ')' after for clauses.");

        Stmt body = statement();

        if (increment != null) {
            body = new Stmt.Block(
                    Arrays.asList(
                            body,
                            new Stmt.Expression(increment)));
        }

        if (condition == null) condition = new Expr.Literal(true);
        body = new Stmt.While(condition, body);

        if (initializer != null) {
            body = new Stmt.Block(Arrays.asList(initializer, body));
        }


        return body;
    }

    private Stmt whileStatement() {
        consume(LEFT_PAREN, "Expect '(' after 'while'.");
        Expr condition = expression();
        consume(LEFT_PAREN, "Expect ')' after while condition.");
        Stmt body = statement();

        return new Stmt.While(condition, body);
    }

    private Stmt ifStatement() {
        consume(LEFT_PAREN, "Expect '(' after 'if'.");
        Expr condition = expression();
        consume(RIGHT_PAREN, "Expect ')' after if condition.");

        Stmt thenBranch = statement();
        Stmt elseBranch = null;
        if (match(ELSE)) {
            elseBranch = statement();
        }

        return new Stmt.If(condition, thenBranch, elseBranch);
    }

    // private Stmt printStatement() {
    //     Expr value = expression();
    //     consume(SEMICOLON, "Expect ';' after value.");
    //     return new Stmt.Print(value);
    // }

    private Stmt printSexprStatement() {
        Expr value = expression();
        consume(SEMICOLON, "Expect ';' after value.");
        return new Stmt.PrintSexpr(value);
    }

    private List<Stmt> block() {
        List<Stmt> statements = new ArrayList<>();

        while (!check(RIGHT_BRACE) && !isAtEnd()) {
            statements.add(declaration());
        }

        consume(RIGHT_BRACE, "Expect '}' after block.");
        return statements;
    }

    private Stmt expressionStatement() {
        Expr value = expression();
        consume(SEMICOLON, "Expect ';' after value.");
        return new Stmt.Expression(value);
    }

    private Stmt.Function function(String kind) {
        Token name = null;
        if (check(IDENTIFIER)) {
            name = advance();
        }
        consume(LEFT_PAREN, "Expect '(' after " + kind + " name.");
        List<Token> parameters = new ArrayList<>();

        if (!check(RIGHT_PAREN)) {
            do {
                if (parameters.size() >= 255) {
                    error(peek(), "Can't have more than 255 arguments.");
                }

                parameters.add(consume(IDENTIFIER, "Expect parameter name."));
            } while (match(COMMA));
        }
        consume(RIGHT_PAREN, "Expect ')' after parameters.");

        consume(LEFT_BRACE, "Expect '{' before " + kind + " body.");
        List<Stmt> body = block();

        return new Stmt.Function(new Expr.Function(name, parameters, body));
    }

    private Stmt varDeclaration() {
        Expr initializer = null;
        Token name = consume(IDENTIFIER, "Expect variable name.");

        if (match(EQUAL)) initializer = expression();
        consume(SEMICOLON, "Expect ';' after variable declaration.");

        return new Stmt.Var(name, initializer);
    }

    private Expr expression() {
        return lambda();
    }

    private Expr nonCommaExpression() {
        return lambda(false);
    }

    private Expr lambda() {
        return lambda(true);
    }

    private Expr lambda(boolean commaParseable) {
        final int tokenIndex = currentIndex();
        final List<Token> parameters = new ArrayList<>();
        TokenType delimeter = GREATER_EQUAL;

        if (match(LEFT_PAREN)) delimeter = RIGHT_PAREN;
        if (!check(delimeter)) {
            do {
                if (!check(IDENTIFIER)) {
                    rewind(tokenIndex);
                    return commaParseable ? comma() : assignment();
                }
                parameters.add(advance());
            } while (match(COMMA));
        }

        if (delimeter == RIGHT_PAREN) {
            consume(RIGHT_PAREN, "EXPECT ')' after lambda parameters.");
        } else if (parameters.size() == 0) {
            error(peek(), "EXPECT '()' or parameters.");
        }

        if (!match(EQUAL_GREATER)) {
            rewind(tokenIndex);
            return commaParseable ? comma() : assignment();
        }

        List<Stmt> body;
        if (match(LEFT_BRACE)) {
            body = block();
        } else {
            body = new ArrayList<>();
            body.add(new Stmt.Return(peek(), nonCommaExpression()));
        }

        return new Expr.Function(null, parameters, body);
    }

    private Expr comma() {
        Expr expr = assignment();

        while (match(COMMA)) {
            final Token operator = previous();
            final Expr right = assignment();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    private Expr assignment() {
        Expr expr = or();

        if (match(EQUAL)) {
            Token equals = previous();
            Expr value = assignment();

            if (expr instanceof Expr.Variable) {
                Token name = ((Expr.Variable)expr).name;
                return new Expr.Assign(name, value);
            }

            // Hold off on until Resolver.
            // if (expr instanceof Expr.Ternary) {
            //     ternExpr = ((Expr.Variable)expr);
            //     if (ternExpr.left instanceof Expr.Variable, 
            //             ternExpr.right instanceof Expr.Variable) {
            //
            //     }
            // }

            error(equals, "Invalid assignment target.");
        }

        return expr;
    }

    private Expr or() {
        Expr expr = and();

        while (match(OR)) {
            Token operator = previous();
            Expr right = and();
            expr = new Expr.Logical(expr, operator, right);
        }

        return expr;
    }

    private Expr and() {
        Expr expr = ternaryRL();

        while (match(OR)) {
            Token operator = previous();
            Expr right = ternaryRL();
            expr = new Expr.Logical(expr, operator, right);
        }

        return expr;
    }

    private Expr ternaryRL() {
        Expr expr = ternaryLR();

        if (match(EROTEME)) {
            Expr left = ternaryLR();
            consume(COLON, "EXPECT ':' after expression.");
            Expr right = ternaryRL();
            return new Expr.Ternary(expr, left, right);
        }

        return expr;
    }

    private Expr ternaryLR() {
        Expr expr = equality();

        while (match(EROTEME_DOT)) {
            Expr left = equality();
            consume(COLON, "EXPECT ':' after expression.");
            Expr right = equality();
            expr = new Expr.Ternary(expr, left, right);
        }

        return expr;
    }

    private Expr equality() {
        Expr expr = comparison();

        while (match(BANG_EQUAL, EQUAL_EQUAL)) {
            Token operator = previous();
            Expr right = comparison();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    private Expr comparison() {
        Expr expr = term();

        while (match(GREATER, GREATER_EQUAL, LESS, LESS_EQUAL)) {
            Token operator = previous();
            Expr right = term();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    private Expr term() {
        Expr expr = factor();

        while (match(MINUS, PLUS)) {
            Token operator = previous();
            Expr right = factor();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    private Expr factor() {
        Expr expr = unary();

        while (match(SLASH, STAR)) {
            Token operator = previous();
            Expr right = unary();
            expr = new Expr.Binary(expr, operator, right);
        }

        return expr;
    }

    private Expr unary() {
        if (match(BANG, MINUS)) {
            Token operator = previous();
            Expr right = unary();
            return new Expr.Unary(operator, right);
        }

        return call();
    }

    private Expr call() {
        Expr expr = primary();

        while (true) {
            if (match(LEFT_PAREN)) {
                expr = finishCall(expr);
            } else {
                break;
            }
        }

        return expr;
    }

    private Expr finishCall(Expr callee) {
        List<Expr> arguments = new ArrayList<>();

        if (!check(RIGHT_PAREN)) {
            do {
                if (arguments.size() >= 255) {
                    error(peek(), "Can't have more than 255 arguments.");
                }
                arguments.add(nonCommaExpression());
            } while (match(COMMA));
        }
        Token paren = consume(RIGHT_PAREN, "Expect ')' after arguments.");

        return new Expr.Call(callee, paren, arguments);
    }

    private Expr primary() {
        if (match(FALSE)) return new Expr.Literal(false);
        if (match(TRUE)) return new Expr.Literal(true);
        if (match(NIL)) return new Expr.Literal(null);

        if (match(NUMBER, STRING)) {
            return new Expr.Literal(previous().literal);
        }

        if (match(IDENTIFIER)) {
            return new Expr.Variable(previous());
        }

        if (match(LEFT_PAREN)) {
            Expr expr = expression();
            consume(RIGHT_PAREN, "EXPECT ')' after expression.");
            return new Expr.Grouping(expr);
        }

        throw error(peek(), "Expect expression.");
    }

    private boolean match(TokenType... types) {
        for (TokenType type : types) {
            if (check(type)) {
                advance();
                return true;
            }
        }
        return false;
    }

    private Token consume(TokenType type, String message) {
        if (check(type)) return advance();
        throw error(peek(), message);
    }

    private boolean check(TokenType type) {
        if (isAtEnd()) return false;
        return peek().type == type;
    }

    private Token advance() {
        if (!isAtEnd()) current++;
        return previous();
    }

    private boolean isAtEnd() {
        return peek().type == EOF;
    }

    private Token peek() {
        return tokens.get(current);
    }

    private Token previous() {
        return tokens.get(current - 1);
    }

    private int currentIndex() {
        return current;
    }

    private void rewind(int index) {
        current = Math.max(0, index);
    }

    private ParseError error(Token token, String message) {
        Lox.error(token, message);
        return new ParseError();
    }

    private void synchronize() {
        advance();

        while (!isAtEnd()) {
            if (previous().type == SEMICOLON) return;

            switch (peek().type) {
            case CLASS:
            case FUN:
            case VAR:
            case FOR:
            case IF:
            case WHILE:
            case PRINT:
            case RETURN:
                return;
            }

            advance();
        }
    }
}
