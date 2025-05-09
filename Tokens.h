#ifndef TOKENS_H
#define TOKENS_H

#include <string>

enum TokenType {
    T_GATE, T_END, T_INIT, T_FUSE, T_WHEN,
    T_IDENTIFIER, T_NUMBER, T_STRING,
    T_COLON, T_BULLET, T_SEMI, T_DASH, T_OP,
    T_NEWLINE, T_SPACE, T_EOF
};

struct Token {
    TokenType type;
    std::string value;
    int line, column;
};

#endif
