# 失败模型

## 定义

**失败模型（Failure Model）**：系统可能遇到的所有失败场景的分类与描述。

## 为什么重要

在 AI 时代，我们可以快速生成"正常情况下能工作"的代码，但真正的挑战在于：

- 系统在什么情况下会失败？
- 失败后会发生什么？
- 如何测试失败场景？

明确失败模型是设计可靠系统的前提。

## 常见失败模型清单

### 单机失败模型

#### 1. 进程崩溃（Crash）

**描述**：进程突然终止，内存状态丢失。

**影响**：
- 未持久化的数据丢失
- 持有的锁被释放
- 网络连接断开

**测试方法**：
- 在关键操作中间调用 `exit()` 或 `kill -9`
- 检查重启后系统是否能恢复

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
```

#### 2. 磁盘损坏（Disk Corruption）

**描述**：磁盘扇区损坏，读取返回错误或错误数据。

**影响**：
- 数据丢失或损坏
- 文件系统元数据损坏

**测试方法**：
- 随机翻转磁盘上的某些位
- 模拟读取错误

**例子**：
```python
def test_disk_corruption():
    fs = FileSystem()
    fs.write("file.txt", "hello")
    # 模拟磁盘损坏
    corrupt_random_sector()
    # 不变量：文件系统应该检测到损坏并报错
    with pytest.raises(CorruptionError):
        fs.read("file.txt")
```

#### 3. 内存错误（Memory Error）

**描述**：内存分配失败、越界访问、悬空指针。

**影响**：
- 程序崩溃
- 数据损坏
- 安全漏洞

**测试方法**：
- 限制可用内存
- 使用内存检查工具（如 Valgrind、AddressSanitizer）

**例子**：
```python
def test_out_of_memory():
    db = Database()
    # 限制内存
    set_memory_limit(1MB)
    # 不变量：内存不足时应该优雅地失败
    with pytest.raises(OutOfMemoryError):
        db.write("key", "x" * 10MB)
```

### 网络失败模型

#### 4. 网络分区（Network Partition）

**描述**：网络被分割成多个互不连通的部分。

**影响**：
- 节点无法通信
- 可能出现"脑裂"（split-brain）

**测试方法**：
- 使用网络模拟工具（如 tc、iptables）阻断部分节点间的通信
- 检查系统是否能正确处理分区

**例子**：
```python
def test_network_partition():
    cluster = Cluster(nodes=3)
    # 模拟分区：节点 1 与节点 2、3 隔离
    partition([node1], [node2, node3])
    # 不变量：系统应该停止服务或选举新领导者
    assert cluster.is_available() == False or cluster.has_leader()
```

#### 5. 消息延迟（Message Delay）

**描述**：消息传输时间不确定，可能非常长。

**影响**：
- 超时
- 顺序错乱
- 活锁

**测试方法**：
- 随机延迟消息
- 检查系统是否能正确处理超时

**例子**：
```python
def test_message_delay():
    cluster = Cluster(nodes=3)
    # 随机延迟消息 0-1000ms
    set_message_delay(0, 1000)
    # 不变量：系统最终应该达成一致
    cluster.propose("value")
    eventually(lambda: cluster.get_value() == "value")
```

#### 6. 消息丢失（Message Loss）

**描述**：消息在传输过程中丢失。

**影响**：
- 请求未到达
- 响应未返回
- 需要重试机制

**测试方法**：
- 随机丢弃消息
- 检查系统是否能正确重试

**例子**：
```python
def test_message_loss():
    cluster = Cluster(nodes=3)
    # 随机丢弃 10% 的消息
    set_message_loss_rate(0.1)
    # 不变量：系统应该通过重试保证可靠性
    cluster.propose("value")
    eventually(lambda: cluster.get_value() == "value")
```

#### 7. 消息重复（Message Duplication）

**描述**：同一消息被传输多次。

**影响**：
- 重复执行操作
- 需要幂等性

**测试方法**：
- 随机重复消息
- 检查系统是否能正确去重

**例子**：
```python
def test_message_duplication():
    db = Database()
    # 随机重复消息
    set_message_duplication_rate(0.1)
    # 不变量：重复的写入不应该改变结果
    db.increment("counter")
    db.increment("counter")
    assert db.read("counter") == 2  # 不是 3 或 4
```

#### 8. 消息乱序（Message Reordering）

**描述**：消息到达的顺序与发送的顺序不同。

**影响**：
- 因果关系被破坏
- 需要序列号或向量时钟

**测试方法**：
- 随机重排消息顺序
- 检查系统是否能正确处理乱序

**例子**：
```python
def test_message_reordering():
    db = Database()
    # 随机重排消息
    set_message_reordering(True)
    # 不变量：最终一致性应该保证所有节点达成一致
    db.write("key", "value1")
    db.write("key", "value2")
    eventually(lambda: all(node.read("key") == "value2" for node in db.nodes))
```

### 分布式失败模型

#### 9. 时钟漂移（Clock Skew）

**描述**：不同节点的时钟不同步。

**影响**：
- 时间戳不可靠
- 超时判断错误
- 因果关系混乱

**测试方法**：
- 随机调整节点时钟
- 检查系统是否依赖时钟同步

**例子**：
```python
def test_clock_skew():
    cluster = Cluster(nodes=3)
    # 节点 1 的时钟快 1 小时
    set_clock_offset(node1, +3600)
    # 不变量：系统不应该依赖绝对时间
    cluster.propose("value")
    assert cluster.get_value() == "value"
```

#### 10. 拜占庭故障（Byzantine Failure）

**描述**：节点表现出任意的恶意行为。

**影响**：
- 发送错误消息
- 伪造签名
- 破坏一致性

**测试方法**：
- 让部分节点发送随机或恶意消息
- 检查系统是否能容忍 f < n/3 个拜占庭节点

**例子**：
```python
def test_byzantine_failure():
    cluster = ByzantineCluster(nodes=4)
    # 节点 1 发送恶意消息
    node1.set_byzantine(True)
    # 不变量：系统应该容忍 1 个拜占庭节点（f < n/3）
    cluster.propose("value")
    assert cluster.get_value() == "value"
```

## 如何选择失败模型

不同系统需要考虑不同的失败模型：

| 系统类型 | 需要考虑的失败模型 |
|---------|------------------|
| 单机数据库 | 进程崩溃、磁盘损坏、内存错误 |
| 分布式存储 | 网络分区、消息延迟、节点崩溃 |
| 共识协议 | 网络分区、消息丢失、时钟漂移 |
| 区块链 | 拜占庭故障、网络分区、时钟漂移 |

## 测试失败场景的工具

### 单机工具

- **Valgrind**：检测内存错误
- **AddressSanitizer**：检测越界访问
- **Fault Injection**：模拟磁盘错误、内存不足

### 分布式工具

- **Jepsen**：测试分布式系统的一致性
- **Chaos Monkey**：随机杀死节点
- **tc/iptables**：模拟网络故障

## 总结

- 明确失败模型是设计可靠系统的前提
- 不同系统需要考虑不同的失败场景
- 测试失败场景需要故障注入工具
- 失败模型应该在设计阶段就明确，而不是在出问题后才补救

---

下一步：阅读 [验证优先](./verification-first.md) 了解如何测试不变量与失败场景。
