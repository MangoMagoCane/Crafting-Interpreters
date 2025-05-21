#include <stdio.h>
#include <stdlib.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    Token current;
    Token previous;
    Scanner *scanner;
    Chunk *chunk;
    bool hadError;
    bool panicMode;
} Parser;

typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_CONDITIONAL, // ?
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_POWER,       // ^
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_POST,        // !
    PREC_CALL,        // . ()
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Parser *parser);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

static void initParser(Parser *parser, Scanner *scanner, Chunk *chunk) {
    parser->scanner = scanner;
    parser->chunk = chunk;
    parser->hadError = false;
    parser->panicMode = false;
}

static Chunk *currentChunk(Parser *parser) {
    return parser->chunk;
}

static void errorAt(Parser *parser, Token *token, const char *message) {
    if (parser->panicMode) return;
    parser->panicMode = true;
    fprintf(stderr, "[line %d] Error\n", token->line);

    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // Do nothing.
    } else {
        fprintf(stderr, " at '%.*s'\n", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

static void error(Parser *parser, const char *message) {
    errorAt(parser, &parser->previous, message);
}

static void errorAtCurrent(Parser *parser, const char *message) {
    errorAt(parser, &parser->current, message);
}

static void advance(Parser *parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = scanToken(parser->scanner);
        if (parser->current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser, parser->current.start);
    }
}

static void consume(Parser *parser, TokenType type, const char *message) {
    if (parser->current.type == type) {
        advance(parser);
        return;
    }

    errorAtCurrent(parser, message);
}

static void emitByte(Parser *parser, uint8_t byte) {
    writeChunk(currentChunk(parser), byte, parser->previous.line);
}

static void emitBytes(Parser *parser, uint8_t byte1, uint8_t byte2) {
    emitByte(parser, byte1);
    emitByte(parser, byte2);
}

static void emitReturn(Parser *parser) {
    emitByte(parser, OP_RETURN);
}

static int makeConstant(Parser *parser, Value value) {
    int constant = addConstant(currentChunk(parser), value);
    if (constant > UINT24_MAX) {
        error(parser, "Too many constants in one chunk.");
        return 0;
    }

    return constant;
}

static void emitConstant(Parser *parser, Value value) {
    int constant = makeConstant(parser, value);
    emitBytes(parser, OP_CONSTANT, constant);
    if (constant > UINT8_MAX) {
        emitBytes(parser, constant >> 8, constant >> 16);
    }
}

static void endCompiler(Parser *parser) {
    emitReturn(parser);
#ifdef DEBUG_PRINT_CODE
    if (!parser->hadError) {
        disassembleChunk(currentChunk(parser), "code");
    }
#endif
}

static void expression(Parser *parser);
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Parser *parser, Precedence precedence);

static void binary(Parser *parser) {
    TokenType operatorType = parser->previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence(parser, rule->precedence + 1);

    switch (operatorType) {
    case TOKEN_PLUS:  emitByte(parser, OP_ADD); break;
    case TOKEN_MINUS: emitByte(parser, OP_SUBTRACT); break;
    case TOKEN_STAR:  emitByte(parser, OP_MULTIPLY); break;
    case TOKEN_SLASH: emitByte(parser, OP_DIVIDE); break;
    default: return; // Unreachable.
    }
}

static void grouping(Parser *parser) {
    expression(parser);
    consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void expression(Parser *parser) {
    parsePrecedence(parser, PREC_ASSIGNMENT);
}

static void number(Parser *parser) {
    double value = strtod(parser->previous.start, NULL);
    emitConstant(parser, NUMBER_VAL(value));
}

static void unary(Parser *parser) {
    TokenType operatorType = parser->previous.type;
    parsePrecedence(parser, PREC_UNARY);

    switch (operatorType) {
    case TOKEN_MINUS: emitByte(parser, OP_NEGATE); break;
    default: return; // Unreachable.
    }
}

static void post(Parser *parser) {
    TokenType operatorType = parser->previous.type;

    switch (operatorType) {
    case TOKEN_BANG: emitByte(parser, OP_FACTORIAL); break;
    default: return; // Unreachable.
    }
}

static void mix(Parser *parser) {
    // TokenType operatorType = parser->previous.type;
    parsePrecedence(parser, PREC_CONDITIONAL);
    consume(parser, TOKEN_COLON, "Expect ':' in expression.");
    parsePrecedence(parser, PREC_CONDITIONAL);

}
static void binrl(Parser *parser) {
    TokenType operatorType = parser->previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence(parser, rule->precedence + 1);

    switch (operatorType) {
    case TOKEN_CARET: emitByte(parser, OP_EXPONENTIATE); break;
    default: return; // Unreachable.
    }
}

ParseRule rules[] = {
    [TOKEN_LEFT_PAREN]    = {grouping, NULL,   PREC_NONE},
    [TOKEN_RIGHT_PAREN]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LEFT_BRACE]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RIGHT_BRACE]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_COMMA]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_DOT]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_MINUS]         = {unary,    binary, PREC_TERM},
    [TOKEN_PLUS]          = {NULL,     binary, PREC_TERM},
    [TOKEN_COLON]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SEMICOLON]     = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SLASH]         = {NULL,     binary, PREC_FACTOR},
    [TOKEN_STAR]          = {NULL,     binary, PREC_FACTOR},
    [TOKEN_EROTEME]       = {NULL,     mix,    PREC_CONDITIONAL},
    [TOKEN_CARET]         = {NULL,     binrl,  PREC_POWER},
    [TOKEN_BANG]          = {NULL,     post,   PREC_POST},
    [TOKEN_BANG_EQUAL]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EQUAL_EQUAL]   = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GREATER]       = {NULL,     NULL,   PREC_NONE},
    [TOKEN_GREATER_EQUAL] = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LESS]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_LESS_EQUAL]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IDENTIFIER]    = {NULL,     NULL,   PREC_NONE},
    [TOKEN_STRING]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NUMBER]        = {number,   NULL,   PREC_NONE},
    [TOKEN_AND]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_CLASS]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ELSE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FALSE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FOR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_FUN]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_IF]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_NIL]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_OR]            = {NULL,     NULL,   PREC_NONE},
    [TOKEN_PRINT]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_RETURN]        = {NULL,     NULL,   PREC_NONE},
    [TOKEN_SUPER]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_THIS]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_TRUE]          = {NULL,     NULL,   PREC_NONE},
    [TOKEN_VAR]           = {NULL,     NULL,   PREC_NONE},
    [TOKEN_WHILE]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_ERROR]         = {NULL,     NULL,   PREC_NONE},
    [TOKEN_EOF]           = {NULL,     NULL,   PREC_NONE},
};

static void parsePrecedence(Parser *parser, Precedence precedence) {
    advance(parser);
    ParseFn prefixRule = getRule(parser->previous.type)->prefix;
    if (prefixRule == NULL) {
        error(parser, "Expect expression.");
        return;
    }

    prefixRule(parser);

    while (precedence <= getRule(parser->current.type)->precedence) {
        advance(parser);
        ParseFn infixRule = getRule(parser->previous.type)->infix;
        infixRule(parser);
    }
}

static ParseRule *getRule(TokenType type) {
    return &rules[type];
}

static int prec(char c) {
    switch (c) {
    case '+': return 1;
    case '-': return 1;
    case '*': return 2;
    case '/': return 2;
    default: return 0;
    }
}

static void expr(Parser *parser) {
    char stack[100];
    int i = 0;
    bool push_to_stack;

    for (; parser->previous.type != TOKEN_EOF; advance(parser)) {
        bool push_to_stack = true;
        if (parser->previous.type == TOKEN_NUMBER) {
            printf("%.*s ", parser->previous.length, parser->previous.start);
            continue;
        }

        char c = *parser->previous.start;

        if (i <= 0) {
            stack[i++] = c;
            continue;
        }

        while (i > 0 && prec(stack[i-1]) >= prec(c)) {
            printf("%c ", stack[--i]);
            push_to_stack = false;
        }

        stack[i++] = c;
    }

    for (int j = i-1; j >= 0; j--) {
        printf("%c ", stack[j]);
    }

    printf("\n");
}

bool compile(const char *source, Chunk *chunk) {
    Parser parser;
    Scanner scanner;

    initScanner(&scanner, source);
    initParser(&parser, &scanner, chunk);

    advance(&parser);
    expr(&parser);
    // expression(&parser);
    consume(&parser, TOKEN_EOF, "Expect end of expression.");
    endCompiler(&parser);
    // return !parser.hadError;
    return parser.hadError;
}
