# Generate the NASM output from the sample input using the compiler logic

sample_input = """
::gate main:

   init: 
      • load.env
      • set.mode = SAFE

::end
"""

# Step 1: Tokenize
from typing import List

class TokenType:
    T_GATE = 'T_GATE'
    T_END = 'T_END'
    T_INIT = 'T_INIT'
    T_FUSE = 'T_FUSE'
    T_WHEN = 'T_WHEN'
    T_IDENTIFIER = 'T_IDENTIFIER'
    T_NUMBER = 'T_NUMBER'
    T_STRING = 'T_STRING'
    T_COLON = 'T_COLON'
    T_BULLET = 'T_BULLET'
    T_SEMI = 'T_SEMI'
    T_DASH = 'T_DASH'
    T_OP = 'T_OP'
    T_NEWLINE = 'T_NEWLINE'
    T_SPACE = 'T_SPACE'
    T_EOF = 'T_EOF'

class Token:
    def __init__(self, type_, value, line, column):
        self.type = type_
        self.value = value
        self.line = line
        self.column = column

class Lexer:
    def __init__(self, src):
        self.source = src
        self.pos = 0
        self.line = 1
        self.column = 1

    def peek(self):
        return self.source[self.pos] if self.pos < len(self.source) else '\0'

    def get(self):
        if self.pos >= len(self.source): return '\0'
        c = self.source[self.pos]
        self.pos += 1
        if c == '\n':
            self.line += 1
            self.column = 1
        else:
            self.column += 1
        return c

    def skip_whitespace(self):
        while self.peek().isspace() and self.peek() != '\n':
            self.get()

    def identifier(self):
        start_col = self.column
        value = ''
        while self.peek().isalnum() or self.peek() in ['_', '.', ':', '=']:
            value += self.get()

        if value == "::gate": return Token(TokenType.T_GATE, value, self.line, start_col)
        if value == "::end": return Token(TokenType.T_END, value, self.line, start_col)
        if value == "init": return Token(TokenType.T_INIT, value, self.line, start_col)
        if value == "::fuse": return Token(TokenType.T_FUSE, value, self.line, start_col)
        if value == "when": return Token(TokenType.T_WHEN, value, self.line, start_col)

        return Token(TokenType.T_IDENTIFIER, value, self.line, start_col)

    def next_token(self):
        self.skip_whitespace()
        c = self.peek()
        if c == '\0': return Token(TokenType.T_EOF, "", self.line, self.column)
        if c.isalpha() or c == ':': return self.identifier()
        if c == '\n': self.get(); return Token(TokenType.T_NEWLINE, "\\n", self.line, self.column)
        if c == '•': self.get(); return Token(TokenType.T_BULLET, "•", self.line, self.column)
        if c == ':': self.get(); return Token(TokenType.T_COLON, ":", self.line, self.column)
        self.get()
        return self.next_token()

    def tokenize(self):
        tokens = []
        while True:
            tok = self.next_token()
            tokens.append(tok)
            if tok.type == TokenType.T_EOF:
                break
        return tokens

class ASTNode:
    def emit(self, out):
        raise NotImplementedError

class InitStatement(ASTNode):
    def __init__(self):
        self.actions = []

    def emit(self, out):
        out.append("    ; INIT BLOCK")
        for act in self.actions:
            out.append(f"    ; action: {act}")
            out.append("    nop")

class GateBlock(ASTNode):
    def __init__(self, name):
        self.name = name
        self.body = []

    def emit(self, out):
        out.append(f"; Gate: {self.name}")
        for stmt in self.body:
            stmt.emit(out)

class Parser:
    def __init__(self, tokens):
        self.tokens = tokens
        self.current = 0

    def peek(self):
        return self.tokens[self.current]

    def match(self, kind):
        if self.peek().type == kind:
            self.current += 1
            return True
        return False

    def advance(self):
        self.current += 1
        return self.tokens[self.current - 1]

    def parse(self):
        return self.parse_gate()

    def parse_gate(self):
        self.match(TokenType.T_GATE)
        name_tok = self.advance()
        self.match(TokenType.T_COLON)
        self.match(TokenType.T_NEWLINE)

        gate = GateBlock(name_tok.value)
        while not self.match(TokenType.T_END):
            if self.peek().type == TokenType.T_EOF:
                break
            stmt = self.parse_statement()
            if stmt: gate.body.append(stmt)
        return gate

    def parse_statement(self):
        if self.match(TokenType.T_INIT):
            self.match(TokenType.T_COLON)
            stmt = InitStatement()
            while self.match(TokenType.T_BULLET):
                stmt.actions.append(self.advance().value)
                self.match(TokenType.T_NEWLINE)
            return stmt
        self.advance()
        return None

# Run compilation process
lexer = Lexer(sample_input)
tokens = lexer.tokenize()
parser = Parser(tokens)
ast = parser.parse()

nasm_output = []
ast.emit(nasm_output)

# Save to file
output_path = "/mnt/data/output.asm"
with open(output_path, "w") as f:
    f.write("\n".join(nasm_output))

output_path

# We'll extend the parser and AST to support `::fuse when` conditional blocks with actions,
# and simple assignment expressions with operators.

# Update the AST node classes to include FuseStatement and ExpressionStatement

class ExpressionStatement(ASTNode):
    def __init__(self, left, op=None, right=None):
        self.left = left
        self.op = op
        self.right = right

    def emit(self, out):
        if self.op:
            out.append(f"    ; expr: {self.left} {self.op} {self.right}")
        else:
            out.append(f"    ; expr: {self.left}")
        out.append("    nop")

class FuseStatement(ASTNode):
    def __init__(self, condition):
        self.condition = condition
        self.actions = []

    def emit(self, out):
        out.append(f"    ; FUSE WHEN {self.condition}")
        for act in self.actions:
            act.emit(out)

# Extend the parser

class ExtendedParser(Parser):
    def parse_statement(self):
        if self.match(TokenType.T_INIT):
            self.match(TokenType.T_COLON)
            stmt = InitStatement()
            while self.match(TokenType.T_BULLET):
                stmt.actions.append(self.advance().value)
                self.match(TokenType.T_NEWLINE)
            return stmt

        elif self.match(TokenType.T_FUSE):
            if not self.match(TokenType.T_WHEN): return None
            condition = self.advance().value
            self.match(TokenType.T_COLON)
            self.match(TokenType.T_NEWLINE)
            stmt = FuseStatement(condition)
            while self.match(TokenType.T_BULLET):
                left = self.advance().value
                op = None
                right = None
                if self.peek().type == TokenType.T_OP:
                    op = self.advance().value
                    right = self.advance().value
                stmt.actions.append(ExpressionStatement(left, op, right))
                self.match(TokenType.T_NEWLINE)
            return stmt

        elif self.match(TokenType.T_BULLET):
            left = self.advance().value
            op = None
            right = None
            if self.peek().type == TokenType.T_OP:
                op = self.advance().value
                right = self.advance().value
            self.match(TokenType.T_NEWLINE)
            return ExpressionStatement(left, op, right)

        return None

# Re-parse with extended parser

extended_parser = ExtendedParser(tokens)
ast = extended_parser.parse()

nasm_output = []
ast.emit(nasm_output)

# Save updated NASM output
output_path = "/mnt/data/output_fuse.asm"
with open(output_path, "w") as f:
    f.write("\n".join(nasm_output))

output_path

# Fix the parsing logic to avoid infinite loops and retry the process with Fuse and Expression handling

class FixedExtendedParser(Parser):
    def parse_statement(self):
        if self.match(TokenType.T_INIT):
            self.match(TokenType.T_COLON)
            stmt = InitStatement()
            while self.peek().type == TokenType.T_BULLET:
                self.match(TokenType.T_BULLET)
                stmt.actions.append(self.advance().value)
                self.match(TokenType.T_NEWLINE)
            return stmt

        elif self.match(TokenType.T_FUSE):
            if not self.match(TokenType.T_WHEN): return None
            condition = self.advance().value
            self.match(TokenType.T_COLON)
            self.match(TokenType.T_NEWLINE)
            stmt = FuseStatement(condition)
            while self.peek().type == TokenType.T_BULLET:
                self.match(TokenType.T_BULLET)
                left = self.advance().value
                op = None
                right = None
                if self.peek().type == TokenType.T_OP:
                    op = self.advance().value
                    right = self.advance().value
                self.match(TokenType.T_NEWLINE)
                stmt.actions.append(ExpressionStatement(left, op, right))
            return stmt

        elif self.peek().type == TokenType.T_BULLET:
            self.match(TokenType.T_BULLET)
            left = self.advance().value
            op = None
            right = None
            if self.peek().type == TokenType.T_OP:
                op = self.advance().value
                right = self.advance().value
            self.match(TokenType.T_NEWLINE)
            return ExpressionStatement(left, op, right)

        self.advance()
        return None

# Use extended sample input that includes fuse block
extended_sample_input = """
::gate main:

   init: 
      • load.env
      • set.mode = SAFE

   ::fuse when clock > 1200:
      • burn.signal @ch4
      • sync.pulse = ENABLED

::end
"""

# Tokenize and parse
lexer = Lexer(extended_sample_input)
tokens = lexer.tokenize()
fixed_parser = FixedExtendedParser(tokens)
ast = fixed_parser.parse()

# Emit NASM output
nasm_output = []
ast.emit(nasm_output)

# Save to file
output_path = "/mnt/data/output_fuse.asm"
with open(output_path, "w") as f:
    f.write("\n".join(nasm_output))

output_path

