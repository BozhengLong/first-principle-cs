#!/usr/bin/env python3
"""交互式探索脚本 - 体验解释器的各个阶段。

运行方式：
    python learn/00-introduction/playground.py
"""

import sys
import os

# 添加项目根目录到路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from src.tiny_interpreter.lexer import Lexer
from src.tiny_interpreter.parser import Parser
from src.tiny_interpreter.evaluator import Evaluator


def print_header(title: str):
    """打印带边框的标题。"""
    print("\n" + "=" * 60)
    print(f"  {title}")
    print("=" * 60)


def print_section(title: str):
    """打印小节标题。"""
    print(f"\n--- {title} ---\n")


def explore_tokenization():
    """探索词法分析。"""
    print_header("探索 1：词法分析 (Tokenization)")

    source = "(+ 1 (* 2 3))"
    print(f"源代码: {source}")
    print_section("Token 序列")

    lexer = Lexer(source)
    tokens = lexer.tokenize()

    for i, token in enumerate(tokens):
        print(f"  {i}: {token}")

    print("\n思考：")
    print("  - 每个 token 包含什么信息？")
    print("  - 为什么需要记录行号和列号？")
    print("  - 空格去哪了？")


def explore_parsing():
    """探索语法分析。"""
    print_header("探索 2：语法分析 (Parsing)")

    source = "(+ 1 (* 2 3))"
    print(f"源代码: {source}")
    print_section("抽象语法树 (AST)")

    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()

    def print_ast(node, indent=0):
        """递归打印 AST。"""
        prefix = "  " * indent
        node_type = type(node).__name__

        if hasattr(node, 'value'):
            print(f"{prefix}{node_type}: {node.value}")
        elif hasattr(node, 'name'):
            print(f"{prefix}{node_type}: {node.name}")
        elif hasattr(node, 'elements'):
            print(f"{prefix}{node_type}:")
            for elem in node.elements:
                print_ast(elem, indent + 1)
        else:
            print(f"{prefix}{node_type}")

    for node in ast:
        print_ast(node)

    print("\n思考：")
    print("  - AST 的结构和源代码有什么关系？")
    print("  - 为什么叫'树'？")
    print("  - 嵌套的括号变成了什么？")


def explore_evaluation():
    """探索求值。"""
    print_header("探索 3：求值 (Evaluation)")

    evaluator = Evaluator()

    examples = [
        ("简单算术", "(+ 1 2)"),
        ("嵌套表达式", "(+ 1 (* 2 3))"),
        ("定义变量", "(begin (define x 10) (+ x 5))"),
        ("定义函数", "(begin (define square (lambda (n) (* n n))) (square 5))"),
    ]

    for name, source in examples:
        print_section(name)
        print(f"  源代码: {source}")
        result = evaluator.run(source)
        print(f"  结果: {result}")


def explore_closure():
    """探索闭包 - 最神奇的部分！"""
    print_header("探索 4：闭包的魔力")

    evaluator = Evaluator()

    print("让我们一步步揭开闭包的秘密...\n")

    # 步骤 1：定义 make-adder
    print_section("步骤 1：定义 make-adder")
    code1 = """
(define make-adder
  (lambda (x)
    (lambda (y) (+ x y))))
"""
    print(f"代码:{code1}")
    evaluator.run(code1)
    print("make-adder 已定义。它是一个返回函数的函数。")

    # 步骤 2：创建 add5
    print_section("步骤 2：创建 add5")
    code2 = "(define add5 (make-adder 5))"
    print(f"代码: {code2}")
    evaluator.run(code2)
    print("add5 现在是一个函数。")
    print("问题：make-adder 已经执行完了，x=5 存在哪里？")

    # 步骤 3：调用 add5
    print_section("步骤 3：调用 add5")
    code3 = "(add5 3)"
    print(f"代码: {code3}")
    result = evaluator.run(code3)
    print(f"结果: {result}")
    print("\n答案揭晓：")
    print("  add5 是一个闭包，它'记住'了创建时的环境（x=5）")
    print("  当调用 (add5 3) 时，y=3，而 x 从闭包的环境中获取")
    print("  所以 (+ x y) = (+ 5 3) = 8")

    # 步骤 4：创建另一个 adder
    print_section("步骤 4：创建 add10")
    code4 = "(define add10 (make-adder 10))"
    print(f"代码: {code4}")
    evaluator.run(code4)

    result5 = evaluator.run("(add5 1)")
    result10 = evaluator.run("(add10 1)")
    print(f"\n(add5 1) = {result5}")
    print(f"(add10 1) = {result10}")
    print("\nadd5 和 add10 各自记住了自己的 x 值！")
    print("这就是闭包的威力：每个闭包都有自己独立的环境。")


def main():
    """主函数。"""
    print("\n" + "=" * 60)
    print("  欢迎来到 Tiny Interpreter 交互式探索！")
    print("=" * 60)
    print("\n这个脚本将带你体验解释器的各个阶段。")
    print("每个探索都会展示一些代码和结果，并提出思考问题。")

    input("\n按 Enter 开始探索...")

    explore_tokenization()
    input("\n按 Enter 继续...")

    explore_parsing()
    input("\n按 Enter 继续...")

    explore_evaluation()
    input("\n按 Enter 继续...")

    explore_closure()

    print_header("探索完成！")
    print("\n现在你已经看到了解释器的各个阶段：")
    print("  1. 词法分析：字符串 → Token 序列")
    print("  2. 语法分析：Token 序列 → AST")
    print("  3. 求值：AST → 结果")
    print("\n接下来，让我们动手实现它们！")
    print("\n[进入模块 1：词法分析 →](../01-lexer/README.md)")
    print()


if __name__ == "__main__":
    main()
