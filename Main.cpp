#include <iostream>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "AST.h"

int main() {
    std::ifstream file("program.hlx");
    if (!file) {
        std::cerr << "Could not open source file.\n";
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

    std::cout << "Compilation complete.\n";
    return 0;
}
