package lox;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.List;

public class Lox {
    private static final Interpreter interpreter = new Interpreter();
    static boolean hadError = false;
    static boolean hadRuntimeError = false;
    static boolean printReport = true;

    public static void main(String[] args) throws IOException {
        if (args.length > 1) {
            System.out.println("Usage: jlox [script]");
            System.exit(64);
        } else if (args.length == 1) {
            runFile(args[0]);
        } else {
            runPrompt();
        }
    }

    private static void runFile(String path) throws IOException {
        byte[] bytes = Files.readAllBytes(Paths.get(path));
        run(new String(bytes, Charset.defaultCharset()));

        if (hadError) System.exit(65);
        if (hadRuntimeError) System.exit(70);
    }

    private static void runPrompt() throws IOException {
        InputStreamReader input = new InputStreamReader(System.in);
        BufferedReader reader = new BufferedReader(input);

        for (;;) {
            System.out.print("> ");
            String line = reader.readLine();
            if (line == null) break;
            run(line, true);
            hadError = false;
        }
    }

    private static void run(String source) {
        run(source, false);
    }

    private static void run(String source, boolean isPossibleExpression) {
        Scanner scanner = new Scanner(source);
        List<Token> tokens = scanner.scanTokens();
        Parser parser = new Parser(tokens);
        Resolver resolver = new Resolver(interpreter);

        if (!isPossibleExpression) {
            List<Stmt> statements = parser.parse();
            if (hadError) return;
            resolver.resolve(statements);
            interpreter.interpret(statements);
            return;
        }

        printReport = false;
        List<Stmt> statements = parser.parse();
        printReport = true;

        if (!hadError) {
            resolver.resolve(statements);
            interpreter.interpret(statements);
            return;
        }

        parser.reset();
        Expr expression = parser.parseExpression();
        if (expression != null) {
            resolver.resolve(expression);
            interpreter.interpret(expression);
        }
        return;
    }

    static void warning(Token token, String message) {
        error(token, message, true);
    }

    static void error(int line, String message) {
        report(line, "", message, false);
    }

    static void error(Token token, String message) {
        error(token, message, false);
    }

    static void error(Token token, String message, boolean isWarning) {
        if (token.type == TokenType.EOF) {
            report(token.line, " at end", message, isWarning);
        } else {
            report(token.line, " at '" + token.lexeme + "'", message, isWarning);
        }
    }

    static void runtimeError(RuntimeError error) {
        System.err.println(error.getMessage() +
                "\n[line " + error.token.line + "]");
        hadRuntimeError = true;
    }

    private static void report(int line, String where, String message, boolean isWarning) {
        if (printReport) {
            System.err.println("[line " + line + "] " + (isWarning ? "Warning" : "Error")+ where + ": " + message);
        }
        hadError = !isWarning;
    }
}
