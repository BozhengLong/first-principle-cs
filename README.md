# first-principles-cs

AI 时代的第一性原理计算机科学课程体系。

## 这是什么

这是一个基于第一性原理重建计算机科学基础设施的项目。我们不满足于仅仅"会用工具"，而是要理解这个世界是如何被搭建起来的——即使有一天只能靠我们和一个 AI，也能从零把它重建出来。

## 核心原则

我们遵循四大原则来构建和验证系统：

1. **接口与不变量** - 明确系统边界，定义必须始终成立的性质
2. **失败模型** - 识别所有可能的失败场景，设计容错机制
3. **验证优先** - 测试先行，用属性测试和故障注入验证不变量
4. **重构记录** - 持续简化实现，保持测试通过，记录设计演化

详见 [principles/](./principles/) 目录。

## 文档导航

- [WHY - 为什么做这个项目](./guide/WHY.md)
- [ROADMAP - 12 个月路线图](./guide/ROADMAP.md)
- [PROJECTS - 核心系统仓库清单](./guide/PROJECTS.md)
- [原则文档](./principles/) - 四大原则的详细说明
- [课程体系](./courses/) - 六大模块的学习路径

## 与 AI 协作

我们使用固定的协作流程来构建系统：

1. **规格先行** - 明确接口、不变量、失败模型
2. **生成实现** - AI 生成代码实现
3. **约束测试** - 验证不变量与失败场景
4. **重构压复杂度** - 简化实现，保持测试通过

详见 [principles/ai-collaboration-protocol.md](./principles/ai-collaboration-protocol.md)。

## 模板

我们提供统一的模板来保持项目一致性：

- [仓库 README 模板](./templates/repo-readme-template.md)
- [设计文档模板](./templates/design-doc-template.md)
- [实验模板](./templates/experiment-template.md)

## 许可证

本项目采用 MIT License，详见 [LICENSE](./LICENSE)。
