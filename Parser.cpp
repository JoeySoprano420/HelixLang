#include "Parser.h"

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), current(0) {}

Token Parser::peek() {
    return tokens[current];
}

bool Parser::match(TokenType type) {
    if (peek().type == type) {
        advance();
        return true;
    }
    return false;
}

Token Parser::advance() {
    return tokens[current++];
}

ASTNode* Parser::parse() {
    return parseGate();
}

ASTNode* Parser::parseGate() {
    match(T_GATE);
    Token nameTok = advance();
    match(T_COLON);
    match(T_NEWLINE);

    auto gate = new GateBlock();
    gate->name = nameTok.value;

    while (!match(T_END) && peek().type != T_EOF) {
        ASTNode* stmt = parseStatement();
        if (stmt) gate->body.push_back(stmt);
    }

    return gate;
}

ASTNode* Parser::parseStatement() {
    if (match(T_INIT)) {
        match(T_COLON);
        auto stmt = new InitStatement();
        while (peek().type == T_BULLET) {
            match(T_BULLET);
            stmt->actions.push_back(advance().value);
            match(T_NEWLINE);
        }
        return stmt;
    }

    if (match(T_FUSE)) {
        if (!match(T_WHEN)) return nullptr;
        std::string cond = advance().value;
        match(T_COLON);
        match(T_NEWLINE);
        auto fuse = new FuseStatement(cond);
        while (peek().type == T_BULLET) {
            match(T_BULLET);
            std::string left = advance().value;
            std::string op = "", right = "";
            if (peek().type == T_OP) {
                op = advance().value;
                right = advance().value;
            }
            match(T_NEWLINE);
            fuse->actions.push_back(new ExpressionStatement(left, op, right));
        }
        return fuse;
    }

    if (match(T_BULLET)) {
        std::string left = advance().value;
        std::string op = "", right = "";
        if (peek().type == T_OP) {
            op = advance().value;
            right = advance().value;
        }
        match(T_NEWLINE);
        return new ExpressionStatement(left, op, right);
    }

    advance();
    return nullptr;
}
