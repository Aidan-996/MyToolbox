# Ua Toolbox

[中文](#ua-toolbox-中文) | [English](#ua-toolbox-english)

---

## Ua Toolbox 中文

一键为 AI 编程工具安装提示词和技能的 Windows 原生工具。

### 法律声明

本工具依据中华人民共和国相关法律法规发布，使用本工具即表示您同意以下条款：

1. **合法用途**：本工具仅用于对用户本人合法拥有或已获书面授权的 AI 编程工具进行本地配置管理。严禁将本工具用于任何违反《中华人民共和国网络安全法》《数据安全法》《个人信息保护法》及其他法律法规的活动。

2. **授权使用**：用户在使用本工具对任何系统、软件或服务进行安全研究时，必须事先取得相关权利人的明确书面授权。未经授权的渗透测试、逆向工程或漏洞利用行为可能违反《中华人民共和国刑法》第二百八十五条、第二百八十六条等相关规定。

3. **免责条款**：本工具按"原样"提供，作者不对因使用本工具产生的任何直接或间接损失承担责任。用户应对自身使用行为及其后果承担全部法律责任。

4. **免费声明**：本工具完全免费且开源。任何以本工具名义收取费用的行为均属欺诈，请立即举报并申请退款。

5. **未成年人保护**：未满 18 周岁的用户应在监护人指导下使用本工具。

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

### Legal Notice

This tool is provided in compliance with applicable laws and regulations. By using this tool, you agree to the following terms:

1. **Lawful Use Only**: This tool is intended solely for managing local configurations of AI coding tools that the user legally owns or has written authorization to access. Any use that violates applicable cybersecurity laws, data protection regulations, or other legal requirements is strictly prohibited.

2. **Authorization Required**: Users must obtain explicit written authorization from the relevant rights holders before conducting any security research on systems, software, or services. Unauthorized penetration testing, reverse engineering, or vulnerability exploitation may constitute criminal offenses under applicable law.

3. **Disclaimer**: This tool is provided "as is" without warranty. The author assumes no liability for any direct or indirect damages arising from the use of this tool. Users bear full legal responsibility for their actions.

4. **Free Software**: This tool is completely free and open source. Any attempt to charge for this tool is fraudulent — please report it and request a refund immediately.

5. **Minor Protection**: Users under 18 years of age should use this tool under parental guidance.

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
