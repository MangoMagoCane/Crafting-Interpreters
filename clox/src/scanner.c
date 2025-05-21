#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

void initScanner(Scanner *scanner, const char *source) {
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
}

static bool isAlpha(char c) {
    return (c >= 'a' && c <= 'z') || 
           (c >= 'A' && c <= 'Z') || 
            c == '_';
}

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

static bool isAtEnd(Scanner *scanner) {
    return *scanner->current == '\0';
}

static char advance(Scanner *scanner) {
    scanner->current++;
    return scanner->current[-1];
}

static char peek(Scanner *scanner) {
    return *scanner->current;
}

static char peekNext(Scanner *scanner) {
    if (isAtEnd(scanner)) return '\0';
    return scanner->current[1];
}

static char match(Scanner *scanner, char expected) {
    if (isAtEnd(scanner)) return false;
    if (*scanner->current != expected) return false;
    scanner->current++;
    return true;
}

static Token makeToken(Scanner *scanner, TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = scanner->current - scanner->start;
    token.line = scanner->line;
    return token;
}

static Token errorToken(Scanner *scanner, const char *message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = strlen(message);
    token.line = scanner->line;
    return token;
}

static void skipWhiteSpace(Scanner *scanner) {
    for (;;) {
        char c = peek(scanner);
        switch (c) {
        case ' ':
        case '\r':
        case '\t':
            advance(scanner);
            break;
        case '/':
            if (peekNext(scanner) == '/') {
                while (peek(scanner) != '\n' && !isAtEnd(scanner)) {
                    advance(scanner);
                }
            } else {
                return;
            }
            break;
        case '\n':
            scanner->line++;
            advance(scanner);
            break;
        default:
            return;
        }
    }
}

static TokenType checkKeyword(Scanner *scanner, int start, int length,
                              const char *rest, TokenType type) {
    if (scanner->current - scanner->start == start + length &&
            memcmp(scanner->start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType(Scanner *scanner) {
#define checkKeyword(start, length, rest, type) \
    checkKeyword(scanner, start, length, rest, type)

    switch (scanner->start[0]) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
        if (scanner->current - scanner->start > 1) {
            switch (scanner->start[1]) {
            case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
            case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
            case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
            }
        }
        break;
    case 'i': return checkKeyword(1, 1, "f", TOKEN_IF);
    case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
    case 't':
        if (scanner->current - scanner->start > 1) {
            switch (scanner->start[1]) {
            case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
            case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
            }
        }
        break;
    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
    }

    return TOKEN_IDENTIFIER;

#undef checkKeyword
}

static Token identifier(Scanner *scanner) {
    while (isAlpha(peek(scanner)) || isDigit(peek(scanner))) {
        advance(scanner);
    }
    return makeToken(scanner, identifierType(scanner));
}

static Token number(Scanner *scanner) {
    while (isDigit(peek(scanner))) advance(scanner);

    if (peek(scanner) == '.' && isDigit(peekNext(scanner))) {
        advance(scanner);
        while (isDigit(peek(scanner))) advance(scanner);
    }

    return makeToken(scanner, TOKEN_NUMBER);
}

static Token string(Scanner *scanner) {
    while (peek(scanner) != '"' && !isAtEnd(scanner)) {
        if (peek(scanner) == '\n') scanner->line++;
        advance(scanner);
    }

    if (isAtEnd(scanner)) return errorToken(scanner, "Unterminated string.");

    advance(scanner); // Closing quote.
    return makeToken(scanner, TOKEN_STRING);
}

Token scanToken(Scanner *scanner) {
#define makeToken(token) makeToken(scanner, token)

    skipWhiteSpace(scanner);
    scanner->start = scanner->current;

    if (isAtEnd(scanner)) return makeToken(TOKEN_EOF);

    char c = advance(scanner);
    if (isAlpha(c)) return identifier(scanner);
    if (isDigit(c)) return number(scanner);

    switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': return makeToken(TOKEN_RIGHT_BRACE);
    case ':': return makeToken(TOKEN_COLON);
    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '-': return makeToken(TOKEN_MINUS);
    case '+': return makeToken(TOKEN_PLUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '*': return makeToken(TOKEN_STAR);
    case '?': return makeToken(TOKEN_EROTEME);
    case '^': return makeToken(TOKEN_CARET);
    case '!':
        return makeToken(
                match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
        return makeToken(
                match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
        return makeToken(
                match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
        return makeToken(
                match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
    case '"': return string(scanner);
    }
 
    return errorToken(scanner, "Unexpected character.");

#undef makeToken
}
