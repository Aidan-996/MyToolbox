# MyBox 命令行版（Linux / macOS）

Windows 图形版（`MyBox/` 目录）的跨平台命令行对应版本，同一套代码支持 **Linux 与 macOS**。
**与 Windows 版完全独立、互不影响**，共用同一套数据文件：`shared/prompts/`（8 个提示词模板）
和 `shared/skills/`（40 个安全/逆向 SKILL.md）。

> ⚠️ 同样的法律边界：仅供学习和**授权**安全研究使用；商业化被禁止。

## 功能

- 探测本机已安装的 AI 编程工具（Claude Code / Codex / Gemini CLI / OpenCode 等 13 个）
- 一键把提示词写入工具的全局配置文件（如 `~/.claude/CLAUDE.md`，各工具在 macOS/Linux 路径一致），首次注入前自动备份
- 一键卸载恢复原文件；Skills 安装/卸载只动自己装过的文件
- 从 GitHub 同步新提示词模板（`mybox sync`）

## 编译与安装

**一键安装（推荐，免 git / 免编译 / 免 root）**

```bash
curl -fsSL https://raw.githubusercontent.com/Aidan-996/MyToolbox/main/cli/install.sh | sh
```

装到 `~/.local/bin/mybox`（数据在平台惯例目录）；要装到 `/usr/local` 就下载脚本后 `sudo sh install.sh --system`。

**源码编译**

```bash
make                    # 编译出 ./mybox（g++ 或 clang++，C++17）
sudo make install       # 安装到 /usr/local/bin/mybox，数据到 /usr/local/share/mybox
```

macOS 构建通用二进制（Apple Silicon + Intel）：

```bash
make CXXFLAGS="-std=c++17 -O2 -Wall -Wextra -arch arm64 -arch x86_64"
```

也可以直接从 [Releases](https://github.com/Aidan-996/MyToolbox/releases) 下载现成的
`mybox-macos-universal` / `mybox-linux-x64`，或免安装跑：仓库里 `./cli/mybox list`，
数据目录会自动找到 `../shared/`。

## 数据目录查找顺序（第一个存在的生效）

1. `$MYBOX_DATA`（环境变量，最优先；兼容旧变量 `$UA_DATA`）
2. macOS：`~/Library/Application Support/MyBox/`；Linux：`~/.local/share/mybox/`
3. `<exe>/../share/mybox/`（make install 的位置）
4. `<exe>/data/`
5. `<exe>/../shared/`（仓库布局）

状态与备份存放在数据目录下的 `state/` 与 `backups/`（每个工具保留最近 200 份备份）。

## 使用

```bash
mybox list                              # 工具列表与注入状态
mybox install claude                    # 注入提示词（多模板时会列出可选项）
mybox install codex Ua-standard-pro.md  # 指定提示词文件
mybox uninstall claude                  # 恢复原文件
mybox skills list claude                # 查看技能安装情况
mybox skills install claude             # 安装全部 40 个技能到 ~/.claude/skills/
mybox skills install claude radare2     # 只装一个
mybox skills uninstall claude           # 全部卸载
mybox sync                              # 从 GitHub 同步新提示词模板
```

## CI

`.github/workflows/cli.yml` 在每次 `cli/` 有改动时于 **ubuntu + macOS** 上编译
（macOS 产通用二进制），并用假 `$HOME` 做注入/恢复/Skills 的端到端冒烟测试。
