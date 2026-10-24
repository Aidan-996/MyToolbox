# MyToolbox

[![Release](https://img.shields.io/github/v/release/Aidan-996/MyToolbox?style=flat-square)](https://github.com/Aidan-996/MyToolbox/releases)
[![License](https://img.shields.io/github/license/Aidan-996/MyToolbox?style=flat-square)](LICENSE)
[![CI](https://img.shields.io/github/actions/workflow/status/Aidan-996/MyToolbox/cli.yml?style=flat-squarelinux.yml?style=flat-square&label=linux%20CIlabel=CLI%20CI)](https://github.com/Aidan-996/MyToolbox/actions)

[中文](#mytoolbox-中文) | [English](#mytoolbox-english)

---

## MyToolbox 中文

一键为 AI 编程工具安装提示词和技能的工具。**Windows 图形界面 + Linux/macOS 命令行**双版本，
基于 [freyrswift/UaStation](https://github.com/freyrswift/UaStation)（Ua Toolbox）独立维护，
已与上游脱钩：更新与提示词同步只指向本仓库。

> Windows 版程序与可执行文件均已品牌化为 MyBox：界面、窗口类与产物都是 `MyBox.exe`。

<p align="center">
  <img src="docs/screenshots/main-editor.png" width="520" alt="主界面" />
</p>

### ⚠️ 禁止商用声明

> **本工具完全免费、开源，禁止任何形式的商业使用。**
>
> 严禁对本工具进行出售、倒卖、打包收费、捆绑销售或以任何方式向他人收取费用。
> 如果你是花钱购买的，**你被骗了，请立即要求退款并举报卖家**。
>
> 违反本声明的行为将保留依法追究的权利。

### 法律声明

本工具依据中华人民共和国相关法律法规发布，使用本工具即表示您同意以下条款：

1. **合法用途**：本工具仅用于对用户本人合法拥有或已获书面授权的 AI 编程工具进行本地配置管理。严禁将本工具用于任何违反《中华人民共和国网络安全法》《数据安全法》《个人信息保护法》及其他法律法规的活动。

2. **授权使用**：用户在使用本工具对任何系统、软件或服务进行安全研究时，必须事先取得相关权利人的明确书面授权。未经授权的渗透测试、逆向工程或漏洞利用行为可能违反《中华人民共和国刑法》第二百八十五条、第二百八十六条等相关规定。

3. **免责条款**：本工具按"原样"提供，作者不对因使用本工具产生的任何直接或间接损失承担责任。用户应对自身使用行为及其后果承担全部法律责任。

4. **免费声明**：本工具完全免费且开源。任何以本工具名义收取费用的行为均属欺诈，请立即举报并申请退款。

5. **未成年人保护**：未满 18 周岁的用户应在监护人指导下使用本工具。

### 功能特性

| 特性 | 说明 |
|------|------|
| 🔧 一键安装 | 选择 AI 工具 → 选择提示词 → 点击安装，三步搞定 |
| 🤖 13+ 款 AI 工具 | Claude Code、Codex、Gemini CLI、OpenCode、Cursor、VS Code 等主流工具全覆盖 |
| 🔍 智能检测 | 自动识别电脑中已安装的 AI 工具，未安装的自动灰显 |
| 📝 8 套提示词模板 | 专家模式 / 开发工程 / 写作润色，按需选择 |
| 🧩 40 个内嵌技能 | 逆向 / 漏洞利用 / 内核 / 渗透等方向，随提示词一键装进各工具 |
| 🌐 中英双语 | 界面完整双语（重构后免责声明等界面全部走翻译表） |
| 🎨 深浅主题 | 跟随系统或手动切换，Win11 支持 Mica 效果 |
| 🔄 自动更新 | 启动时自动检查更新，发现新版本自动下载安装（更新源为本仓库 Releases） |
| 🐧 Linux/macOS 命令行版 | 同一套提示词/技能数据，`make` 一键编译，CI 双系统端到端测试 |
| 📦 单文件运行 | 1 MB 的 exe，无需安装，无任何依赖 |

### 相比上游的改动

本仓库在上游 v1.0.2 基础上做了一轮完整重构（Windows 版行为不变，只修错误路径）：

- **与上游脱钩**：更新检查、下载、提示词同步地址全部指向本仓库，启动时自动检查更新
- **修复真实 bug**：更新包截断下载被误判成功并覆盖真实 exe；更新器忽略写入/启动失败仍退出进程；下载标志线程时序；可点击区域缺手型光标；Skills 滚轮按未过滤数量钳制；颜色格式化溢出隐患
- **国际化补全**：免责声明正文、技能计数、空列表提示等 5 处硬编码中文改走翻译表（英文界面不再出现中文）
- **线程安全**：下载标志改 `std::atomic`；删除跨线程裸指针 toast hack，后台线程不再读任何 UI 状态
- **死代码清理**：未使用的函数/字段/翻译行/VS 向导遗留资源全部移除
- **去重**：矩形命中、按钮位置、toast、字符编码转换等 6 处重复实现各合并为一份（顺带修复非 ANSI 路径下更新批处理的编码隐患）
- **常量收拢**：强调色/面板色收进主题 `Palette`，DWM 裸属性号改具名常量
- **工程修正**：工具集从 v145 重定向到 v143（VS 2022 可直接编译）
- **目录架构重构**：Windows 源码目录 `Ua/` 更名 `MyBox/`，两个版本共用的提示词/技能数据提升到顶层 `shared/`；程序标识（exe 名、窗口类、注册表值、AppData 目录）品牌化为 MyBox

### 部署教程（Windows）

**方式一：下载现成版本（推荐）**

1. 打开 [Releases](https://github.com/Aidan-996/MyToolbox/releases) 页面，下载最新版的 `MyBox.exe`
2. 放到任意目录（如桌面）即可——单文件免安装、无需管理员权限；也可以在 [Actions](https://github.com/Aidan-996/MyToolbox/actions) 页面下载 CI 构建产物

**方式二：源码编译**

```powershell
git clone https://github.com/Aidan-996/MyToolbox.git
cd MyToolbox
msbuild MyBox.slnx /p:Configuration=Release /p:Platform=x64
```

产物在 `x64\Release\MyBox.exe`（需要 VS 2022+，详见下文"构建方法"）。

**首次启动会发生什么**

1. 弹出免责声明，点"我已了解"
2. 程序自动检查更新（更新源为本仓库），并同步最新提示词模板
3. exe 旁边会生成 `prompts\` 文件夹（8 个提示词模板，可直接编辑，也可以放自己的 .md 进去）

> 💡 配置与备份存放在 `%APPDATA%\MyBox\`（`state` 为注入状态，`backups` 为每次安装的备份）。若曾用过旧版 `Ua.exe`，旧状态在 `%APPDATA%\Ua`，不会自动迁移，对相应工具重新安装一次即可；确认无误后旧目录可手动删除。

### 使用指南（Windows）

主界面分两个标签：「**编辑器**」列出 15 个 AI 编程工具；「**工具**」是 Skills 管理与帮助入口。
卡片状态：**灰色** = 未安装；**绿色圆点** = 已注入；**黄色圆点** = 文件有内容但非本工具注入；整卡灰显 = 本机未检测到该工具。

**安装提示词（三步）**

1. 点一张工具卡片（如 Claude Code），弹窗内显示注入目标路径（如 `~\.claude\CLAUDE.md`）与提示词列表
2. 选择一个模板（8 个，带分类说明：专家 / 开发 / 写作），点「安装」
3. 重启该 AI 工具即可生效

安装时程序自动完成两件事：原配置文件备份（可随时还原）、40 个内嵌技能装入该工具的 skills 目录。

**卸载**：打开同一工具的弹窗，点「卸载」——从备份恢复原始文件（原本没有则删除），并移除该工具装入的技能。

**管理技能**：「工具」标签 →「Skills 管理」：搜索、按分类筛选、逐个 ✕ 卸载。不想让 AI 携带安全/逆向技能，在这里清空即可，不影响提示词。

**更换提示词**：重新打开弹窗选择其他模板安装即可；每次安装都会对当前内容留一份滚动备份（保留最近 200 份）。

**自动更新**：每次启动自动检查（也可在设置菜单里"检查更新"手动触发）。在仓库推送一个新的 `v*` 标签，CI 会自动构建双平台二进制并发布 Release，所有已安装用户下次启动时自动升级。

**文件位置速查**

| 内容 | 位置 |
|------|------|
| 注入目标 | 各工具的全局配置文件（弹窗内显示，如 `~\.claude\CLAUDE.md`） |
| 状态与备份 | `%APPDATA%\MyBox\` |
| 提示词模板 | exe 旁的 `prompts\` 文件夹，可自行编辑新增 |
| 技能存放 | 各工具的 skills 目录（如 `~\.claude\skills\`） |

#### 验证安装效果

打开目标 AI 工具，输入 `Ua` 后回车，如果回复"**Ua 已经准备好随时待命**"即表示提示词已成功加载：

<p align="center">
  <img src="docs/screenshots/claude-code-demo.png" width="640" alt="Ua 已经准备好随时待命" />
</p>

之后就可以用大白话向 AI 分配任务，提示词会自动触发对应的技能模板。

### 系统要求（Windows）

- Windows 10 1809+ / Windows 11
- x64 架构

### 构建方法（Windows）

```powershell
msbuild MyBox.slnx /p:Configuration=Release /p:Platform=x64
```

需要 Visual Studio 2022+（含 C++ 桌面开发工作负载）。项目已重定向到 v143 工具集，
VS 2022 直接打开 `MyBox.slnx` 生成即可；产物在 `x64\Release\MyBox.exe`。

### 命令行版（Linux / macOS）

Windows 版的跨平台命令行对应版本（Linux / macOS）在 [`cli/`](cli/) 目录，与 Windows 版**完全独立**，共用同一套提示词与技能数据文件。**一键安装**（免编译免 root）：

```bash
curl -fsSL https://raw.githubusercontent.com/Aidan-996/MyToolbox/main/cli/install.sh | sh

mybox list                           # 工具列表与注入状态
mybox install claude                 # 注入提示词（自动备份，mybox uninstall claude 恢复）
mybox skills install claude radare2  # 按需安装单个技能
mybox sync                           # 从 GitHub 同步新提示词模板
```

功能：探测 13 个 AI 编程工具、注入/卸载提示词（自动备份恢复）、Skills 安装/卸载、从 GitHub 同步提示词。详见 [cli/README.md](cli/README.md)，CI 会在 ubuntu + macOS 上自动编译并做端到端冒烟测试。

### Star History

[![Star History Chart](https://api.star-history.com/svg?repos=Aidan-996/MyToolbox&type=Date)](https://star-history.com/#Aidan-996/MyToolbox&Date)

### 反馈

遇到问题或建议请[提 Issue](https://github.com/Aidan-996/MyToolbox/issues)。

### 许可证与致谢

MIT License，详见 [LICENSE](LICENSE)。基于 [freyrswift/UaStation](https://github.com/freyrswift/UaStation)（MIT）重构与扩展，
内嵌 Skills 与提示词源自其 seep-kit 项目集，图标来自 [lobehub/lobe-icons](https://github.com/lobehub/lobe-icons) 与
[vscode-icons](https://github.com/vscode-icons/vscode-icons)（MIT），一并致谢。

---

## MyToolbox English

One-click prompt and skill installer for AI coding tools. **Windows GUI + Linux/macOS CLI**,
independently maintained from [freyrswift/UaStation](https://github.com/freyrswift/UaStation) (Ua Toolbox) —
update checks and prompt sync point only to this repository.

> The Windows app and binary are branded **MyBox** (`MyBox.exe`).

<p align="center">
  <img src="docs/screenshots/main-editor.png" width="520" alt="Main window" />
</p>

### ⚠️ Anti-Commercial Notice

> **This tool is completely free and open source. Commercial use is strictly prohibited.**
>
> It is forbidden to sell, resell, bundle, or charge for this tool in any way.
> If you paid money for it, **you were scammed — demand a refund and report the seller immediately**.
>
> The author reserves the right to pursue legal action against violators.

### Legal Notice

This tool is provided in compliance with applicable laws and regulations. By using this tool, you agree to the following terms:

1. **Lawful Use Only**: This tool is intended solely for managing local configurations of AI coding tools that the user legally owns or has written authorization to access.

2. **Authorization Required**: Users must obtain explicit written authorization before conducting any security research on systems, software, or services. Unauthorized penetration testing, reverse engineering or exploitation may be illegal in your jurisdiction.

3. **Disclaimer**: This tool is provided "as is" without warranty. The author assumes no liability for any damages arising from use. Users bear full legal responsibility for their actions.

4. **Free Software**: This tool is completely free and open source. Any attempt to charge for it is fraudulent.

5. **Minor Protection**: Users under 18 should use this tool under parental guidance.

### Features

| Feature | Description |
|---------|-------------|
| 🔧 One-Click Install | Select tool → Pick prompt → Install. Three steps. |
| 🤖 13+ AI Tools | Claude Code, Codex, Gemini CLI, OpenCode, Cursor, VS Code and more |
| 🔍 Auto Detection | Automatically detects installed AI tools on your system |
| 📝 8 Prompt Templates | Expert / Development / Writing modes |
| 🧩 40 Embedded Skills | Reverse engineering / exploit / kernel / pentest, installed together with prompts |
| 🌐 Bilingual | Fully translated CN/EN interface (disclaimer etc. now use the translation table) |
| 🎨 Themes | Dark / Light / System, Win11 Mica support |
| 🔄 Auto Update | Checks on launch and auto-installs new versions from this repo's Releases |
| 🐧 Linux/macOS CLI Edition | Same prompt/skill data, one `make` to build, tested on both OSes in CI |
| 📦 Portable | Single 1 MB exe, no installer, zero dependencies |

### What Changed from Upstream

This repository is a full refactor on top of upstream v1.0.2 (Windows behavior unchanged; only error paths fixed):

- **Decoupled from upstream**: update/download/prompt-sync URLs point to this repo; updates are checked automatically on launch
- **Real bug fixes**: truncated update downloads were treated as success and could overwrite the real exe; the updater ignored write/launch failures before quitting; download-flag timing; missing hand cursors on clickable lists; skills wheel clamping; color formatting overflow
- **i18n completion**: 5 hardcoded Chinese strings (disclaimer body, skill counts, empty lists, no-link toast) now go through the translation table
- **Thread safety**: download flag is `std::atomic`; removed the cross-thread toast pointer hack — background threads no longer read UI state
- **Dead code removal**: unused functions/fields/translation rows/VS-template resources
- **Deduplication**: 6 duplicated implementations merged (rect hit-testing, button rects, toast, charset conversion — also fixes a non-ANSI path encoding hazard in the update script)
- **Constant consolidation**: accent/panel colors moved into the theme `Palette`; DWM attribute IDs named
- **Build fix**: toolset retargeted v145 → v143 so VS 2022 builds out of the box
- **Directory restructure**: the Windows source dir `Ua/` is now `MyBox/`; the shared prompt/skill data moved to a top-level `shared/`; app identity (exe name, window class, registry value, AppData dir) rebranded to MyBox

### Deployment (Windows)

**Option 1: download a ready build (recommended)**

1. Open the [Releases](https://github.com/Aidan-996/MyToolbox/releases) page and download `MyBox.exe` from the latest release
2. Put it anywhere (e.g. the desktop) — a single portable file, no installer, no admin rights; CI artifacts are also available on the [Actions](https://github.com/Aidan-996/MyToolbox/actions) page

**Option 2: build from source**

```powershell
git clone https://github.com/Aidan-996/MyToolbox.git
cd MyToolbox
msbuild MyBox.slnx /p:Configuration=Release /p:Platform=x64
```

Output lands in `x64\Release\MyBox.exe` (requires VS 2022+, see "Build" below).

**What happens on first launch**

1. A disclaimer dialog appears — click "I Understand"
2. The app checks for updates (from this repo) and syncs the latest prompt templates
3. A `prompts\` folder is created next to the exe (8 templates; edit them freely or drop in your own .md files)

> 💡 Config and backups live in `%APPDATA%\MyBox\` (`state` = install state, `backups` = one backup per install). If you previously used the old `Ua.exe`, its state in `%APPDATA%\Ua` is not migrated — just reinstall for the affected tools, then delete the old folder.

### User Guide (Windows)

The main window has two tabs: 「**Editor**」 lists 15 AI coding tools; 「**Tools**」 holds the Skills manager and help entries.
Card states: **gray** = not installed; **green dot** = injected; **yellow dot** = target file has content but wasn't injected by this tool; a fully dimmed card = tool not detected on this machine.

**Install a prompt (three steps)**

1. Click a tool card (e.g. Claude Code) — the dialog shows the target path (e.g. `~\.claude\CLAUDE.md`) and the prompt list
2. Pick a template (8 available: Expert / Development / Writing) and click "Install"
3. Restart that AI tool to apply

Two things happen automatically on install: the original config file is backed up (restorable anytime), and 40 embedded skills are copied into the tool's skills directory.

**Uninstall**: open the same tool's dialog and click "Remove" — the original file is restored from backup (deleted if there was none) and the installed skills are removed.

**Manage skills**: 「Tools」 tab → 「Skills Manager」: search, filter by category, remove individually with ✕. If you don't want the AI to carry security/RE skills, clear them here — prompts are unaffected.

**Switch prompts**: reopen the dialog and install another template; every install keeps a rolling backup of the current content (last 200 kept).

**Auto update**: checked on every launch (or via menu "Check for Updates"). Pushing a new `v*` tag makes CI build and publish both binaries; every installed user upgrades automatically on next launch.

**File locations**

| What | Where |
|------|-------|
| Injection targets | Each tool's global config file (shown in the dialog, e.g. `~\.claude\CLAUDE.md`) |
| State & backups | `%APPDATA%\MyBox\` |
| Prompt templates | `prompts\` folder next to the exe — editable |
| Skills | Each tool's skills directory (e.g. `~\.claude\skills\`) |

#### Verify Installation

Open your AI tool and type `Ua` — if it replies **"Ua is ready and standing by"**, the prompt is loaded correctly:

<p align="center">
  <img src="docs/screenshots/claude-code-demo.png" width="640" alt="Ua is ready" />
</p>

### Requirements (Windows)

- Windows 10 1809+ / Windows 11
- x64

### Build (Windows)

```powershell
msbuild MyBox.slnx /p:Configuration=Release /p:Platform=x64
```

Requires Visual Studio 2022+ with the C++ Desktop workload. The project is retargeted to the
v143 toolset, so VS 2022 opens `MyBox.slnx` and builds directly; output lands in `x64\Release\MyBox.exe`.

### CLI Version (Linux / macOS)

A cross-platform CLI edition (Linux / macOS) lives in [`cli/`](cli/). It is fully independent from the Windows GUI, but shares the same prompt and skill data files. **One-line install** (no compiler, no root):

```bash
curl -fsSL https://raw.githubusercontent.com/Aidan-996/MyToolbox/main/cli/install.sh | sh

mybox list                           # tools and install state
mybox install claude                 # inject a prompt (auto backup; mybox uninstall claude restores)
mybox skills install claude radare2  # install a single skill
mybox sync                           # sync new prompt templates from GitHub
```

Features: detect 13 AI coding tools, install/uninstall prompts (with automatic backup & restore), manage skills, sync new prompts from GitHub. See [cli/README.md](cli/README.md). CI builds it on ubuntu and macOS and runs end-to-end smoke tests on every push.

### Star History

[![Star History Chart](https://api.star-history.com/svg?repos=Aidan-996/MyToolbox&type=Date)](https://star-history.com/#Aidan-996/MyToolbox&Date)

### Feedback

Please [open an issue](https://github.com/Aidan-996/MyToolbox/issues) for bugs and feature requests.

### License & Credits

MIT License, see [LICENSE](LICENSE). Refactored and extended from
[freyrswift/UaStation](https://github.com/freyrswift/UaStation) (MIT). Embedded skills and prompts
come from its seep-kit collection; icons from [lobehub/lobe-icons](https://github.com/lobehub/lobe-icons)
and [vscode-icons](https://github.com/vscode-icons/vscode-icons) (MIT). Many thanks.
