"""Evaluator for Tiny Interpreter.

The evaluator executes AST nodes in an environment.
"""

from typing import Any, List, Callable
from .parser import ASTNode, Number, Boolean, Symbol, SExpression
from .environment import Environment


class EvaluatorError(Exception):
    """Exception raised for evaluation errors."""
    pass


class Closure:
    """A closure captures a function and its defining environment."""

    def __init__(self, params: List[str], body: List[ASTNode], env: Environment):
        self.params = params
        self.body = body
        self.env = env

    def __repr__(self):
        return f"<closure {self.params}>"


class Evaluator:
    """Evaluator for executing AST nodes."""

    def __init__(self):
        self.global_env = self.create_global_environment()

    def create_global_environment(self) -> Environment:
        """Create the global environment with built-in functions."""
        env = Environment()

        # Arithmetic operations
        env.define('+', lambda *args: sum(args))
        env.define('-', lambda a, b: a - b)
        env.define('*', lambda *args: eval('*'.join(map(str, args))) if args else 1)
        env.define('/', lambda a, b: a // b)  # Integer division

        # Comparison operations
        env.define('=', lambda a, b: a == b)
        env.define('<', lambda a, b: a < b)
        env.define('>', lambda a, b: a > b)
        env.define('<=', lambda a, b: a <= b)
        env.define('>=', lambda a, b: a >= b)

        # List operations
        env.define('cons', lambda a, b: [a] + (b if isinstance(b, list) else [b]))
        env.define('car', lambda lst: lst[0] if lst else None)
        env.define('cdr', lambda lst: lst[1:] if len(lst) > 1 else [])
        env.define('list', lambda *args: list(args))
        env.define('null?', lambda lst: len(lst) == 0 if isinstance(lst, list) else False)

        # Type predicates
        env.define('number?', lambda x: isinstance(x, int))
        env.define('boolean?', lambda x: isinstance(x, bool))
        env.define('list?', lambda x: isinstance(x, list))

        return env

    def eval(self, node: ASTNode, env: Environment) -> Any:
        """Evaluate an AST node in an environment.

        Args:
            node: AST node to evaluate.
            env: Environment for variable lookups.

        Returns:
            The result of evaluation.
        """
        # Self-evaluating expressions
        if isinstance(node, Number):
            return node.value

        if isinstance(node, Boolean):
            return node.value

        # Variable lookup
        if isinstance(node, Symbol):
            return env.get(node.name)

        # S-expressions (function calls and special forms)
        if isinstance(node, SExpression):
            if len(node.elements) == 0:
                return []  # Empty list

            first = node.elements[0]

            # Special forms
            if isinstance(first, Symbol):
                # define
                if first.name == 'define':
                    return self.eval_define(node.elements[1:], env)

                # lambda
                if first.name == 'lambda':
                    return self.eval_lambda(node.elements[1:], env)

                # if
                if first.name == 'if':
                    return self.eval_if(node.elements[1:], env)

                # quote
                if first.name == 'quote':
                    return self.eval_quote(node.elements[1:])

                # begin (sequence of expressions)
                if first.name == 'begin':
                    return self.eval_begin(node.elements[1:], env)

            # Function application
            return self.eval_application(node.elements, env)

        raise EvaluatorError(f"Unknown node type: {type(node)}")

    def eval_define(self, args: List[ASTNode], env: Environment) -> None:
        """Evaluate a define expression.

        (define name value)
        """
        if len(args) != 2:
            raise EvaluatorError(f"define expects 2 arguments, got {len(args)}")

        name_node = args[0]
        if not isinstance(name_node, Symbol):
            raise EvaluatorError("define expects a symbol as first argument")

        value = self.eval(args[1], env)
        env.define(name_node.name, value)
        return None

    def eval_lambda(self, args: List[ASTNode], env: Environment) -> Closure:
        """Evaluate a lambda expression.

        (lambda (params...) body...)
        """
        if len(args) < 2:
            raise EvaluatorError("lambda expects at least 2 arguments")

        params_node = args[0]
        if not isinstance(params_node, SExpression):
            raise EvaluatorError("lambda expects a list of parameters")

        params = []
        for param in params_node.elements:
            if not isinstance(param, Symbol):
                raise EvaluatorError("lambda parameters must be symbols")
            params.append(param.name)

        body = args[1:]
        return Closure(params, body, env)

    def eval_if(self, args: List[ASTNode], env: Environment) -> Any:
        """Evaluate an if expression.

        (if condition then-expr else-expr)
        """
        if len(args) != 3:
            raise EvaluatorError(f"if expects 3 arguments, got {len(args)}")

        condition = self.eval(args[0], env)
        if condition:
            return self.eval(args[1], env)
        else:
            return self.eval(args[2], env)

    def eval_quote(self, args: List[ASTNode]) -> Any:
        """Evaluate a quote expression.

        (quote expr)
        """
        if len(args) != 1:
            raise EvaluatorError(f"quote expects 1 argument, got {len(args)}")

        return self.ast_to_value(args[0])

    def eval_begin(self, args: List[ASTNode], env: Environment) -> Any:
        """Evaluate a begin expression (sequence).

        (begin expr1 expr2 ... exprN)
        """
        result = None
        for expr in args:
            result = self.eval(expr, env)
        return result

    def eval_application(self, elements: List[ASTNode], env: Environment) -> Any:
        """Evaluate a function application.

        (func arg1 arg2 ...)
        """
        func = self.eval(elements[0], env)
        args = [self.eval(arg, env) for arg in elements[1:]]

        # Built-in function
        if callable(func) and not isinstance(func, Closure):
            return func(*args)

        # User-defined function (closure)
        if isinstance(func, Closure):
            if len(args) != len(func.params):
                raise EvaluatorError(
                    f"Function expects {len(func.params)} arguments, got {len(args)}"
                )

            # Create new environment for function execution
            func_env = Environment(func.env)
            for param, arg in zip(func.params, args):
                func_env.define(param, arg)

            # Evaluate function body
            result = None
            for expr in func.body:
                result = self.eval(expr, func_env)
            return result

        raise EvaluatorError(f"Not a function: {func}")

    def ast_to_value(self, node: ASTNode) -> Any:
        """Convert an AST node to a value (for quote)."""
        if isinstance(node, Number):
            return node.value
        if isinstance(node, Boolean):
            return node.value
        if isinstance(node, Symbol):
            return node.name
        if isinstance(node, SExpression):
            return [self.ast_to_value(elem) for elem in node.elements]
        return node

    def run(self, source: str) -> Any:
        """Parse and evaluate source code.

        Args:
            source: Source code string.

        Returns:
            The result of the last expression.
        """
        from .parser import parse

        ast_nodes = parse(source)
        result = None
        for node in ast_nodes:
            result = self.eval(node, self.global_env)
        return result
