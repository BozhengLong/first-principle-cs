"""环境模型骨架代码。

你的任务是实现标记为 TODO 的方法。

运行测试：
    pytest learn/03-environment/test_skeleton.py -v
"""

from typing import Dict, Optional, Any


class Environment:
    """环境：存储变量绑定，支持词法作用域。

    环境是一个变量名到值的映射，加上一个指向父环境的引用。
    这种链式结构支持嵌套作用域和变量遮蔽。

    使用方法：
        # 创建全局环境
        global_env = Environment()
        global_env.define('x', 10)

        # 创建子环境（如函数调用时）
        local_env = Environment(parent=global_env)
        local_env.define('y', 20)

        # 查找变量
        local_env.get('x')  # 10（从父环境找到）
        local_env.get('y')  # 20（从当前环境找到）
    """

    def __init__(self, parent: Optional['Environment'] = None):
        """初始化环境。

        Args:
            parent: 父环境，用于实现词法作用域。
                   如果是全局环境，parent 为 None。
        """
        self.bindings: Dict[str, Any] = {}
        self.parent = parent

    def define(self, name: str, value: Any):
        """在当前环境中定义一个新变量。

        TODO: 实现这个方法

        Args:
            name: 变量名
            value: 变量值

        注意：
        - 这个方法总是在当前环境中创建绑定
        - 如果变量已存在，会被覆盖
        - 不会影响父环境

        提示：
        - 直接在 self.bindings 中设置键值对
        """
        # TODO: 实现
        pass

    def get(self, name: str) -> Any:
        """查找变量的值。

        TODO: 实现这个方法

        Args:
            name: 变量名

        Returns:
            变量的值

        Raises:
            NameError: 如果变量未定义

        查找规则：
        1. 先在当前环境的 bindings 中查找
        2. 如果找不到，递归在父环境中查找
        3. 如果到达顶层（parent 为 None）还找不到，抛出 NameError

        提示：
        - 使用 `in` 操作符检查键是否存在
        - 使用递归或循环向上查找
        """
        # TODO: 实现
        pass

    def set(self, name: str, value: Any):
        """修改一个已存在变量的值。

        TODO: 实现这个方法

        Args:
            name: 变量名
            value: 新值

        Raises:
            NameError: 如果变量未定义

        注意：
        - 这个方法修改已存在的变量，不创建新变量
        - 需要找到变量所在的环境，然后修改
        - 如果变量在父环境中，应该修改父环境

        提示：
        - 先检查当前环境是否有这个变量
        - 如果有，修改当前环境
        - 如果没有，递归在父环境中查找并修改
        - 如果到达顶层还找不到，抛出 NameError
        """
        # TODO: 实现
        pass

    def __repr__(self):
        """返回环境的字符串表示。"""
        return f"Environment({list(self.bindings.keys())})"

    def __contains__(self, name: str) -> bool:
        """检查变量是否在环境链中定义。

        支持 `'x' in env` 语法。
        """
        try:
            self.get(name)
            return True
        except NameError:
            return False
