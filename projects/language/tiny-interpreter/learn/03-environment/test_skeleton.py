"""环境模型测试。

运行测试：
    pytest learn/03-environment/test_skeleton.py -v
"""

import pytest
from skeleton import Environment


class TestBasicOperations:
    """基本操作测试。"""

    def test_define_and_get(self):
        """测试定义和获取变量。"""
        env = Environment()
        env.define('x', 10)

        assert env.get('x') == 10

    def test_define_multiple(self):
        """测试定义多个变量。"""
        env = Environment()
        env.define('x', 10)
        env.define('y', 20)
        env.define('z', 30)

        assert env.get('x') == 10
        assert env.get('y') == 20
        assert env.get('z') == 30

    def test_define_overwrites(self):
        """测试重复定义会覆盖。"""
        env = Environment()
        env.define('x', 10)
        env.define('x', 20)

        assert env.get('x') == 20

    def test_get_undefined_raises(self):
        """测试获取未定义变量抛出错误。"""
        env = Environment()

        with pytest.raises(NameError):
            env.get('x')


class TestParentEnvironment:
    """父环境测试。"""

    def test_get_from_parent(self):
        """测试从父环境获取变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)

        assert child.get('x') == 10

    def test_child_shadows_parent(self):
        """测试子环境遮蔽父环境变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)
        child.define('x', 20)

        assert child.get('x') == 20
        assert parent.get('x') == 10  # 父环境不受影响

    def test_deep_nesting(self):
        """测试深层嵌套环境。"""
        env1 = Environment()
        env1.define('a', 1)

        env2 = Environment(parent=env1)
        env2.define('b', 2)

        env3 = Environment(parent=env2)
        env3.define('c', 3)

        # env3 可以访问所有层级的变量
        assert env3.get('a') == 1
        assert env3.get('b') == 2
        assert env3.get('c') == 3

    def test_undefined_in_chain(self):
        """测试在整个环境链中都未定义的变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)
        child.define('y', 20)

        with pytest.raises(NameError):
            child.get('z')


class TestSetOperation:
    """set 操作测试。"""

    def test_set_local(self):
        """测试修改当前环境的变量。"""
        env = Environment()
        env.define('x', 10)
        env.set('x', 20)

        assert env.get('x') == 20

    def test_set_in_parent(self):
        """测试修改父环境的变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)
        child.set('x', 20)

        # 父环境的变量被修改
        assert parent.get('x') == 20
        assert child.get('x') == 20

    def test_set_undefined_raises(self):
        """测试修改未定义变量抛出错误。"""
        env = Environment()

        with pytest.raises(NameError):
            env.set('x', 10)

    def test_set_does_not_create(self):
        """测试 set 不会创建新变量。"""
        parent = Environment()

        child = Environment(parent=parent)

        with pytest.raises(NameError):
            child.set('x', 10)


class TestContains:
    """__contains__ 测试。"""

    def test_contains_local(self):
        """测试检查当前环境的变量。"""
        env = Environment()
        env.define('x', 10)

        assert 'x' in env
        assert 'y' not in env

    def test_contains_parent(self):
        """测试检查父环境的变量。"""
        parent = Environment()
        parent.define('x', 10)

        child = Environment(parent=parent)

        assert 'x' in child


class TestRepr:
    """__repr__ 测试。"""

    def test_repr(self):
        """测试字符串表示。"""
        env = Environment()
        env.define('x', 10)
        env.define('y', 20)

        repr_str = repr(env)
        assert 'Environment' in repr_str
        assert 'x' in repr_str or 'y' in repr_str


class TestRealWorldScenarios:
    """真实场景测试。"""

    def test_function_scope(self):
        """模拟函数调用的作用域。"""
        # 全局环境
        global_env = Environment()
        global_env.define('x', 10)

        # 函数调用创建新环境
        func_env = Environment(parent=global_env)
        func_env.define('x', 5)  # 参数 x 遮蔽全局 x
        func_env.define('y', 3)

        # 函数内部
        assert func_env.get('x') == 5  # 使用参数
        assert func_env.get('y') == 3

        # 全局环境不受影响
        assert global_env.get('x') == 10

    def test_nested_function_scope(self):
        """模拟嵌套函数调用。"""
        # 全局
        global_env = Environment()
        global_env.define('a', 1)

        # 外层函数
        outer_env = Environment(parent=global_env)
        outer_env.define('b', 2)

        # 内层函数
        inner_env = Environment(parent=outer_env)
        inner_env.define('c', 3)

        # 内层函数可以访问所有外层变量
        assert inner_env.get('a') == 1
        assert inner_env.get('b') == 2
        assert inner_env.get('c') == 3

    def test_closure_scenario(self):
        """模拟闭包场景。"""
        # 全局环境
        global_env = Environment()

        # make-adder 的环境
        make_adder_env = Environment(parent=global_env)
        make_adder_env.define('x', 5)

        # 返回的闭包捕获 make_adder_env
        # 当闭包被调用时，创建新环境，parent 是 make_adder_env
        closure_call_env = Environment(parent=make_adder_env)
        closure_call_env.define('y', 3)

        # 闭包内部可以访问 x（来自 make_adder_env）和 y（参数）
        assert closure_call_env.get('x') == 5
        assert closure_call_env.get('y') == 3
