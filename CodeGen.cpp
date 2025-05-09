#include "AST.h"

void InitStatement::emitNASM(std::ostream& out) {
    out << "    ; INIT BLOCK\n";
    for (const auto& action : actions) {
        out << "    ; action: " << action << "\n";
        out << "    nop\n";
    }
}

void ExpressionStatement::emitNASM(std::ostream& out) {
    if (!op.empty())
        out << "    ; expr: " << left << " " << op << " " << right << "\n";
    else
        out << "    ; expr: " << left << "\n";
    out << "    nop\n";
}

void FuseStatement::emitNASM(std::ostream& out) {
    out << "    ; FUSE WHEN " << condition << "\n";
    for (auto* act : actions) {
        act->emitNASM(out);
    }
}

void GateBlock::emitNASM(std::ostream& out) {
    out << "; Gate: " << name << "\n";
    for (auto* stmt : body) {
        stmt->emitNASM(out);
    }
}
