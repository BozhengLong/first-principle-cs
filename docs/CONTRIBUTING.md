[中文](CONTRIBUTING.md) | [English](CONTRIBUTING.en.md)

---

# 贡献指南

感谢你对 first-principles-cs 项目的关注！本文档说明如何为项目贡献内容，特别是翻译和国际化（i18n）相关的贡献。

## 翻译约定

### 文件命名

- **默认语言**：中文（无后缀），例如 `README.md`
- **英文**：`.en.md` 后缀，例如 `README.en.md`
- **其他语言**：使用 ISO 639-1 语言代码，例如：
  - 日语：`.ja.md`
  - 韩语：`.ko.md`
  - 法语：`.fr.md`
  - 西班牙语：`.es.md`

### 语言切换器

每个翻译文件的顶部必须包含语言切换器：

```markdown
[中文](README.md) | [English](README.en.md)

---
```

如果有更多语言：

```markdown
[中文](README.md) | [English](README.en.md) | [日本語](README.ja.md)

---
```

### 链接规则

- 语言切换器中的链接使用**相对路径**（同目录下的文件）
- 文档内部的交叉引用应指向**同语言版本**的文件
  - 中文文档中：`[项目清单](./projects.md)`
  - 英文文档中：`[Project List](./projects.en.md)`

## 如何添加新语言

1. **选择语言代码**：使用 ISO 639-1 标准代码（如 `ja`、`ko`、`fr`）

2. **创建翻译文件**：为每个需要翻译的文件创建对应的语言版本
   ```
   README.md → README.ja.md
   docs/why.md → docs/why.ja.md
   ```

3. **添加语言切换器**：在新文件顶部添加切换器，并更新所有已有语言版本的切换器

4. **更新交叉引用**：确保文档内部链接指向同语言版本

5. **提交 PR**：在 PR 描述中说明翻译的语言和范围

## 翻译优先级

建议按以下顺序翻译：

1. `README.md`（根目录）
2. `docs/why.md`
3. `docs/roadmap.md`
4. `docs/projects.md`
5. 各项目的 `README.md`

## 翻译过时追踪

当原始中文文档更新后，对应的翻译可能会过时。我们使用以下方式追踪：

- 翻译 PR 应记录翻译时原文的 commit hash
- 如果发现翻译过时，请提交 issue 标记为 `translation-outdated`
- 定期检查翻译与原文的差异

## 通用贡献指南

### 提交 Issue

- 使用清晰的标题描述问题
- 提供复现步骤（如适用）
- 标注相关的项目模块

### 提交 PR

- 一个 PR 只做一件事
- 提供清晰的描述
- 确保所有链接有效
- 翻译 PR 请标注 `i18n` 标签

## 行为准则

请保持友善和尊重。我们欢迎所有背景的贡献者。
