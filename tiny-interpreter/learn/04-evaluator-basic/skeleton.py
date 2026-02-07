"""基础求值器骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/04-evaluator-basic/test_skeleton.py -v

注意：
    这个模块只实现基础求值，不包括 lambda。
    lambda 和闭包在模块 5 中实现。
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


class Evaluator:
    """求值器：执行 AST 节点。

    使用方法：
        evaluator = Evaluator()
        result = evaluator.run("(+ 1 2)")
    """

    def __init__(self):
        """初始化求值器，创建全局环境。"""
        self.global_env = self.create_global_environment()

    def create_global_environment(self) -> Environment:
        """创建包含内置函数的全局环境。

        内置函数包括：
        - 算术：+, -, *, /
        - 比较：=, <, >, <=, >=
        - 列表：cons, car, cdr, list, null?
        - 类型：number?, boolean?, list?
        """
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
        """求值一个 AST 节点。

        TODO: 实现这个方法

        Args:
            node: AST 节点
            env: 当前环境

        Returns:
            求值结果

        求值规则：
        1. Number → 返回 node.value
        2. Boolean → 返回 node.value
        3. Symbol → 在环境中查找 env.get(node.name)
        4. SExpression → 特殊形式或函数调用

        对于 SExpression：
        - 如果是空列表 () → 返回空列表 []
        - 如果第一个元素是 Symbol：
          - "define" → 调用 eval_define
          - "if" → 调用 eval_if
          - "quote" → 调用 eval_quote
          - "begin" → 调用 eval_begin
          - 否则 → 函数调用，调用 eval_application
        - 如果第一个元素不是 Symbol → 函数调用
        """
        # TODO: 实现
        pass

    def eval_define(self, args: List[ASTNode], env: Environment) -> None:
        """求值 define 表达式。

        TODO: 实现这个方法

        形式：(define name value)

        Args:
            args: define 的参数列表 [name, value]
            env: 当前环境

        Returns:
            None

        步骤：
        1. 检查参数数量是否为 2
        2. 检查第一个参数是否是 Symbol
        3. 求值第二个参数
        4. 在环境中定义变量
        """
        # TODO: 实现
        pass

    def eval_if(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 if 表达式。

        TODO: 实现这个方法

        形式：(if condition then-expr else-expr)

        Args:
            args: if 的参数列表 [condition, then-expr, else-expr]
            env: 当前环境

        Returns:
            then-expr 或 else-expr 的值

        步骤：
        1. 检查参数数量是否为 3
        2. 求值条件
        3. 如果条件为真，求值并返回 then-expr
        4. 否则求值并返回 else-expr

        注意：只求值需要的分支！
        """
        # TODO: 实现
        pass

    def eval_quote(self, args: List[ASTNode]) -> Any:
        """求值 quote 表达式。

        TODO: 实现这个方法

        形式：(quote expr)

        Args:
            args: quote 的参数列表 [expr]

        Returns:
            expr 转换为值（不求值）

        步骤：
        1. 检查参数数量是否为 1
        2. 调用 ast_to_value 转换 AST 为值
        """
        # TODO: 实现
        pass

    def eval_begin(self, args: List[ASTNode], env: Environment) -> Any:
        """求值 begin 表达式。

        TODO: 实现这个方法

        形式：(begin expr1 expr2 ... exprN)

        Args:
            args: begin 的参数列表 [expr1, expr2, ...]
            env: 当前环境

        Returns:
            最后一个表达式的值

        步骤：
        1. 依次求值每个表达式
        2. 返回最后一个表达式的值
        """
        # TODO: 实现
        pass

    def eval_application(self, elements: List[ASTNode], env: Environment) -> Any:
        """求值函数调用。

        TODO: 实现这个方法

        形式：(func arg1 arg2 ...)

        Args:
            elements: S-表达式的所有元素 [func, arg1, arg2, ...]
            env: 当前环境

        Returns:
            函数调用的结果

        步骤：
        1. 求值第一个元素，得到函数
        2. 求值其余元素，得到参数列表
        3. 调用函数

        注意：这里只处理内置函数（callable）。
        用户定义的函数（Closure）在模块 5 中处理。
        """
        # TODO: 实现
        pass

    def ast_to_value(self, node: ASTNode) -> Any:
        """将 AST 节点转换为值（用于 quote）。

        Args:
            node: AST 节点

        Returns:
            对应的值
        """
        if isinstance(node, Number):
            return node.value
        if isinstance(node, Boolean):
            return node.value
        if isinstance(node, Symbol):
            return node.name  # 符号变成字符串
        if isinstance(node, SExpression):
            return [self.ast_to_value(elem) for elem in node.elements]
        return node

    def run(self, source: str) -> Any:
        """解析并执行源代码。

        Args:
            source: 源代码字符串

        Returns:
            最后一个表达式的值
        """
        ast_nodes = parse(source)
        result = None
        for node in ast_nodes:
            result = self.eval(node, self.global_env)
        return result
