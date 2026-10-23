# Ua Toolbox

[中文](#ua-toolbox-中文) | [English](#ua-toolbox-english)

---

## Ua Toolbox 中文

一键为 AI 编程工具安装提示词和技能的 Windows 原生工具。

### 功能特性

- **15 款主流 AI 编程工具**一键安装提示词
- **中英双语界面**：可在设置菜单切换
- **深浅色主题**：跟随系统或手动固定
- **自动更新检查**：静默检查 GitHub Releases
- **单文件、免安装**：一个 EXE，无外部依赖

### 使用方法

1. 从 [Releases](https://github.com/freyrswift/UaStation/releases) 下载 `Ua.exe`
2. 双击运行（无需安装、无需管理员权限）
3. 选择 AI 编程工具（如 Claude Code）
4. 选择提示词模板，点"安装"
5. 重启目标 AI 工具，即刻生效

卸载：选中已安装的工具，点"卸载"即可恢复原始配置。

### 系统要求

- Windows 10 1809+ / Windows 11
- x64 架构

### 构建方法

```powershell
msbuild Ua.slnx /p:Configuration=Release /p:Platform=x64
```

需要 Visual Studio 2022+，含 C++ 桌面开发工作负载。

### 许可证

MIT License，详见 [LICENSE](LICENSE)。

---

## Ua Toolbox English

A lightweight Windows tool for installing prompts and skills into AI coding tools.

### Features

- **15 AI coding tools** supported with one-click prompt installation
- **CN/EN bilingual interface**
- **Dark/Light/System theme**
- **Auto-update check** via GitHub Releases
- **Single exe, zero dependencies**

### Usage

1. Download `Ua.exe` from [Releases](https://github.com/freyrswift/UaStation/releases)
2. Double-click to run (no installer, no admin required)
3. Select an AI tool (e.g. Claude Code)
4. Choose a prompt template and click "Install"
5. Restart the AI tool to apply

To uninstall: select the tool and click "Remove" to restore original config.

### Requirements

- Windows 10 1809+ / Windows 11
- x64

### Build

```powershell
msbuild Ua.slnx /p:Configuration=Release /p:Platform=x64
```

Requires Visual Studio 2022+ with C++ Desktop Development workload.

### License

MIT License. See [LICENSE](LICENSE).
