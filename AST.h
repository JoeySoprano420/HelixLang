#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <iostream>

class ASTNode {
public:
    virtual ~ASTNode() {}
    virtual void emitNASM(std::ostream& out) = 0;
};

class InitStatement : public ASTNode {
public:
    std::vector<std::string> actions;
    void emitNASM(std::ostream& out) override;
};

class GateBlock : public ASTNode {
public:
    std::string name;
    std::vector<ASTNode*> body;
    void emitNASM(std::ostream& out) override;
};

#endif
