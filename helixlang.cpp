#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cctype>

//====================== TOKENS ======================//
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

//====================== LEXER ======================//
class Lexer {
public:
    Lexer(const std::string& src) : source(src), pos(0), line(1), column(1) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        Token tok;
        do {
            tok = nextToken();
            tokens.push_back(tok);
        } while (tok.type != T_EOF);
        return tokens;
    }

private:
    std::string source;
    size_t pos;
    int line, column;

    char peek() const {
        return pos < source.size() ? source[pos] : '\0';
    }

    char get() {
        if (pos >= source.size()) return '\0';
        char c = source[pos++];
        if (c == '\n') { line++; column = 1; }
        else { column++; }
        return c;
    }

    void skipWhitespace() {
        while (isspace(peek()) && peek() != '\n') get();
    }

    Token identifier() {
        int startCol = column;
        std::string value;
        while (isalnum(peek()) || peek() == '_' || peek() == '.' || peek() == ':' || peek() == '=') {
            value += get();
        }

        if (value == "::gate") return { T_GATE, value, line, startCol };
        if (value == "::end") return { T_END, value, line, startCol };
        if (value == "::fuse") return { T_FUSE, value, line, startCol };
        if (value == "when") return { T_WHEN, value, line, startCol };
        if (value == "init") return { T_INIT, value, line, startCol };

        return { T_IDENTIFIER, value, line, startCol };
    }

    Token nextToken() {
        skipWhitespace();
        char c = peek();

        if (c == '\0') return { T_EOF, "", line, column };
        if (isalpha(c) || c == ':') return identifier();
        if (isdigit(c)) return { T_NUMBER, std::string(1, get()), line, column };
        if (c == '\n') { get(); return { T_NEWLINE, "\\n", line, column }; }
        if (c == '•') { get(); return { T_BULLET, "•", line, column }; }
        if (c == ':') { get(); return { T_COLON, ":", line, column }; }
        if (c == '=') { get(); return { T_OP, "=", line, column }; }

        get(); // skip unknown
        return nextToken();
    }
};

//====================== AST ======================//
class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual void emitNASM(std::ostream& out) = 0;
};

class InitStatement : public ASTNode {
public:
    std::vector<std::string> actions;
    void emitNASM(std::ostream& out) override {
        out << "    ; INIT BLOCK\n";
        for (const auto& action : actions) {
            out << "    ; action: " << action << "\n";
            out << "    nop\n";
        }
    }
};

class ExpressionStatement : public ASTNode {
public:
    std::string left, op, right;
    ExpressionStatement(std::string l, std::string o = "", std::string r = "") :
        left(l), op(o), right(r) {}

    void emitNASM(std::ostream& out) override {
        if (!op.empty())
            out << "    ; expr: " << left << " " << op << " " << right << "\n";
        else
            out << "    ; expr: " << left << "\n";
        out << "    nop\n";
    }
};

class FuseStatement : public ASTNode {
public:
    std::string condition;
    std::vector<ASTNode*> actions;
    FuseStatement(std::string cond) : condition(cond) {}

    void emitNASM(std::ostream& out) override {
        out << "    ; FUSE WHEN " << condition << "\n";
        for (auto* act : actions) {
            act->emitNASM(out);
        }
    }
};

class GateBlock : public ASTNode {
public:
    std::string name;
    std::vector<ASTNode*> body;

    void emitNASM(std::ostream& out) override {
        out << "; Gate: " << name << "\n";
        for (auto* stmt : body) {
            stmt->emitNASM(out);
        }
    }
};

//====================== PARSER ======================//
class Parser {
public:
    Parser(const std::vector<Token>& tokens) : tokens(tokens), current(0) {}

    ASTNode* parse() {
        return parseGate();
    }

private:
    std::vector<Token> tokens;
    size_t current;

    Token peek() {
        return tokens[current];
    }

    bool match(TokenType type) {
        if (peek().type == type) {
            current++;
            return true;
        }
        return false;
    }

    Token advance() {
        return tokens[current++];
    }

    ASTNode* parseGate() {
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

    ASTNode* parseStatement() {
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
};

//====================== MAIN ======================//
int main() {
    std::ifstream file("program.hlx");
    if (!file) {
        std::cerr << "Could not open input file.\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    ASTNode* program = parser.parse();

    std::ofstream out("output.asm");
    program->emitNASM(out);

    std::cout << "Compiled to output.asm\n";
    return 0;
}
