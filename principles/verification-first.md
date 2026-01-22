# 验证优先

## 定义

**验证优先（Verification First）**：在实现之前或同时设计测试，确保系统满足不变量。

## 为什么重要

在 AI 时代，我们可以快速生成代码，但如果没有测试，我们就无法：

- 判断代码是否正确
- 验证重构是否安全
- 诊断系统为什么失败

测试不是"事后补救"，而是"设计的一部分"。

## 测试金字塔

不同层次的测试有不同的目的：

```
        /\
       /  \  端到端测试（少量）
      /    \  - 测试整个系统的行为
     /------\  - 慢，但覆盖真实场景
    /        \
   /  集成测试 \ （中等数量）
  /   - 测试模块间的交互
 /    - 中等速度
/----------------\
/   单元测试      \ （大量）
/  - 测试单个函数或类
/ - 快，但覆盖范围小
/--------------------\
```

### 单元测试

**目的**：测试单个函数或类的行为。

**特点**：
- 快速（毫秒级）
- 隔离（不依赖外部资源）
- 大量（覆盖所有边界情况）

**例子**：
```python
def test_stack_push_pop():
    stack = Stack()
    stack.push(1)
    stack.push(2)
    assert stack.pop() == 2
    assert stack.pop() == 1
```

### 集成测试

**目的**：测试多个模块之间的交互。

**特点**：
- 中等速度（秒级）
- 依赖真实或模拟的外部资源
- 中等数量（覆盖主要交互路径）

**例子**：
```python
def test_database_transaction():
    db = Database()
    tx = db.begin_transaction()
    tx.write("key", "value")
    tx.commit()
    assert db.read("key") == "value"
```

### 端到端测试

**目的**：测试整个系统的行为。

**特点**：
- 慢（分钟级）
- 依赖真实环境
- 少量（覆盖关键用户场景）

**例子**：
```python
def test_user_signup_flow():
    client = HTTPClient()
    response = client.post("/signup", {"email": "test@example.com"})
    assert response.status == 200
    assert "Welcome" in response.body
```

## 属性测试（Property-Based Testing）

**目的**：生成随机输入，验证不变量在所有情况下都成立。

**特点**：
- 自动生成测试用例
- 覆盖边界情况
- 发现意外的 bug

**例子**：
```python
from hypothesis import given, strategies as st

@given(st.lists(st.integers()))
def test_sort_idempotent(lst):
    # 不变量：排序两次应该与排序一次结果相同
    assert sorted(sorted(lst)) == sorted(lst)

@given(st.lists(st.integers()))
def test_sort_preserves_length(lst):
    # 不变量：排序不应该改变列表长度
    assert len(sorted(lst)) == len(lst)
```

**常用属性**：
- **幂等性**：`f(f(x)) == f(x)`
- **可逆性**：`decode(encode(x)) == x`
- **交换律**：`f(x, y) == f(y, x)`
- **结合律**：`f(f(x, y), z) == f(x, f(y, z))`

## Fuzz 测试

**目的**：生成随机或畸形输入，发现崩溃或安全漏洞。

**特点**：
- 自动生成大量输入
- 覆盖异常情况
- 发现安全漏洞

**例子**：
```python
def test_parser_fuzz():
    for _ in range(10000):
        # 生成随机字符串
        input = random_string()
        try:
            parse(input)
        except ParseError:
            # 预期的错误
            pass
        except Exception as e:
            # 不应该崩溃
            pytest.fail(f"Unexpected crash: {e}")
```

**工具**：
- **AFL**：自动化 fuzz 测试工具
- **libFuzzer**：LLVM 的 fuzz 测试库
- **Hypothesis**：Python 的属性测试库

## 基准测试（Benchmark）

**目的**：测量性能，发现性能回归。

**特点**：
- 量化性能指标
- 对比不同实现
- 检测性能回归

**例子**：
```python
def benchmark_hash_table():
    ht = HashTable()
    # 插入 1M 个元素
    start = time.time()
    for i in range(1_000_000):
        ht.insert(i, i)
    elapsed = time.time() - start
    # 不变量：平均插入时间应该是 O(1)
    assert elapsed < 1.0  # 1 秒内完成
```

**指标**：
- **吞吐量**：每秒处理的请求数
- **延迟**：单个请求的响应时间
- **P99 延迟**：99% 的请求的响应时间

## 故障注入（Fault Injection）

**目的**：模拟失败场景，验证系统的容错能力。

**特点**：
- 测试失败场景
- 验证不变量在失败时仍然成立
- 发现边界情况的 bug

**例子**：
```python
def test_crash_recovery():
    db = Database()
    db.write("key", "value")
    # 模拟崩溃
    simulate_crash()
    # 重启
    db = Database()
    # 不变量：已提交的数据不应该丢失
    assert db.read("key") == "value"

def test_network_partition():
    cluster = Cluster(nodes=3)
    # 模拟网络分区
    partition([node1], [node2, node3])
    # 不变量：系统应该停止服务或选举新领导者
    assert cluster.is_available() == False or cluster.has_leader()
```

**工具**：
- **Jepsen**：测试分布式系统的一致性
- **Chaos Monkey**：随机杀死节点
- **tc/iptables**：模拟网络故障

## 最小可复现实验

**目的**：隔离问题，找到最小的复现步骤。

**步骤**：
1. 复现问题
2. 删除无关代码
3. 简化输入
4. 验证问题仍然存在
5. 重复 2-4 直到无法再简化

**例子**：
```python
# 原始问题：复杂的分布式系统崩溃
def test_complex_system_crash():
    cluster = Cluster(nodes=10)
    for i in range(1000):
        cluster.write(f"key{i}", f"value{i}")
    # 系统崩溃

# 简化后：最小复现
def test_minimal_crash():
    cluster = Cluster(nodes=2)
    cluster.write("key", "value")
    # 系统崩溃
```

## 测试策略

### 策略 1: 测试驱动开发（TDD）

1. 写测试（定义接口与不变量）
2. 运行测试（应该失败）
3. 写实现（让测试通过）
4. 重构（保持测试通过）

### 策略 2: 不变量驱动测试

1. 识别不变量
2. 为每个不变量写测试
3. 用属性测试验证不变量在所有情况下都成立
4. 用故障注入验证不变量在失败时仍然成立

### 策略 3: 失败场景驱动测试

1. 列出所有失败场景
2. 为每个失败场景写测试
3. 验证系统能正确处理失败
4. 验证不变量在失败时仍然成立

## 常见陷阱

### 陷阱 1: 只测试正常情况

**错误**：只测试"happy path"，不测试失败场景。

**正确**：测试所有失败场景，验证系统的容错能力。

### 陷阱 2: 测试实现细节

**错误**：测试内部实现，而不是接口与不变量。

**正确**：测试接口与不变量，允许实现自由变化。

### 陷阱 3: 测试覆盖率迷信

**错误**：追求 100% 代码覆盖率，但不测试不变量。

**正确**：优先测试不变量，代码覆盖率是副产品。

## 总结

- 测试是设计的一部分，不是事后补救
- 使用测试金字塔：大量单元测试，中等集成测试，少量端到端测试
- 使用属性测试验证不变量
- 使用 fuzz 测试发现崩溃
- 使用基准测试量化性能
- 使用故障注入测试失败场景
- 最小可复现实验是诊断问题的关键

---

下一步：阅读 [AI 协作协议](./ai-collaboration-protocol.md) 了解如何与 AI 高效协作。
