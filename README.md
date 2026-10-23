# Ua AI Toolbox

一键为各个 AI 编程工具注入提示词和技能 (Skills) 的 Windows 原生工具。

---

> ## ⚠️ 仅供学习与授权安全研究使用
>
> 本工具的本质是对各 AI 编程工具的**本地客户端配置文件**做自动化编辑，
> 不涉及任何对网络服务、他人系统的攻击。所注入的提示词和 Skills 内容
> 覆盖安全研究方向 (逆向、漏洞挖掘、渗透测试、游戏安全等)，
> **仅应在授权范围内的实验环境中使用**。
>
> 使用者应自行确保所有操作符合所在地区法律法规。作者和贡献者不对任何
> 滥用后果承担责任。
>
> 本工具完全免费、开源。**如你是花钱买的，请立即退款**。

---

## 功能特性 (Features)

- **15 款主流 AI 编程工具**一键注入：覆盖 Claude Code、Codex、Cursor、
  Gemini CLI、DeepSeek Harness、Grok Build、OpenCode、VS Code Copilot、
  Devin Desktop 等。
- **40 个安全研究 Skills 内嵌**：全部作为资源打包进单个 EXE，离线可用，
  按需释放到目标工具的 `skills/` 目录。
- **中英双语界面** (Simplified Chinese / English)：启动时跟随系统语言，
  也可在设置菜单切换。
- **深浅色主题** (Dark / Light)：跟随系统或手动固定。
- **启动免责声明**：每次启动都显示一次免责声明，必须确认后才进入主界面。
- **自动更新检查**：静默检查 GitHub Releases，可在"关于"面板里查看。
- **单文件、免安装**：一个 EXE，没有安装包，没有外部依赖，没有释放
  dll 到用户目录。

---

## 支持的 AI 工具 (Supported Tools)

| 工具               | 注入路径 (相对用户目录)                             | Skills |
| ------------------ | --------------------------------------------------- | :----: |
| Claude Code        | `.claude\CLAUDE.md`                                 |   OK   |
| Codex (OpenAI)     | `.codex\AGENTS.md`                                  |   OK   |
| Gemini CLI         | `.gemini\GEMINI.md`                                 |        |
| Grok Build         | `.grok\AGENTS.md`                                   |        |
| ZCode (Z.ai)       | `.zcode\AGENTS.md`                                  |   OK   |
| DeepSeek Harness   | `.dsh\AGENTS.md`                                    |   OK   |
| OpenCode           | `.config\opencode\AGENTS.md`                        |        |
| Hermes             | `.hermes\SOUL.md`                                   |        |
| MiniMax Code       | `.minimax\AGENTS.md`                                |        |
| Cursor             | `.cursor\rules\Ua.mdc`                              |        |
| VS Code (Copilot)  | `.copilot\copilot-instructions.md`                  |        |
| Devin Desktop      | `.codeium\windsurf\memories\global_rules.md`        |        |
| CodeBuddy          | `.codebuddy\CODEBUDDY.md`                           |   OK   |
| WorkBuddy          | *(本地提示词位置待确认)*                            |        |
| 豆包 (Doubao)      | *(无本地提示词文件)*                                |        |

> 路径以上游官方文档为准，若上游调整请在 issue 中反馈。

---

## 内嵌的 Skills (40 个)

> Skills 为 Markdown 格式的"任务型提示词包"，供 Claude Code 等支持
> skill 机制的工具加载。注入后在目标工具启动时自动生效。

**逆向工程 (Reverse Engineering)**
`reverse-engineering` · `re-angr` · `re-deobfuscate` · `re-packer-id` ·
`re-patching` · `re-unpack` · `re-shellcode` · `re-crash-triage` ·
`ida-reverse` · `radare2` · `re-frida`

**游戏安全 (Game Security)**
`re-anti-cheat` · `re-game` · `re-console` · `cheat-engine` ·
`unity-game-research` · `dma-attack`

**内核与底层 (Kernel / Low-level)**
`re-kernel` · `re-hypervisor` · `re-ebpf`

**漏洞挖掘与利用 (Vulnerability & Exploit)**
`re-exploit` · `re-fuzzing` · `re-pwn` · `exploit-development` ·
`re-evasion`

**恶意代码分析 (Malware Analysis)**
`malware-analysis` · `process-injection-detect` · `threat-hunting`

**软件破解与保护 (Licensing / DRM)**
`re-license` · `re-keygen` · `re-drm` · `softseep` ·
`client-license-validation-bypass`

**移动端 (Mobile)**
`apkseep` · `apk-reverse` · `mobile-security`

**Web / 红队 / AI 安全**
`web-security` · `red-team-ops` · `ai-llm-security` ·
`mcp-js-reverse-playbook`

Skills 的具体来源与原始许可证见 [NOTICES.md](NOTICES.md)。

---

## 使用方法 (Usage)

1. 从 [Releases](https://github.com/freyrswift/UaStation/releases) 下载最新的
   `Ua.exe`。
2. 双击运行 (**无需安装、无需管理员权限**)。
3. 阅读并接受启动时的免责声明。
4. 在列表里选择你用的 AI 编程工具 (如 Claude Code)。
5. 在弹出的面板中选择一份内置提示词模板，点"注入"。
6. 可选: 在 "Skills 管理" 中按需挑选 Skills 注入到支持的工具。
7. 重启目标 AI 工具 (或在同一会话里重新加载)，注入内容即刻生效。

卸载: 在列表中选中已注入的工具，点"卸载"，Ua 会把该工具配置文件恢复到
注入前的原始内容。

---

## 系统要求 (Requirements)

- Windows 10 1809 (build 17763) 或更高 / Windows 11
- x64 架构
- 无需 .NET、无需 VC++ Redistributable (使用静态 CRT 链接)

---

## 构建方法 (Build)

### 依赖

- Visual Studio 2022 (17.8+)，含"使用 C++ 的桌面开发"工作负载
- Windows 11 SDK (10.0.22621 或更高)
- 平台工具集 v143

### 命令行构建

```powershell
# 在仓库根目录
msbuild Ua.slnx /p:Configuration=Release /p:Platform=x64
```

构建产物位于 `Ua\x64\Release\Ua.exe`。

### 从 IDE 构建

直接用 Visual Studio 打开 `Ua.slnx`，选 `Release | x64`，F7 编译。

---

## 项目结构 (Project Layout)

```
Ua/
├─ Ua.slnx                     // 解决方案
└─ Ua/
   ├─ Ua.vcxproj               // 项目文件
   ├─ Ua.rc / Resource.h       // 资源 (图标、SVG、Skills、提示词)
   ├─ framework.h / targetver.h
   ├─ config.h                 // 常量、枚举、翻译表、工具与 skill 清单
   ├─ state.h / state.cpp      // 全局运行时状态 (主题、语言、hover 等)
   ├─ inject.h / inject.cpp    // 注入引擎 (标记块、幂等覆盖、备份恢复)
   ├─ ui.h / ui.cpp            // 窗口绘制、hit-test、交互
   ├─ app.cpp                  // 程序入口与消息循环
   ├─ icons/                   // Logo SVG 源文件
   └─ skills/                  // SKILL.md 源文件
```

主要模块职责:

- **`config.h`** — 只读的编译期数据：UI 常量、调色板、字符串表、工具与
  Skill 清单。所有 `.cpp` 共享。
- **`state.cpp`** — 单一全局状态结构 `g_state`，承载当前标签、hover
  目标、主题、语言、置顶/自启等运行时信息。
- **`inject.cpp`** — 核心注入逻辑：读取目标 AI 工具的配置文件，用
  标记块包裹并写入提示词；卸载时按标记块精确回退；Skill 释放到对应
  `skills/<name>/SKILL.md`。
- **`ui.cpp`** — Direct2D + DirectWrite 直接绘制整个界面：tab、列表、
  卡片、悬停、设置菜单、Skill 管理面板、关于面板、免责声明。
- **`app.cpp`** — `WinMain`、窗口类注册、消息循环、更新检查线程。

---

## 致谢 (Credits)

本项目内嵌了来自多个开源项目的图标、Skills 和提示词模板。完整的来源、
许可证和修改说明见 **[NOTICES.md](NOTICES.md)**。特别感谢:

- `lobehub/lobe-icons` — 大部分 AI 工具 Logo
- `vscode-icons/vscode-icons` — VS Code Logo
- `dslsdzc/rev-skills` — 21 个逆向 skill
- `Masriyan/Claude-Code-CyberSecurity-Skill` — 7 个网络安全 skill
- `farion1231/cc-switch` — 注入引擎设计思路

---

## 许可证 (License)

本项目本体以 **MIT License** 发布，详见 [LICENSE](LICENSE)。

内嵌的第三方素材分别遵循其原始许可证，详见 [NOTICES.md](NOTICES.md)。
