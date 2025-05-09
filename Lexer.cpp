#include "Lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& src)
    : source(src), pos(0), line(1), column(1) {}

char Lexer::peek() const {
    return pos < source.size() ? source[pos] : '\0';
}

char Lexer::get() {
    if (pos >= source.size()) return '\0';
    char c = source[pos++];
    if (c == '\n') { line++; column = 1; }
    else { column++; }
    return c;
}

void Lexer::skipWhitespace() {
    while (isspace(peek()) && peek() != '\n') get();
}

Token Lexer::identifier() {
    int startCol = column;
    std::string value;
    while (isalnum(peek()) || peek() == '_' || peek() == ':') {
        value += get();
    }

    if (value == "::gate") return { T_GATE, value, line, startCol };
    if (value == "::end") return { T_END, value, line, startCol };
    if (value == "::fuse") return { T_FUSE, value, line, startCol };
    if (value == "when") return { T_WHEN, value, line, startCol };
    if (value == "init") return { T_INIT, value, line, startCol };

    return { T_IDENTIFIER, value, line, startCol };
}

Token Lexer::number() {
    int startCol = column;
    std::string value;
    while (isdigit(peek())) value += get();
    return { T_NUMBER, value, line, startCol };
}

Token Lexer::nextToken() {
    skipWhitespace();
    char c = peek();

    if (c == '\0') return { T_EOF, "", line, column };
    if (isalpha(c) || c == ':') return identifier();
    if (isdigit(c)) return number();
    if (c == '\n') { get(); return { T_NEWLINE, "\\n", line, column }; }
    if (c == ':') { get(); return { T_COLON, ":", line, column }; }
    if (c == '•') { get(); return { T_BULLET, "•", line, column }; }
    if (c == ';') { get(); return { T_SEMI, ";", line, column }; }
    if (c == '—') { get(); return { T_DASH, "—", line, column }; }
    if (c == '=' || c == '@' || c == '%') {
        std::string op;
        op += get();
        if (peek() == '=') op += get();
        return { T_OP, op, line, column };
    }

    get();  // Skip unknown
    return nextToken();
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    Token tok;
    do {
        tok = nextToken();
        tokens.push_back(tok);
    } while (tok.type != T_EOF);
    return tokens;
}
