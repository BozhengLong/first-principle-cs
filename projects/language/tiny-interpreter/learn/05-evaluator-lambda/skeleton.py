"""Lambda 与闭包骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/05-evaluator-lambda/test_skeleton.py -v

这个模块在模块 4 的基础上添加 lambda 和闭包支持。
"""

from typing import Any, List

import sys
import os
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from src.tiny_interpreter.parser import ASTNode, Number, Boolean, Symbol, SExpression, parse
from src.tiny_interpreter.environment import Environment


class EvaluatorError(Exception):
    """求值错误。"""
    pass


class Closure:
    """闭包：捕获函数定义时的环境。

    TODO: 理解这个类

    闭包是函数的运行时表示，包含：
    - params: 参数名列表
    - body: 函数体（AST 节点列表）
    - env: 定义时的环境（这是闭包的关键！）

    当闭包被调用时：
    1. 创建新环境，parent 指向 self.env
    2. 在新环境中绑定参数
    3. 在新环境中执行函数体
    """

    def __init__(self, params: List[str], body: List[ASTNode], env: Environment):
        """创建闭包。

        Args:
            params: 参数名列表，如 ['x', 'y']
            body: 函数体，AST 节点列表
            env: 定义时的环境（会被闭包"捕获"）
        """
        self.params = params
        self.body = body
        self.env = env  # 这是闭包的魔力所在！

    def __repr__(self):
        return f"<closure {self.params}>"


class Evaluator:
    """求值器：支持 lambda 和闭包。"""

    def __init__(self):
        """初始化求值器。"""
        self.global_env = self.create_global_environment()

    def create_global_environment(self) -> Environment:
        """创建全局环境。"""
        env = Environment()

        # 算术运算
        env.define('+', lambda *args: sum(args))
        env.define('-', lambda a, b: a - b)
        env.define('*', lambda *args: 1 if not args else args[0] if len(args) == 1 else args[0] * self.global_env.get('*')(*args[1:]))
        env.define('/', lambda a, b: a // b)

        # 比较运算
        env.define('=', lambda a, b: a == b)
        env.define('<', lambda a, b: a < b)
        env.define('>', lambda a, b: a > b)
        env.define('<=', lambda a, b: a <= b)
        env.define('>=', lambda a, b: a >= b)

        # 列表操作
        env.define('cons', lambda a, b: [a] + (b if isinstance(b, list) else [b]))
        env.define('car', lambda lst: lst[0] if lst else None)
        env.define('cdr', lambda lst: lst[1:] if len(lst) > 1 else [])
        env.define('list', lambda *args: list(args))
        env.define('null?', lambda lst: len(lst) == 0 if isinstance(lst, list) else False)

        # 类型判断
        env.define('number?', lambda x: isinstance(x, int))
        env.define('boolean?', lambda x: isinstance(x, bool))
        env.define('list?', lambda x: isinstance(x, list))

        return env

    def eval(self, node: ASTNode, env: Environment) -> Any:
        """求值 AST 节点。"""
        if isinstance(node, Number):
            return node.value

        if isinstance(node, Boolean):
            return node.value

        if isinstance(node, Symbol):
            return env.get(node.name)

        if isinstance(node, SExpression):
            if len(node.elements) == 0:
                return []

            first = node.elements[0]

            if isinstance(first, Symbol):
                if first.name == 'define':
                    return self.eval_define(node.elements[1:], env)

                if first.name == 'lambda':
                    return self.eval_lambda(node.elements[1:], env)

                if first.name == 'if':
                    return self.eval_if(node.elements[1:], env)

                if first.name == 'quote':
                    return self.eval_quote(node.elements[1:])

                if first.name == 'begin':
                    return self.eval_begin(node.elements[1:], env)

            return self.eval_application(node.elements, env)

        raise EvaluatorError(f"Unknown node type: {type(node)}")

    def eval_define(self, args: List[ASTNode], env: Environment) -> None:
        """求值 define。"""
        if len(args) != 2:
            raise EvaluatorError(f"define expects 2 arguments, got {len(args)}")

        name_node = args[0]
        if not isinstance(name_node, Symbol):
            raise EvaluatorError("define expects a symbol as first argument")

        value = self.eval(args[1], env)
        env.define(name_node.name, value)
        return None

    def eval_lambda(self, args: List[ASTNode], env: Environment) -> Closure:
        """求值 lambda 表达式，创建闭包。

        TODO: 实现这个方法

        形式：(lambda (param1 param2 ...) body1 body2 ...)

        Args:
            args: lambda 的参数，[params-list, body1, body2, ...]
            env: 当前环境

        Returns:
            Closure 对象

        步骤：
        1. 检查参数数量至少为 2（参数列表 + 至少一个 body）
        2. 提取参数列表（第一个元素，应该是 SExpression）
        3. 验证参数列表中的每个元素都是 Symbol
        4. 提取参数名列表
        5. 提取函数体（剩余的元素）
        6. 创建 Closure，捕获当前环境 env

        关键：Closure 的 env 参数应该是当前的 env！
        这就是闭包"捕获"环境的地方。
        """
        # TODO: 实现
        pass

    def eval_if(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 if。"""
        if len(args) != 3:
            raise EvaluatorError(f"if expects 3 arguments, got {len(args)}")

        condition = self.eval(args[0], env)
        if condition:
            return self.eval(args[1], env)
        else:
            return self.eval(args[2], env)

    def eval_quote(self, args: List[ASTNode]) -> Any:
        """求值 quote。"""
        if len(args) != 1:
            raise EvaluatorError(f"quote expects 1 argument, got {len(args)}")
        return self.ast_to_value(args[0])

    def eval_begin(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 begin。"""
        result = None
        for expr in args:
            result = self.eval(expr, env)
        return result

    def eval_application(self, elements: List[ASTNode], env: Environment) -> Any:
        """求值函数调用。

        TODO: 修改这个方法以支持闭包调用

        形式：(func arg1 arg2 ...)

        Args:
            elements: [func, arg1, arg2, ...]
            env: 当前环境

        Returns:
            函数调用结果

        步骤：
        1. 求值函数
        2. 求值所有参数
        3. 如果函数是内置函数（callable 但不是 Closure），直接调用
        4. 如果函数是 Closure：
           a. 检查参数数量是否匹配
           b. 创建新环境，parent 是 closure.env（不是当前 env！）
           c. 在新环境中绑定参数
           d. 在新环境中依次求值函数体
           e. 返回最后一个表达式的值

        关键：新环境的 parent 是 closure.env，这实现了词法作用域！
        """
        # 1. 求值函数
        func = self.eval(elements[0], env)

        # 2. 求值参数
        args = [self.eval(arg, env) for arg in elements[1:]]

        # 3. 内置函数
        if callable(func) and not isinstance(func, Closure):
            return func(*args)

        # 4. 闭包调用
        if isinstance(func, Closure):
            # TODO: 实现闭包调用
            # a. 检查参数数量
            # b. 创建新环境，parent 是 func.env
            # c. 绑定参数
            # d. 执行函数体
            # e. 返回结果
            pass

        raise EvaluatorError(f"Not a function: {func}")

    def ast_to_value(self, node: ASTNode) -> Any:
        """将 AST 转换为值。"""
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
        """解析并执行源代码。"""
        ast_nodes = parse(source)
        result = None
        for node in ast_nodes:
            result = self.eval(node, self.global_env)
        return result
