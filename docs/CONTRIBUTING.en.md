[中文](CONTRIBUTING.md) | [English](CONTRIBUTING.en.md)

---

# Contributing Guide

Thank you for your interest in the first-principles-cs project! This document explains how to contribute, with a focus on translation and internationalization (i18n).

## Translation Conventions

### File Naming

- **Default language**: Chinese (unsuffixed), e.g., `README.md`
- **English**: `.en.md` suffix, e.g., `README.en.md`
- **Other languages**: Use ISO 639-1 language codes, e.g.:
  - Japanese: `.ja.md`
  - Korean: `.ko.md`
  - French: `.fr.md`
  - Spanish: `.es.md`

### Language Switcher

Every translated file must include a language switcher at the top:

```markdown
[中文](README.md) | [English](README.en.md)

---
```

If more languages are available:

```markdown
[中文](README.md) | [English](README.en.md) | [日本語](README.ja.md)

---
```

### Link Rules

- Language switcher links use **relative paths** (files in the same directory)
- Internal cross-references should point to the **same-language version**
  - In Chinese docs: `[项目清单](./projects.md)`
  - In English docs: `[Project List](./projects.en.md)`

## How to Add a New Language

1. **Choose a language code**: Use the ISO 639-1 standard (e.g., `ja`, `ko`, `fr`)

2. **Create translation files**: Create the corresponding language version for each file
   ```
   README.md → README.ja.md
   docs/why.md → docs/why.ja.md
   ```

3. **Add language switchers**: Add the switcher to the new file and update all existing language versions

4. **Update cross-references**: Ensure internal links point to the same-language version

5. **Submit a PR**: Describe the language and scope in the PR description

## Translation Priority

We recommend translating in this order:

1. `README.md` (root)
2. `docs/why.md`
3. `docs/roadmap.md`
4. `docs/projects.md`
5. Individual project `README.md` files

## Staleness Tracking

When the original Chinese document is updated, translations may become stale. We track this as follows:

- Translation PRs should record the commit hash of the source at translation time
- If a translation is found to be outdated, file an issue with the `translation-outdated` label
- Periodically check for differences between translations and their source documents

## General Contribution Guidelines

### Filing Issues

- Use a clear title describing the problem
- Provide reproduction steps (if applicable)
- Tag the relevant project module

### Submitting PRs

- One PR, one concern
- Provide a clear description
- Ensure all links are valid
- Tag translation PRs with the `i18n` label

## Code of Conduct

Please be kind and respectful. We welcome contributors of all backgrounds.
