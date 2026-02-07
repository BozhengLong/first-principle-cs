# AI 协作协议

## 定义

**AI 协作协议（AI Collaboration Protocol）**：与 AI 协作构建系统的固定流程，确保人类保持对系统的理解与控制。

## 为什么需要协议

在 AI 时代，我们可以快速生成大量代码，但这也带来了新的风险：

- **风险 1**：代码能运行，但我们不知道它保证了什么
- **风险 2**：测试能通过，但我们不知道它测试了什么
- **风险 3**：系统能工作，但我们不知道它在什么情况下会失败

协议的目的是确保：人类负责思考，AI 负责执行，人类验证结果。

## 四阶段协作流程

```
规格先行 → 生成实现 → 约束测试 → 重构压复杂度
   ↑                                      ↓
   └──────────────────────────────────────┘
              (循环迭代)
```

### 阶段 1: 规格先行（Specification First）

**目标**：明确系统的接口、不变量、失败模型。

**人类负责**：
- 定义接口（输入、输出、副作用）
- 识别不变量（什么必须始终成立）
- 列出失败模型（什么可能出错）
- 设计测试策略（如何验证不变量）

**AI 负责**：
- 帮助澄清模糊的需求
- 提供类似系统的参考
- 指出潜在的边界情况

**输出**：
- 接口定义文档
- 不变量清单
- 失败模型清单
- 测试计划

**验收标准**：
- [ ] 接口定义清晰，没有歧义
- [ ] 不变量可验证（能写出测试）
- [ ] 失败模型完整（覆盖所有可能的失败场景）
- [ ] 测试计划可执行（明确测试方法与工具）

**例子**：
```markdown
## 接口定义

```python
class KeyValueStore:
    def get(self, key: str) -> Optional[str]:
        """获取键对应的值，如果键不存在返回 None"""
        pass

    def put(self, key: str, value: str) -> None:
        """设置键对应的值"""
        pass

    def delete(self, key: str) -> None:
        """删除键"""
        pass
```

## 不变量

1. **持久化**：`put()` 返回后，即使进程崩溃，数据也不会丢失
2. **一致性**：`get()` 返回最近一次 `put()` 的值
3. **原子性**：`put()` 要么完全成功，要么完全失败

## 失败模型

1. 进程崩溃
2. 磁盘损坏
3. 内存不足

## 测试计划

1. 单元测试：测试正常的 get/put/delete
2. 故障注入：模拟崩溃，验证持久化
3. 属性测试：验证一致性
```

### 阶段 2: 生成实现（Generate Implementation）

**目标**：让 AI 生成满足规格的实现。

**人类负责**：
- 提供规格文档
- 选择实现策略（如使用 LSM-tree 还是 B-tree）
- 审查生成的代码

**AI 负责**：
- 生成实现代码
- 生成测试代码
- 生成文档

**输出**：
- 实现代码
- 测试代码
- 实现文档（解释设计决策）

**验收标准**：
- [ ] 代码实现了所有接口
- [ ] 代码风格一致
- [ ] 有单元测试覆盖所有接口
- [ ] 有文档解释关键设计决策

**例子**：
```python
# 人类提供的提示
"""
请实现一个简单的键值存储，使用以下策略：
- 内存中使用哈希表
- 持久化使用追加日志（append-only log）
- 崩溃恢复时重放日志
"""

# AI 生成的实现
class KeyValueStore:
    def __init__(self, log_path: str):
        self.data = {}
        self.log = open(log_path, "a+")
        self._replay_log()

    def get(self, key: str) -> Optional[str]:
        return self.data.get(key)

    def put(self, key: str, value: str) -> None:
        self.data[key] = value
        self.log.write(f"PUT {key} {value}\n")
        self.log.flush()

    def _replay_log(self):
        self.log.seek(0)
        for line in self.log:
            op, key, value = line.strip().split()
            if op == "PUT":
                self.data[key] = value
```

### 阶段 3: 约束测试（Constraint Testing）

**目标**：验证实现满足不变量，能正确处理失败场景。

**人类负责**：
- 设计测试用例
- 审查测试结果
- 诊断失败原因

**AI 负责**：
- 生成测试代码
- 运行测试
- 报告测试结果

**输出**：
- 测试代码
- 测试报告
- Bug 修复（如果测试失败）

**验收标准**：
- [ ] 所有不变量都有对应的测试
- [ ] 所有失败场景都有对应的测试
- [ ] 所有测试都通过
- [ ] 测试覆盖率 > 80%

**例子**：
```python
# 人类设计的测试用例
"""
请为 KeyValueStore 生成以下测试：
1. 测试持久化：put 后崩溃，重启后 get 应该返回相同值
2. 测试一致性：put 后立即 get 应该返回相同值
3. 测试原子性：put 过程中崩溃，重启后数据应该完整或不存在
"""

# AI 生成的测试
def test_persistence():
    store = KeyValueStore("test.log")
    store.put("key", "value")
    # 模拟崩溃
    del store
    # 重启
    store = KeyValueStore("test.log")
    assert store.get("key") == "value"

def test_consistency():
    store = KeyValueStore("test.log")
    store.put("key", "value")
    assert store.get("key") == "value"

def test_atomicity():
    store = KeyValueStore("test.log")
    # 模拟 put 过程中崩溃
    with simulate_crash_during(store.put, "key", "value"):
        pass
    # 重启
    store = KeyValueStore("test.log")
    # 数据应该完整或不存在
    value = store.get("key")
    assert value in [None, "value"]
```

### 阶段 4: 重构压复杂度（Refactor to Reduce Complexity）

**目标**：简化实现，消除不必要的复杂度，保持测试通过。

**人类负责**：
- 识别复杂的代码
- 设计简化方案
- 审查重构结果

**AI 负责**：
- 执行重构
- 运行测试
- 更新文档

**输出**：
- 重构后的代码
- 测试报告（确保测试仍然通过）
- 重构文档（解释为什么重构）

**验收标准**：
- [ ] 代码更简洁（行数减少或圈复杂度降低）
- [ ] 所有测试仍然通过
- [ ] 性能没有明显下降
- [ ] 有文档解释重构理由

**例子**：
```python
# 人类识别的问题
"""
_replay_log() 方法太长，包含了解析和执行两个职责。
请重构：
1. 提取 _parse_log_line() 方法
2. 提取 _execute_operation() 方法
"""

# AI 重构后的代码
class KeyValueStore:
    def _replay_log(self):
        self.log.seek(0)
        for line in self.log:
            op = self._parse_log_line(line)
            self._execute_operation(op)

    def _parse_log_line(self, line: str) -> tuple:
        parts = line.strip().split()
        return (parts[0], parts[1], parts[2])

    def _execute_operation(self, op: tuple):
        cmd, key, value = op
        if cmd == "PUT":
            self.data[key] = value
```

## 循环迭代

完成四个阶段后，回到阶段 1，开始下一轮迭代：

1. 发现新的不变量或失败场景
2. 生成新的实现或修复 bug
3. 添加新的测试
4. 重构以保持代码简洁

## 关键原则

### 原则 1: 人类负责思考，AI 负责执行

- **人类**：定义问题、设计抽象、识别不变量
- **AI**：生成代码、运行测试、执行重构

### 原则 2: 规格先行，实现在后

- 先明确"要做什么"，再考虑"怎么做"
- 规格是验证的基础

### 原则 3: 测试是设计的一部分

- 测试不是事后补救，而是设计的一部分
- 不变量必须可验证

### 原则 4: 持续简化

- 复杂度是技术债务
- 重构是还债的过程

## 常见陷阱

### 陷阱 1: 跳过规格阶段

**错误**：直接让 AI 生成代码，不明确不变量。

**后果**：无法判断代码是否正确，无法验证重构是否安全。

**正确**：先明确规格，再生成实现。

### 陷阱 2: 盲目信任 AI

**错误**：不审查 AI 生成的代码，直接使用。

**后果**：可能包含 bug、安全漏洞、性能问题。

**正确**：审查代码，运行测试，验证不变量。

### 陷阱 3: 测试覆盖率迷信

**错误**：追求 100% 代码覆盖率，但不测试不变量。

**后果**：测试通过，但系统仍然有 bug。

**正确**：优先测试不变量，代码覆盖率是副产品。

### 陷阱 4: 过早优化

**错误**：在实现阶段就追求性能优化。

**后果**：代码复杂，难以维护。

**正确**：先实现正确性，再优化性能。

## 总结

- AI 协作的关键是明确分工：人类负责思考，AI 负责执行
- 四阶段流程：规格先行 → 生成实现 → 约束测试 → 重构压复杂度
- 规格是验证的基础，测试是设计的一部分
- 持续简化，消除不必要的复杂度

---

下一步：查看 [../templates/](../templates/) 了解如何使用模板来标准化文档。
