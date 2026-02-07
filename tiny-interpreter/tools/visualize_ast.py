#!/usr/bin/env python3
"""AST 可视化工具。

运行方式：
    python tools/visualize_ast.py "(+ 1 (* 2 3))"
    python tools/visualize_ast.py -f examples/factorial.lisp
"""

import sys
import os
import argparse

# 添加项目根目录到路径
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

from src.tiny_interpreter.lexer import Lexer
from src.tiny_interpreter.parser import Parser, Number, Boolean, Symbol, SExpression


def visualize_ast(node, indent=0, prefix="", is_last=True):
    """以树形结构可视化 AST。

    Args:
        node: AST 节点
        indent: 当前缩进级别
        prefix: 前缀字符串
        is_last: 是否是同级的最后一个节点
    """
    # 连接符
    connector = "└── " if is_last else "├── "

    # 节点表示
    if isinstance(node, Number):
        node_str = f"Number: {node.value}"
    elif isinstance(node, Boolean):
        node_str = f"Boolean: {node.value}"
    elif isinstance(node, Symbol):
        node_str = f"Symbol: {node.name}"
    elif isinstance(node, SExpression):
        if len(node.elements) > 0 and isinstance(node.elements[0], Symbol):
            node_str = f"SExp: ({node.elements[0].name} ...)"
        else:
            node_str = f"SExp: ({len(node.elements)} elements)"
    else:
        node_str = str(node)

    # 打印当前节点
    print(f"{prefix}{connector}{node_str}")

    # 递归打印子节点
    if isinstance(node, SExpression):
        # 更新前缀
        new_prefix = prefix + ("    " if is_last else "│   ")

        for i, child in enumerate(node.elements):
            is_last_child = (i == len(node.elements) - 1)
            visualize_ast(child, indent + 1, new_prefix, is_last_child)


def visualize_tokens(source):
    """可视化 Token 序列。"""
    lexer = Lexer(source)
    tokens = lexer.tokenize()

    print("\nToken 序列:")
    print("-" * 40)
    for i, token in enumerate(tokens):
        print(f"  {i:3d}: {token.type.name:10s} {repr(token.value):15s} ({token.line}:{token.column})")
    print()


def visualize(source, show_tokens=False):
    """可视化源代码的 AST。"""
    print(f"\n源代码: {source}")
    print("=" * 50)

    if show_tokens:
        visualize_tokens(source)

    # 解析
    lexer = Lexer(source)
    tokens = lexer.tokenize()
    parser = Parser(tokens)
    ast = parser.parse()

    # 可视化
    print("\nAST 结构:")
    print("-" * 40)

    for i, node in enumerate(ast):
        is_last = (i == len(ast) - 1)
        visualize_ast(node, is_last=is_last)

    print()


def main():
    """主函数。"""
    parser = argparse.ArgumentParser(
        description="AST 可视化工具",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  python visualize_ast.py "(+ 1 2)"
  python visualize_ast.py "(define square (lambda (x) (* x x)))"
  python visualize_ast.py -f examples/factorial.lisp
  python visualize_ast.py -t "(+ 1 2)"  # 同时显示 Token
        """
    )

    parser.add_argument(
        "source",
        nargs="?",
        help="要可视化的源代码"
    )

    parser.add_argument(
        "-f", "--file",
        help="从文件读取源代码"
    )

    parser.add_argument(
        "-t", "--tokens",
        action="store_true",
        help="同时显示 Token 序列"
    )

    args = parser.parse_args()

    # 获取源代码
    if args.file:
        with open(args.file, 'r') as f:
            source = f.read()
    elif args.source:
        source = args.source
    else:
        # 交互模式
        print("AST 可视化工具")
        print("输入表达式，按 Ctrl+D 退出")
        print()

        while True:
            try:
                source = input("> ")
                if source.strip():
                    visualize(source, args.tokens)
            except EOFError:
                print("\n再见！")
                break
            except Exception as e:
                print(f"错误: {e}")

        return

    # 可视化
    visualize(source, args.tokens)


if __name__ == "__main__":
    main()
