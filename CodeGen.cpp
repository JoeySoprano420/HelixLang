#include "AST.h"

void InitStatement::emitNASM(std::ostream& out) {
    out << "    ; INIT BLOCK\n";
    for (const auto& action : actions) {
        out << "    ; action: " << action << "\n";
        out << "    nop\n";
    }
}

void GateBlock::emitNASM(std::ostream& out) {
    out << "; Gate: " << name << "\n";
    for (auto stmt : body) {
        stmt->emitNASM(out);
    }
    out << "\n";
}
