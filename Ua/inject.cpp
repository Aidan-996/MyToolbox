// inject.cpp : 提示词注入、状态/备份管理、Skills 安装
//

#include "inject.h"
#include "state.h"

#include <shlobj.h>
#include <algorithm>
#include <cstdio>

#pragma comment(lib, "shell32.lib")

// ---------------------------------------------------------------------------
// 路径工具
// ---------------------------------------------------------------------------
std::wstring HomeDir()
{
    wchar_t buf[MAX_PATH];
    DWORD n = GetEnvironmentVariableW(L"USERPROFILE", buf, MAX_PATH);
    return n && n < MAX_PATH ? std::wstring(buf) : std::wstring();
}

std::wstring AppDataDir()
{
    wchar_t buf[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, buf)))
        return std::wstring(buf) + L"\\Ua";
    return HomeDir() + L"\\AppData\\Roaming\\Ua";
}

bool Supported(int i) { return kItems[i].target && kItems[i].target[0]; }

bool ToolDetected(int i)
{
    if (!Supported(i)) return false;
    // 检查工具的主目录是否存在（只看第一级，如 ~/.claude/ 而不是子目录）
    std::wstring home = HomeDir();
    if (home.empty()) return false;
    // target 格式如 ".claude\\CLAUDE.md"，取第一个路径分隔符之前的部分
    std::wstring target = kItems[i].target;
    size_t sep = target.find(L'\\');
    std::wstring toolDir = home + L"\\" + (sep != std::wstring::npos ? target.substr(0, sep) : target);
    DWORD a = GetFileAttributesW(toolDir.c_str());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

std::wstring TargetPath(int i)
{
    std::wstring home = HomeDir();
    return Supported(i) && !home.empty() ? home + L"\\" + kItems[i].target : std::wstring();
}

std::wstring PromptDir()
{
    wchar_t exe[MAX_PATH];
    DWORD n = GetModuleFileNameW(nullptr, exe, MAX_PATH);
    std::wstring dir(exe, n < MAX_PATH ? n : 0);
    return dir.substr(0, dir.find_last_of(L'\\')) + L"\\prompts";
}

// 列出 prompts\ 下所有 .md 文件
std::vector<std::wstring> ListPrompts()
{
    std::vector<std::wstring> out;
    std::wstring pattern = PromptDir() + L"\\*.md";
    WIN32_FIND_DATAW fd;
    HANDLE h = FindFirstFileW(pattern.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return out;
    do
    {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            out.push_back(fd.cFileName);
    } while (FindNextFileW(h, &fd));
    FindClose(h);
    std::sort(out.begin(), out.end());
    return out;
}

// 先找 prompts\<key>.md，没有就用 prompts\default.md；shown 返回给界面显示的相对路径
std::wstring PromptPath(int i, std::wstring* shown)
{
    std::wstring own = std::wstring(kItems[i].key) + L".md";
    for (const std::wstring& name : { own, std::wstring(L"default.md") })
    {
        std::wstring p = PromptDir() + L"\\" + name;
        DWORD a = GetFileAttributesW(p.c_str());
        if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY))
        {
            if (shown) *shown = L"prompts\\" + name;
            return p;
        }
    }
    if (shown) *shown = L"prompts\\" + own;
    return std::wstring();
}

// ---------------------------------------------------------------------------
// 内嵌提示词释放
// ---------------------------------------------------------------------------
struct EmbeddedPrompt
{
    int resId;
    const wchar_t* filename;
};

static const EmbeddedPrompt kEmbeddedPrompts[] = {
    { IDR_PROMPT_STANDARD_PRO,    L"Ua-standard-pro.md" },
    { IDR_PROMPT_STANDARD_PRO_EN, L"Ua-standard-pro-en.md" },
    { IDR_PROMPT_SOL_PRO,         L"Ua-sol-pro.md" },
    { IDR_PROMPT_DEV_MAINTAINER,  L"Dev-maintainer.md" },
    { IDR_PROMPT_DEV_DEBUGGING,   L"Dev-debugging.md" },
    { IDR_PROMPT_DEV_CODE_REVIEW, L"Dev-code-review.md" },
    { IDR_PROMPT_WRITING_EDITOR,  L"Writing-editor.md" },
    { IDR_PROMPT_WRITING_TECH_DOCS, L"Writing-tech-docs.md" },
};

void ExtractEmbeddedPrompts()
{
    std::wstring dir = PromptDir();
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);

    for (const auto& ep : kEmbeddedPrompts)
    {
        std::wstring path = dir + L"\\" + ep.filename;
        // 只在文件不存在时释放（不覆盖用户修改过的版本）
        DWORD a = GetFileAttributesW(path.c_str());
        if (a != INVALID_FILE_ATTRIBUTES) continue;

        HRSRC res = FindResourceW(hInst, MAKEINTRESOURCEW(ep.resId), RT_RCDATA);
        HGLOBAL h = res ? LoadResource(hInst, res) : nullptr;
        if (!h) continue;
        DWORD size = SizeofResource(hInst, res);
        const char* data = static_cast<const char*>(LockResource(h));
        if (data && size > 0)
            WriteAtomic(path, std::string(data, size));
    }
}

// ---------------------------------------------------------------------------
// 文件读写
// ---------------------------------------------------------------------------
ReadResult ReadBytes(const std::wstring& path, std::string& out)
{
    out.clear();
    HANDLE h = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        DWORD e = GetLastError();
        return e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND ? kReadMissing : kReadFail;
    }
    LARGE_INTEGER size{};
    bool ok = GetFileSizeEx(h, &size) && size.QuadPart <= (LONGLONG)kMaxFile;
    if (ok && size.QuadPart > 0)
    {
        out.resize((size_t)size.QuadPart);
        DWORD read = 0;
        ok = ReadFile(h, out.data(), (DWORD)out.size(), &read, nullptr) && read == out.size();
    }
    CloseHandle(h);
    return ok ? kReadOk : kReadFail;
}

bool WriteAtomic(const std::wstring& path, const std::string& data)
{
    std::wstring dir = path.substr(0, path.find_last_of(L'\\'));
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);

    std::wstring tmp = path + L".ua.tmp";
    HANDLE h = CreateFileW(tmp.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return false;
    DWORD written = 0;
    bool ok = data.empty() || (WriteFile(h, data.data(), (DWORD)data.size(), &written, nullptr) && written == data.size());
    ok = FlushFileBuffers(h) && ok;
    CloseHandle(h);
    if (!ok || !MoveFileExW(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        DeleteFileW(tmp.c_str());
        return false;
    }
    return true;
}

bool IsUtf16(const std::string& s)
{
    return s.size() >= 2 && ((s[0] == '\xFF' && s[1] == '\xFE') || (s[0] == '\xFE' && s[1] == '\xFF'));
}

bool StripBom(std::string& s)
{
    if (s.compare(0, 3, "\xEF\xBB\xBF") != 0) return false;
    s.erase(0, 3);
    return true;
}

// ---------------------------------------------------------------------------
// 状态文件 (%APPDATA%\Ua\state\<key>.dat)
// 内容：backupFile\noriginalExisted\ninstalledAt
// ---------------------------------------------------------------------------
static std::wstring StateDir() { return AppDataDir() + L"\\state"; }

static std::wstring StateFile(int i) { return StateDir() + L"\\" + kItems[i].key + L".dat"; }

bool LoadState(int i, TargetState& ts)
{
    std::string raw;
    if (ReadBytes(StateFile(i), raw) != kReadOk) return false;
    // 三行：backupFile, originalExisted (0/1), installedAt
    size_t p1 = raw.find('\n'), p2 = raw.find('\n', p1 + 1);
    if (p1 == std::string::npos || p2 == std::string::npos) return false;
    std::string line1 = raw.substr(0, p1);
    if (!line1.empty() && line1.back() == '\r') line1.pop_back();
    // 备份路径从 UTF-8 转回 wchar_t
    int need = MultiByteToWideChar(CP_UTF8, 0, line1.c_str(), (int)line1.size(), nullptr, 0);
    ts.backupFile.resize(need);
    MultiByteToWideChar(CP_UTF8, 0, line1.c_str(), (int)line1.size(), ts.backupFile.data(), need);
    ts.originalExisted = raw[p1 + 1] == '1';
    ts.installedAt = strtoull(raw.c_str() + p2 + 1, nullptr, 10);
    return true;
}

bool SaveState(int i, const TargetState& ts)
{
    SHCreateDirectoryExW(nullptr, StateDir().c_str(), nullptr);
    // 备份路径转 UTF-8
    int need = WideCharToMultiByte(CP_UTF8, 0, ts.backupFile.c_str(), (int)ts.backupFile.size(), nullptr, 0, nullptr, nullptr);
    std::string bkp(need, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ts.backupFile.c_str(), (int)ts.backupFile.size(), bkp.data(), need, nullptr, nullptr);
    char buf[32];
    int n = sprintf_s(buf, "\n%d\n%llu\n", ts.originalExisted ? 1 : 0, ts.installedAt);
    return WriteAtomic(StateFile(i), bkp + std::string(buf, n));
}

bool DeleteState(int i)
{
    return DeleteFileW(StateFile(i).c_str()) || GetLastError() == ERROR_FILE_NOT_FOUND;
}

bool HasState(int i)
{
    DWORD a = GetFileAttributesW(StateFile(i).c_str());
    return a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY);
}

// ---------------------------------------------------------------------------
// 备份 (%APPDATA%\Ua\backups\<key>\)
// ---------------------------------------------------------------------------
static std::wstring BackupDir(int i) { return AppDataDir() + L"\\backups\\" + kItems[i].key; }

std::wstring MakeBackup(int i, bool fileExists)
{
    std::wstring dir = BackupDir(i);
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);

    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t name[64];
    swprintf_s(name, L"%04d%02d%02d-%02d%02d%02d-%03d.bak",
        st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
    std::wstring dst = dir + L"\\" + name;

    if (fileExists)
        CopyFileW(TargetPath(i).c_str(), dst.c_str(), FALSE);

    // 限制备份数量
    WIN32_FIND_DATAW fd;
    std::vector<std::wstring> baks;
    HANDLE h = FindFirstFileW((dir + L"\\*.bak").c_str(), &fd);
    if (h != INVALID_HANDLE_VALUE)
    {
        do { baks.push_back(fd.cFileName); } while (FindNextFileW(h, &fd));
        FindClose(h);
    }
    if ((int)baks.size() > kMaxBackups)
    {
        std::sort(baks.begin(), baks.end());
        for (int n = 0; n < (int)baks.size() - kMaxBackups; ++n)
            DeleteFileW((dir + L"\\" + baks[n]).c_str());
    }
    return dst;
}

bool RestoreBackup(int i, const TargetState& ts)
{
    std::wstring path = TargetPath(i);
    if (!ts.originalExisted)
    {
        DeleteFileW(path.c_str());
        return true;
    }
    if (ts.backupFile.empty()) return false;
    return CopyFileW(ts.backupFile.c_str(), path.c_str(), FALSE) != 0;
}

// ---------------------------------------------------------------------------
// 状态检测
// ---------------------------------------------------------------------------
int ScanState(int i)
{
    if (!Supported(i)) return kNotInstalled;
    bool hasState = HasState(i);

    // 检查目标文件是否有内容
    std::wstring path = TargetPath(i);
    std::string doc;
    bool fileHasContent = false;
    if (!path.empty() && ReadBytes(path, doc) == kReadOk && !IsUtf16(doc))
    {
        StripBom(doc);
        for (char c : doc) if (c != ' ' && c != '\t' && c != '\r' && c != '\n') { fileHasContent = true; break; }
    }

    if (hasState && fileHasContent) return kInstalled;          // 绿色：已注入且生效
    if (hasState && !fileHasContent) return kInstalled;         // 有状态但文件空了（用户手动清了）
    if (!hasState && fileHasContent) return kActiveNoState;     // 黄色：文件有内容但不是我们注入的
    return kNotInstalled;                                       // 灰色
}

void RefreshStates()
{
    for (int i = 0; i < kItemCount; ++i)
        g_state[i] = kItems[i].key ? ScanState(i) : kNotInstalled;
}

// ---------------------------------------------------------------------------
// Skills 安装（从 exe 内嵌资源释放到 AI 工具的 skills 目录）
// ---------------------------------------------------------------------------
bool HasSkillsDir(int i) { return kItems[i].skillsDir && kItems[i].skillsDir[0]; }

std::wstring SkillsTargetDir(int i)
{
    std::wstring home = HomeDir();
    return HasSkillsDir(i) && !home.empty() ? home + L"\\" + kItems[i].skillsDir : std::wstring();
}

// 从 RCDATA 资源释放一个 SKILL.md 到目标目录
static bool ExtractSkill(const SkillDef& sk, const std::wstring& targetDir)
{
    HRSRC res = FindResourceW(hInst, MAKEINTRESOURCEW(sk.resId), RT_RCDATA);
    HGLOBAL h = res ? LoadResource(hInst, res) : nullptr;
    if (!h) return false;
    DWORD size = SizeofResource(hInst, res);
    const char* data = static_cast<const char*>(LockResource(h));
    if (!data || size == 0) return false;

    std::wstring dir = targetDir + L"\\" + sk.dirName;
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
    return WriteAtomic(dir + L"\\SKILL.md", std::string(data, size));
}

// 记录安装了哪些 skill（存在 state 目录下 <key>-skills.dat，每行一个文件夹名）
static std::wstring SkillsStateFile(int i) { return StateDir() + L"\\" + kItems[i].key + L"-skills.dat"; }

static std::vector<std::wstring> LoadInstalledSkillNames(int i)
{
    std::vector<std::wstring> out;
    std::string raw;
    if (ReadBytes(SkillsStateFile(i), raw) != kReadOk) return out;
    size_t p = 0;
    while (p < raw.size())
    {
        size_t nl = raw.find('\n', p);
        if (nl == std::string::npos) nl = raw.size();
        std::string line = raw.substr(p, nl - p);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty())
        {
            int need = MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.size(), nullptr, 0);
            std::wstring w(need, 0);
            MultiByteToWideChar(CP_UTF8, 0, line.c_str(), (int)line.size(), w.data(), need);
            out.push_back(w);
        }
        p = nl + 1;
    }
    return out;
}

static bool SaveInstalledSkillNames(int i, const std::vector<std::wstring>& names)
{
    SHCreateDirectoryExW(nullptr, StateDir().c_str(), nullptr);
    std::string data;
    for (const auto& n : names)
    {
        int need = WideCharToMultiByte(CP_UTF8, 0, n.c_str(), (int)n.size(), nullptr, 0, nullptr, nullptr);
        std::string u(need, 0);
        WideCharToMultiByte(CP_UTF8, 0, n.c_str(), (int)n.size(), u.data(), need, nullptr, nullptr);
        data += u + "\n";
    }
    return WriteAtomic(SkillsStateFile(i), data);
}

int InstallSkills(int i)
{
    std::wstring dir = SkillsTargetDir(i);
    if (dir.empty()) return 0;
    int count = 0;
    std::vector<std::wstring> names;
    for (int s = 0; s < kSkillCount; ++s)
    {
        if (ExtractSkill(kSkills[s], dir))
        {
            names.push_back(kSkills[s].dirName);
            ++count;
        }
    }
    if (!names.empty()) SaveInstalledSkillNames(i, names);
    return count;
}

int UninstallSkills(int i)
{
    std::wstring dir = SkillsTargetDir(i);
    if (dir.empty()) return 0;
    auto names = LoadInstalledSkillNames(i);
    int count = 0;
    for (const auto& name : names)
    {
        std::wstring skillDir = dir + L"\\" + name;
        std::wstring skillFile = skillDir + L"\\SKILL.md";
        DeleteFileW(skillFile.c_str());
        RemoveDirectoryW(skillDir.c_str());     // 只删空目录，有其他文件就留着
        ++count;
    }
    DeleteFileW(SkillsStateFile(i).c_str());
    return count;
}

// 按 skill 名卸载：遍历所有支持 skills 的 AI 工具，只删 state 里记录过的（即我们自己装的），
// 不动用户自己放的同名 skill。返回从几个工具里删除了。
int UninstallSkillByName(const wchar_t* skillName)
{
    if (!skillName || !*skillName) return 0;
    std::wstring target(skillName);
    int toolCount = 0;
    for (int i = 0; i < kItemCount; ++i)
    {
        if (!HasSkillsDir(i)) continue;
        auto names = LoadInstalledSkillNames(i);
        auto it = std::find(names.begin(), names.end(), target);
        if (it == names.end()) continue;    // 这个工具不是我们装的（或没装），跳过

        std::wstring dir = SkillsTargetDir(i);
        if (dir.empty()) continue;

        std::wstring skillDir = dir + L"\\" + target;
        DeleteFileW((skillDir + L"\\SKILL.md").c_str());
        RemoveDirectoryW(skillDir.c_str());     // 只删空目录；有其他文件就留着

        names.erase(it);
        if (names.empty())
            DeleteFileW(SkillsStateFile(i).c_str());
        else
            SaveInstalledSkillNames(i, names);

        ++toolCount;
    }
    return toolCount;
}

std::vector<SkillStatus> GetSkillStatuses()
{
    std::vector<SkillStatus> out;
    for (int s = 0; s < kSkillCount; ++s)
    {
        int count = 0;
        for (int i = 0; i < kItemCount; ++i)
        {
            if (!HasSkillsDir(i)) continue;
            std::wstring dir = SkillsTargetDir(i);
            std::wstring file = dir + L"\\" + kSkills[s].dirName + L"\\SKILL.md";
            DWORD a = GetFileAttributesW(file.c_str());
            if (a != INVALID_FILE_ATTRIBUTES && !(a & FILE_ATTRIBUTE_DIRECTORY)) ++count;
        }
        out.push_back({ kSkills[s].dirName, count });
    }
    return out;
}

// ---------------------------------------------------------------------------
// 注入（替换模式：覆盖整个文件） + 自动安装 skills
// ---------------------------------------------------------------------------
bool Install(int i, std::wstring& err)
{
    std::wstring path = TargetPath(i);
    if (path.empty()) { err = L"\u8FD9\u4E2A\u5E94\u7528\u6682\u4E0D\u652F\u6301\u81EA\u52A8\u6CE8\u5165"; return false; }
    if (g_promptSel < 0 || g_promptSel >= (int)g_prompts.size()) { err = T(S_ERR_NO_PROMPT_SEL); return false; }

    std::wstring promptPath = PromptDir() + L"\\" + g_prompts[g_promptSel];
    std::string prompt;
    if (ReadBytes(promptPath, prompt) != kReadOk) { err = T(S_ERR_READ_PROMPT); return false; }
    StripBom(prompt);
    if (IsUtf16(prompt)) { err = T(S_ERR_UTF16_PROMPT); return false; }
    if (prompt.empty()) { err = T(S_ERR_EMPTY_PROMPT); return false; }

    // 检查目标文件当前状态
    std::string doc;
    ReadResult rr = ReadBytes(path, doc);
    if (rr == kReadFail) { err = T(S_ERR_READ_TARGET); return false; }
    bool existed = rr == kReadOk;

    // 备份当前文件
    std::wstring backupFile = MakeBackup(i, existed);

    // 只在首次注入时记录状态（保留最原始的备份）
    if (!HasState(i))
    {
        TargetState ts;
        ts.backupFile = existed ? backupFile : L"";
        ts.originalExisted = existed;
        ts.installedAt = GetTickCount64();
        if (!SaveState(i, ts)) { err = T(S_ERR_SAVE_STATE); return false; }
    }

    // 写入：替换模式，直接覆盖
    // Cursor 的 .mdc 格式需要 YAML frontmatter
    std::wstring dir = path.substr(0, path.find_last_of(L'\\'));
    SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
    std::string output = prompt;
    if (path.size() > 4 && _wcsicmp(path.c_str() + path.size() - 4, L".mdc") == 0)
    {
        output = "---\ndescription: Ua 工作规范（自动注入 · 全局生效）\nglobs: \"**/*\"\nalwaysApply: true\n---\n" + prompt;
    }
    if (!WriteAtomic(path, output))
    {
        err = T(S_ERR_WRITE_TARGET);
        return false;
    }

    // 自动安装 skills
    if (HasSkillsDir(i))
        InstallSkills(i);

    return true;
}

// ---------------------------------------------------------------------------
// 卸载（恢复到首次注入前的状态）
// ---------------------------------------------------------------------------
bool Uninstall(int i, std::wstring& err)
{
    TargetState ts;
    if (!LoadState(i, ts))
    {
        err = T(S_ERR_NO_STATE);
        return false;
    }

    if (!RestoreBackup(i, ts))
    {
        err = T(S_ERR_RESTORE);
        return false;
    }

    // 卸载 skills
    if (HasSkillsDir(i))
        UninstallSkills(i);

    DeleteState(i);
    return true;
}
