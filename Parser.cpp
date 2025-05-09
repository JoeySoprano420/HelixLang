#include "Parser.h"
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Token Parser::advance() {
    if (current < tokens.size()) return tokens[current++];
    return { T_EOF, "", 0, 0 };
}

Token Parser::peek() {
    return tokens[current];
}

ASTNode* Parser::parse() {
    return parseGate();
}

ASTNode* Parser::parseGate() {
    match(T_GATE);
    Token nameTok = advance();  // gate name
    match(T_COLON);
    match(T_NEWLINE);

    auto gate = new GateBlock();
    gate->name = nameTok.value;

    while (!match(T_END)) {
        if (peek().type == T_EOF) break;
        ASTNode* stmt = parseStatement();
        if (stmt) gate->body.push_back(stmt);
    }

    return gate;
}

ASTNode* Parser::parseStatement() {
    if (match(T_INIT)) {
        match(T_COLON);
        auto init = new InitStatement();
        while (match(T_BULLET)) {
            Token act = advance();  // identifier
            init->actions.push_back(act.value);
            match(T_NEWLINE);
        }
        return init;
    }

    advance();  // fallback
    return nullptr;
}
