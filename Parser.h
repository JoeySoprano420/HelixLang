#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include "Tokens.h"
#include "AST.h"

class Parser {
public:
    Parser(const std::vector<Token>& tokens);
    ASTNode* parse();

private:
    std::vector<Token> tokens;
    size_t current;

    bool match(TokenType type);
    Token advance();
    Token peek();
    ASTNode* parseGate();
    ASTNode* parseStatement();
};

#endif
