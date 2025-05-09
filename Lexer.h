#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "Tokens.h"

class Lexer {
public:
    Lexer(const std::string& src);
    Token nextToken();
    std::vector<Token> tokenize();

private:
    std::string source;
    size_t pos;
    int line, column;

    char peek() const;
    char get();
    void skipWhitespace();
    Token identifier();
    Token number();
};

#endif
