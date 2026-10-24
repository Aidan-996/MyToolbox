# MyBox Linux 版

Windows 版（GUI，`MyBox/` 目录）的 Linux 命令行对应版本。**与 Windows 版完全独立、互不影响**，
但共用同一套数据文件：`shared/prompts/`（8 个提示词模板）和 `shared/skills/`（40 个安全/逆向 SKILL.md）。

> ⚠️ 同样的法律边界：仅供学习和**授权**安全研究使用；商业化被禁止。

## 功能

- 探测本机已安装的 AI 编程工具（Claude Code / Codex / Gemini CLI / OpenCode 等 13 个）
- 一键把提示词写入工具的全局配置文件（如 `~/.claude/CLAUDE.md`），首次注入前自动备份
- 一键卸载恢复原文件；Skills 安装/卸载只动自己装过的文件
- 从 GitHub 同步新提示词模板（`ua sync`）

## 编译与安装

```bash
make                # 编译出 ./ua（需要 g++，C++17）
sudo make install   # 安装到 /usr/local/bin，数据到 /usr/local/share/ua
```

不安装直接跑也可以：在仓库里 `./linux/ua list`，数据目录会自动找到 `../shared/`。

数据目录查找顺序（第一个存在的生效）：

1. `$UA_DATA`（环境变量，最优先）
2. `~/.local/share/ua/`
3. `<exe>/../share/ua/`（make install 的位置）
4. `<exe>/data/`
5. `<exe>/../shared/`（仓库布局）

## 使用

```bash
ua list                              # 工具列表与注入状态
ua install claude                    # 注入（多模板时会列出可选项）
ua install codex Ua-standard-pro.md  # 指定提示词文件
ua uninstall claude                  # 恢复原文件
ua skills list claude                # 查看技能安装情况
ua skills install claude             # 安装全部 40 个技能到 ~/.claude/skills/
ua skills install claude radare2     # 只装一个
ua skills uninstall claude           # 全部卸载
ua sync                              # 从 GitHub 拉取新提示词模板
```

## 状态与备份位置

- 状态：`~/.local/share/ua/state/`（可用 `UA_DATA` 重定向）
- 备份：`~/.local/share/ua/backups/<tool>/`，保留最近 200 份

## CI

`.github/workflows/linux.yml` 在每次 `linux/` 有改动时于 ubuntu-latest 上编译，
并用假 `$HOME` 做注入/恢复/Skills 的端到端冒烟测试。
