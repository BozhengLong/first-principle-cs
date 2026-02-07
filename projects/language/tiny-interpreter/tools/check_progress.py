#!/usr/bin/env python3
"""检查学习进度。

运行方式：
    python tools/check_progress.py
"""

import os
import sys
import subprocess
from pathlib import Path

# 项目根目录
ROOT = Path(__file__).parent.parent

# 学习模块
MODULES = [
    ("00-introduction", "引言", None),
    ("01-lexer", "词法分析", "test_skeleton.py"),
    ("02-parser", "语法分析", "test_skeleton.py"),
    ("03-environment", "环境模型", "test_skeleton.py"),
    ("04-evaluator-basic", "基础求值", "test_skeleton.py"),
    ("05-evaluator-lambda", "Lambda 与闭包", "test_skeleton.py"),
    ("06-putting-together", "整合与扩展", None),
]


def check_module(module_dir: str, test_file: str) -> tuple:
    """检查模块的完成状态。

    Returns:
        (status, passed, total)
        status: 'not_started', 'in_progress', 'completed'
    """
    module_path = ROOT / "learn" / module_dir

    if not module_path.exists():
        return ("not_found", 0, 0)

    if test_file is None:
        # 没有测试的模块，检查是否有 skeleton.py
        skeleton = module_path / "skeleton.py"
        if skeleton.exists():
            return ("in_progress", 0, 0)
        return ("completed", 0, 0)

    test_path = module_path / test_file
    if not test_path.exists():
        return ("not_found", 0, 0)

    # 运行测试
    try:
        result = subprocess.run(
            [sys.executable, "-m", "pytest", str(test_path), "-v", "--tb=no", "-q"],
            capture_output=True,
            text=True,
            cwd=str(module_path),
            timeout=30
        )

        output = result.stdout + result.stderr

        # 解析测试结果
        # 查找类似 "5 passed" 或 "3 passed, 2 failed" 的行
        passed = 0
        failed = 0
        for line in output.split('\n'):
            if 'passed' in line or 'failed' in line:
                import re
                passed_match = re.search(r'(\d+) passed', line)
                failed_match = re.search(r'(\d+) failed', line)
                if passed_match:
                    passed = int(passed_match.group(1))
                if failed_match:
                    failed = int(failed_match.group(1))

        total = passed + failed

        if total == 0:
            return ("not_started", 0, 0)
        elif failed == 0:
            return ("completed", passed, total)
        else:
            return ("in_progress", passed, total)

    except subprocess.TimeoutExpired:
        return ("timeout", 0, 0)
    except Exception as e:
        return ("error", 0, 0)


def print_progress():
    """打印学习进度。"""
    print("\n" + "=" * 60)
    print("  Tiny Interpreter 学习进度")
    print("=" * 60 + "\n")

    status_icons = {
        "not_found": "❓",
        "not_started": "⬜",
        "in_progress": "🔶",
        "completed": "✅",
        "timeout": "⏱️",
        "error": "❌",
    }

    completed_count = 0
    total_modules = len(MODULES)

    for module_dir, module_name, test_file in MODULES:
        status, passed, total = check_module(module_dir, test_file)

        icon = status_icons.get(status, "❓")

        if status == "completed":
            completed_count += 1
            if total > 0:
                print(f"  {icon} {module_name} ({passed}/{total} 测试通过)")
            else:
                print(f"  {icon} {module_name}")
        elif status == "in_progress":
            print(f"  {icon} {module_name} ({passed}/{total} 测试通过)")
        elif status == "not_started":
            print(f"  {icon} {module_name} (未开始)")
        else:
            print(f"  {icon} {module_name} ({status})")

    print("\n" + "-" * 60)
    print(f"  总进度: {completed_count}/{total_modules} 模块完成")

    if completed_count == total_modules:
        print("\n  🎉 恭喜！你已完成所有模块！")
    elif completed_count > 0:
        next_module = MODULES[completed_count]
        print(f"\n  📚 下一步: {next_module[1]}")
        print(f"     cd learn/{next_module[0]}")
    else:
        print("\n  📚 开始学习:")
        print("     cd learn/00-introduction")
        print("     python playground.py")

    print()


def main():
    """主函数。"""
    print_progress()


if __name__ == "__main__":
    main()
