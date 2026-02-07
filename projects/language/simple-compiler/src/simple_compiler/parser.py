"""Parser for SimpleLang."""

from typing import List, Optional
from .tokens import Token, TokenType
from .ast_nodes import *
from .errors import ParserError


class Parser:
    """Recursive descent parser for SimpleLang."""

    def __init__(self, tokens: List[Token]):
        self.tokens = tokens
        self.pos = 0

    def current_token(self) -> Token:
        """Get the current token."""
        if self.pos < len(self.tokens):
            return self.tokens[self.pos]
        return self.tokens[-1]  # Return EOF

    def peek_token(self, offset: int = 1) -> Token:
        """Look ahead at the next token."""
        pos = self.pos + offset
        if pos < len(self.tokens):
            return self.tokens[pos]
        return self.tokens[-1]  # Return EOF

    def advance(self) -> Token:
        """Move to the next token and return the current one."""
        token = self.current_token()
        if self.pos < len(self.tokens) - 1:
            self.pos += 1
        return token

    def expect(self, token_type: TokenType) -> Token:
        """Expect a specific token type and consume it."""
        token = self.current_token()
        if token.type != token_type:
            raise ParserError(
                f"Expected {token_type.name}, got {token.type.name}",
                token.line,
                token.column
            )
        return self.advance()

    def match(self, *token_types: TokenType) -> bool:
        """Check if current token matches any of the given types."""
        return self.current_token().type in token_types

    # Type parsing
    def parse_type(self) -> Type:
        """Parse a type."""
        token = self.current_token()
        if token.type == TokenType.INT:
            self.advance()
            return IntType(token.line, token.column)
        elif token.type == TokenType.BOOL:
            self.advance()
            return BoolType(token.line, token.column)
        elif token.type == TokenType.VOID:
            self.advance()
            return VoidType(token.line, token.column)
        else:
            raise ParserError(
                f"Expected type, got {token.type.name}",
                token.line,
                token.column
            )

    # Expression parsing (with precedence)
    def parse_expression(self) -> Expression:
        """Parse an expression (lowest precedence)."""
        return self.parse_logical_or()

    def parse_logical_or(self) -> Expression:
        """Parse logical OR expression."""
        left = self.parse_logical_and()

        while self.match(TokenType.OR):
            op_token = self.advance()
            right = self.parse_logical_and()
            left = BinaryOp(op_token.line, op_token.column, '||', left, right)

        return left

    def parse_logical_and(self) -> Expression:
        """Parse logical AND expression."""
        left = self.parse_equality()

        while self.match(TokenType.AND):
            op_token = self.advance()
            right = self.parse_equality()
            left = BinaryOp(op_token.line, op_token.column, '&&', left, right)

        return left

    def parse_equality(self) -> Expression:
        """Parse equality expression."""
        left = self.parse_comparison()

        while self.match(TokenType.EQ, TokenType.NE):
            op_token = self.advance()
            op = '==' if op_token.type == TokenType.EQ else '!='
            right = self.parse_comparison()
            left = BinaryOp(op_token.line, op_token.column, op, left, right)

        return left

    def parse_comparison(self) -> Expression:
        """Parse comparison expression."""
        left = self.parse_additive()

        while self.match(TokenType.LT, TokenType.GT, TokenType.LE, TokenType.GE):
            op_token = self.advance()
            op_map = {
                TokenType.LT: '<',
                TokenType.GT: '>',
                TokenType.LE: '<=',
                TokenType.GE: '>='
            }
            op = op_map[op_token.type]
            right = self.parse_additive()
            left = BinaryOp(op_token.line, op_token.column, op, left, right)

        return left

    def parse_additive(self) -> Expression:
        """Parse additive expression."""
        left = self.parse_multiplicative()

        while self.match(TokenType.PLUS, TokenType.MINUS):
            op_token = self.advance()
            op = '+' if op_token.type == TokenType.PLUS else '-'
            right = self.parse_multiplicative()
            left = BinaryOp(op_token.line, op_token.column, op, left, right)

        return left

    def parse_multiplicative(self) -> Expression:
        """Parse multiplicative expression."""
        left = self.parse_unary()

        while self.match(TokenType.STAR, TokenType.SLASH, TokenType.PERCENT):
            op_token = self.advance()
            op_map = {
                TokenType.STAR: '*',
                TokenType.SLASH: '/',
                TokenType.PERCENT: '%'
            }
            op = op_map[op_token.type]
            right = self.parse_unary()
            left = BinaryOp(op_token.line, op_token.column, op, left, right)

        return left

    def parse_unary(self) -> Expression:
        """Parse unary expression."""
        if self.match(TokenType.NOT, TokenType.MINUS):
            op_token = self.advance()
            op = '!' if op_token.type == TokenType.NOT else '-'
            operand = self.parse_unary()
            return UnaryOp(op_token.line, op_token.column, op, operand)

        return self.parse_primary()

    def parse_primary(self) -> Expression:
        """Parse primary expression."""
        token = self.current_token()

        # Integer literal
        if token.type == TokenType.INT_LITERAL:
            self.advance()
            return IntLiteral(token.line, token.column, token.value)

        # Boolean literal
        if token.type == TokenType.BOOL_LITERAL:
            self.advance()
            return BoolLiteral(token.line, token.column, token.value)

        # Identifier (variable or function call)
        if token.type == TokenType.IDENTIFIER:
            name = token.value
            self.advance()

            # Function call
            if self.match(TokenType.LPAREN):
                self.advance()
                arguments = []

                if not self.match(TokenType.RPAREN):
                    arguments.append(self.parse_expression())
                    while self.match(TokenType.COMMA):
                        self.advance()
                        arguments.append(self.parse_expression())

                self.expect(TokenType.RPAREN)
                return FunctionCall(token.line, token.column, name, arguments)

            # Variable
            return Variable(token.line, token.column, name)

        # Parenthesized expression
        if token.type == TokenType.LPAREN:
            self.advance()
            expr = self.parse_expression()
            self.expect(TokenType.RPAREN)
            return expr

        raise ParserError(
            f"Unexpected token in expression: {token.type.name}",
            token.line,
            token.column
        )

    # Statement parsing
    def parse_statement(self) -> Statement:
        """Parse a statement."""
        token = self.current_token()

        # Variable declaration
        if self.match(TokenType.INT, TokenType.BOOL):
            return self.parse_var_decl()

        # If statement
        if self.match(TokenType.IF):
            return self.parse_if_statement()

        # While statement
        if self.match(TokenType.WHILE):
            return self.parse_while_statement()

        # Return statement
        if self.match(TokenType.RETURN):
            return self.parse_return_statement()

        # Assignment or expression statement
        if self.match(TokenType.IDENTIFIER):
            # Look ahead to distinguish assignment from expression
            if self.peek_token().type == TokenType.ASSIGN:
                return self.parse_assignment()
            else:
                return self.parse_expression_statement()

        raise ParserError(
            f"Unexpected token in statement: {token.type.name}",
            token.line,
            token.column
        )

    def parse_var_decl(self) -> VarDecl:
        """Parse variable declaration."""
        var_type = self.parse_type()
        name_token = self.expect(TokenType.IDENTIFIER)
        name = name_token.value

        initializer = None
        if self.match(TokenType.ASSIGN):
            self.advance()
            initializer = self.parse_expression()

        self.expect(TokenType.SEMICOLON)
        return VarDecl(var_type.line, var_type.column, var_type, name, initializer)

    def parse_assignment(self) -> Assignment:
        """Parse assignment statement."""
        name_token = self.expect(TokenType.IDENTIFIER)
        name = name_token.value
        self.expect(TokenType.ASSIGN)
        value = self.parse_expression()
        self.expect(TokenType.SEMICOLON)
        return Assignment(name_token.line, name_token.column, name, value)

    def parse_if_statement(self) -> IfStatement:
        """Parse if statement."""
        if_token = self.expect(TokenType.IF)
        self.expect(TokenType.LPAREN)
        condition = self.parse_expression()
        self.expect(TokenType.RPAREN)

        self.expect(TokenType.LBRACE)
        then_branch = self.parse_block()
        self.expect(TokenType.RBRACE)

        else_branch = None
        if self.match(TokenType.ELSE):
            self.advance()
            self.expect(TokenType.LBRACE)
            else_branch = self.parse_block()
            self.expect(TokenType.RBRACE)

        return IfStatement(if_token.line, if_token.column, condition, then_branch, else_branch)

    def parse_while_statement(self) -> WhileStatement:
        """Parse while statement."""
        while_token = self.expect(TokenType.WHILE)
        self.expect(TokenType.LPAREN)
        condition = self.parse_expression()
        self.expect(TokenType.RPAREN)

        self.expect(TokenType.LBRACE)
        body = self.parse_block()
        self.expect(TokenType.RBRACE)

        return WhileStatement(while_token.line, while_token.column, condition, body)

    def parse_return_statement(self) -> ReturnStatement:
        """Parse return statement."""
        return_token = self.expect(TokenType.RETURN)

        value = None
        if not self.match(TokenType.SEMICOLON):
            value = self.parse_expression()

        self.expect(TokenType.SEMICOLON)
        return ReturnStatement(return_token.line, return_token.column, value)

    def parse_expression_statement(self) -> ExpressionStatement:
        """Parse expression statement."""
        expr = self.parse_expression()
        self.expect(TokenType.SEMICOLON)
        return ExpressionStatement(expr.line, expr.column, expr)

    def parse_block(self) -> List[Statement]:
        """Parse a block of statements."""
        statements = []
        while not self.match(TokenType.RBRACE, TokenType.EOF):
            statements.append(self.parse_statement())
        return statements

    # Function parsing
    def parse_function(self) -> FunctionDecl:
        """Parse function declaration."""
        return_type = self.parse_type()
        name_token = self.expect(TokenType.IDENTIFIER)
        name = name_token.value

        self.expect(TokenType.LPAREN)
        parameters = []

        if not self.match(TokenType.RPAREN):
            # Parse parameters
            param_type = self.parse_type()
            param_name = self.expect(TokenType.IDENTIFIER).value
            parameters.append((param_type, param_name))

            while self.match(TokenType.COMMA):
                self.advance()
                param_type = self.parse_type()
                param_name = self.expect(TokenType.IDENTIFIER).value
                parameters.append((param_type, param_name))

        self.expect(TokenType.RPAREN)
        self.expect(TokenType.LBRACE)
        body = self.parse_block()
        self.expect(TokenType.RBRACE)

        return FunctionDecl(return_type.line, return_type.column, return_type, name, parameters, body)

    # Program parsing
    def parse(self) -> Program:
        """Parse the entire program."""
        functions = []

        while not self.match(TokenType.EOF):
            functions.append(self.parse_function())

        token = self.current_token()
        return Program(token.line, token.column, functions)
