import java.io.IOException;
import java.io.PrintWriter;
import java.util.Arrays;
import java.util.List;

public class GenerateAst {
    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.err.println("Usage: generate_ast <output directory>");
            System.exit(64);
        }
        String outputDir = args[0];

        defineAst(outputDir, "Expr", Arrays.asList(
            "Assign     : Token name, Expr value",
            // "AssignTern : Expr.Ternary ternary, Expr value",
            "Binary     : Expr left, Token operator, Expr right",
            "Call       : Expr callee, Token paren, List<Expr> arguments",
            "Lambda     : List<Token> params, List<Stmt> body",
            "Grouping   : Expr expression",
            "Literal    : Object value",
            "Logical    : Expr left, Token operator, Expr right",
            "Unary      : Token operator, Expr right",
            "Ternary    : Expr expression, Expr left, Expr right",
            "Variable   : Token name"
        ));

        defineAst(outputDir, "Stmt", Arrays.asList(
            "Block       : List<Stmt> statements",
            "Expression  : Expr expression",
            "Function    : Token name, Expr.Lambda lambda",
            "Var         : Token name, Expr initializer",
            "While       : Expr condition, Stmt body",
            "If          : Expr condition, Stmt thenBranch," +
                         " Stmt elseBranch",
            "LoopControl : Token keyword",
            "Return      : Token keyword, Expr value",
            "PrintSexpr  : Expr expression"
            // "Print       : Expr expression",
        ));
    }

    private static void defineAst(
            String outputDir, String baseName, List<String> types)
            throws IOException {
        String path = outputDir + "/" + baseName + ".java";
        PrintWriter writer = new PrintWriter(path, "UTF-8");

        writer.println("package lox;");
        writer.println();
        writer.println("import java.util.List;");
        writer.println();
        writer.println("abstract class " + baseName + " {");

        defineVisitor(writer, baseName, types);
        writer.println(indent(1) + "abstract <R> R accept(Visitor<R> visitor);");
        writer.println();

        for (String type : types) {
            String className = type.split(":")[0].trim();
            String fields = type.split(":")[1].trim();
            defineType(writer, baseName, className, fields);
        }


        writer.println("}");
        writer.close();
    }

    private static void defineVisitor(
            PrintWriter writer, String baseName,
            List<String> types) {
        writer.println(indent(1) + "interface Visitor<R> {");

        for (String type : types) {
            String typeName = type.split(":")[0].trim();
            writer.println(indent(2) + "R visit" + typeName + baseName + "(" + 
                    typeName + " " + baseName.toLowerCase() + ");");
        }

        writer.println(indent(1) + "}");
        writer.println();
    } 

    private static void defineType(
            PrintWriter writer, String baseName,
            String className, String fieldList) {
        writer.println(indent(1) + "static class " + className + " extends " + baseName + " {");

        String[] fields = fieldList.split(", ");

        for (String field : fields) {
            writer.println(indent(2) + "final " + field + ";");
        }
        writer.println();

        writer.println(indent(2) + className + "(" + fieldList + ") {");
        for (String field : fields) {
            String name = field.split(" ")[1];
            writer.println(indent(3) + "this." + name + " = " + name + ";");
        }

        writer.println(indent(2) + "}");

        writer.println();
        writer.println(indent(2) + "@Override");
        writer.println(indent(2) + "<R> R accept(Visitor<R> visitor) {");
        writer.println(indent(3) + "return visitor.visit" + 
                className + baseName + "(this);");
        writer.println(indent(2) + "}");

        writer.println(indent(1) + "}");
        writer.println();
    }

    private static String indent(int amount) {
        String indentChars = "    ";
        if (amount < 1) {
            return "";
        } 
        return indentChars.repeat(amount);
    }
}
