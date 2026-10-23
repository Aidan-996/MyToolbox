# 内嵌 .md 压缩方案评估（P4-16）

## 当前状态

实测数据（2026-10-23）：

- **exe 大小**：`D:\Ua\Ua\x64\Release\Ua.exe` = 909,824 字节（≈ 888 KB，Release/x64）
- **内嵌 SKILL.md 数量**：40 个
- **SKILL.md 原始文本总量**：610,114 字节（≈ 596 KB）
- **加载路径**：`inject.cpp::ExtractSkill()` 通过 `FindResource/LoadResource` 读 RCDATA，运行时按需写入目标目录
- **资源声明**：`Ua.rc` 用 RCDATA 直接引用 `skills/<name>/SKILL.md` 原文件

Markdown 文本可压缩性高（重复的 heading、代码围栏、短语），deflate/LZMS 期望压缩比在 30-40%（即压缩后保留约 180-240 KB）。

## 可行的压缩方案

### 方案 A：Windows Compression API（推荐优先级最高）

- 头文件：`compressapi.h`
- 库：`Cabinet.lib`
- 算法：`COMPRESS_ALGORITHM_XPRESS_HUFF` 或 `COMPRESS_ALGORITHM_LZMS`
- 优点：Windows 8+ 原生 API，零第三方依赖，调用简单（`CreateDecompressor`/`Decompress`）
- 缺点：构建期需要外部工具生成 `.bin`（Windows 的 Compression API 可用来做构建期压缩，或用 PowerShell `[System.IO.Compression.DeflateStream]` 预压）

### 方案 B：miniz（单文件 zlib 兼容）

- 优点：单个 `.c/.h` 文件，deflate/inflate 兼容标准 zlib
- 缺点：引入一份第三方代码，但不涉及 vcpkg

### 方案 C：zstd / brotli

- 压缩比最好（Markdown 场景约 25%），但要引入依赖，不划算

## 预期收益

| 项目 | 当前 | 压缩后（估） |
|------|------|--------------|
| exe 大小 | 888 KB | **≈ 540-600 KB** |
| 节省 | - | 约 290-350 KB（33-40%） |

节省量低于任务描述中的"60-70%"估计，原因是 exe 的 888 KB 里只有约 596 KB 是 Markdown 文本，剩下 ≈ 290 KB 是代码、CRT、PE 结构，不参与压缩。即使 Markdown 压到 0 字节，exe 下限也是 290 KB 左右。

## 实现成本

1. **构建期**：
   - 新增预处理脚本（PowerShell 或 Python）：遍历 `skills/*/SKILL.md` 生成 `skills/*/SKILL.bin`
   - 修改 `Ua.rc`：RCDATA 从 `.md` 改成 `.bin`
   - 修改 `.vcxproj`：加 PreBuildEvent 调用压缩脚本
   - 风险：构建产物不再是幂等的（压缩库版本差异可能让 .bin 字节不一致）

2. **运行期**（inject.cpp）：
   - `ExtractSkill()` 内加解压路径：`LoadResource` → 解压到堆缓冲 → `WriteFile`
   - 需要错误处理：解压失败时的回退（没有回退，因为原文件不再内嵌）
   - 新增约 50-80 行代码

3. **测试负担**：
   - 每新增 SKILL.md 要跑一遍构建验证
   - CI/本地构建环境必须具备压缩工具
   - 解压失败难以在终端用户机器上调试（RCDATA 字节已被压过，无法直接读）

## 风险

- **构建流程脆弱性**：预处理脚本失败会卡住整个构建，比当前"RCDATA 直接引用 .md"的方案多一个失败面
- **调试难度**：当前内嵌 .md 可以直接用 Resource Hacker 查看，压缩后要先解才能看
- **收益边际**：exe 从 888 KB 降到 ≈ 560 KB，对现代分发渠道（GitHub Release、Windows Defender SmartScreen 阈值等）**没有质变**
- **启动延迟**：首次 `ExtractSkill()` 多一步解压，但量级在毫秒以下，可忽略

## 结论

**建议不实施。**

理由：

1. 当前 exe 888 KB 远低于常见"膨胀警戒线"（通常 > 5 MB 才明显感知）
2. 节省 290-350 KB 对用户下载、启动速度、磁盘占用都无实质影响
3. 构建流程复杂度和调试难度的上升，与收益不成比例
4. `ExtractSkill()` 已经是按需加载，运行时内存占用不受 exe 体积影响

**触发实施的阈值**：当 exe 超过 **2 MB**（即 SKILL.md 总量翻 3-4 倍，约 100-150 个 skill）时再重新评估。届时首选方案 A（Windows Compression API + PowerShell 预处理），零依赖、与现有工具链契合最好。

## 相关文件（如果未来实施）

- `D:\Ua\Ua\inject.cpp` - `ExtractSkill()` 需加解压路径
- `D:\Ua\Ua\inject.h` - 加解压辅助函数声明
- `D:\Ua\Ua\Ua.rc` - RCDATA 从 .md 改为 .bin
- `D:\Ua\Ua\Ua.vcxproj` - 新增 PreBuildEvent
- 新建：`D:\Ua\tools\compress-skills.ps1`（或 `.py`）
