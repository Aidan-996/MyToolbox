// mybox.cpp : MyBox 命令行版（Linux / macOS）—— AI 编程工具提示词/Skills 一键安装
//
// 与 Windows 版（MyBox/）共用 shared/ 下的同一套 prompts/ 与 skills/ 数据文件，注入/备份/恢复
// 的行为与 Windows 版保持一致：
//   - 注入 = 把提示词内容整体写入目标工具的全局配置文件（替换式，非追加）
//   - 首次注入前把原文件备份到 $DATA/backups/<key>/，状态记录在 $DATA/state/<key>.dat
//   - 卸载 = 从备份恢复原文件（原本不存在则删除）
//   - Skills = 把 skills/<name>/SKILL.md 复制到 <home>/<skillsDir>/<name>/SKILL.md，
//     卸载时只删除自己记录过的名字，不碰用户自放的文件
//
// $DATA 默认：macOS 为 ~/Library/Application Support/MyBox，Linux 为 ~/.local/share/mybox，
// 可用环境变量 MYBOX_DATA 覆盖（兼容旧变量 UA_DATA）。
// 编译：g++ -std=c++17 -O2 -Wall -Wextra -o mybox mybox.cpp
//   macOS 通用二进制：g++ -std=c++17 -O2 -arch arm64 -arch x86_64 -o mybox mybox.cpp

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>     // _NSGetExecutablePath
#endif

static const char* kVersion = "1.0.4";
static const char* kPromptsApiUrl =
    "https://api.github.com/repos/Aidan-996/MyToolbox/contents/prompts";
static const size_t kMaxFile = 4 * 1024 * 1024;   // 单文件上限，与 Windows 版一致
static const int kMaxBackups = 200;

// ---------------------------------------------------------------------------
// 支持的工具（相对 $HOME 的路径，与 Windows 版 config.h 的 kItems 对齐；
// WorkBuddy/豆包在 Windows 版就不支持自动注入，这里不列）
// ---------------------------------------------------------------------------
struct Tool
{
    const char* key;        // 状态文件名；prompts 首选 prompts/<key>.md
    const char* name;       // 显示名
    const char* target;     // 全局提示词文件，相对 $HOME
    const char* skillsDir;  // skills 目录，相对 $HOME；nullptr 表示不支持 skills
};

static const Tool kTools[] = {
    { "claude",    "Claude Code",                ".claude/CLAUDE.md",                          ".claude/skills"   },
    { "codex",     "Codex (OpenAI)",             ".codex/AGENTS.md",                           ".codex/skills"    },
    { "gemini",    "Gemini CLI",                 ".gemini/GEMINI.md",                          nullptr            },
    { "grokbuild", "Grok Build",                 ".grok/AGENTS.md",                            nullptr            },
    { "zcode",     "ZCode",                      ".zcode/AGENTS.md",                           ".zcode/skills"    },
    { "deepseek",  "DeepSeek Harness",           ".dsh/AGENTS.md",                             ".dsh/skills"      },
    { "opencode",  "OpenCode",                   ".config/opencode/AGENTS.md",                 nullptr            },
    { "hermes",    "Hermes",                     ".hermes/SOUL.md",                            nullptr            },
    { "mcode",     "MiniMax Code",               ".minimax/AGENTS.md",                         nullptr            },
    { "cursor",    "Cursor",                     ".cursor/rules/Ua.mdc",                       nullptr            },
    { "vscode",    "VS Code Copilot",            ".copilot/copilot-instructions.md",           nullptr            },
    { "devin",     "Devin Desktop (Windsurf)",   ".codeium/windsurf/memories/global_rules.md", nullptr            },
    { "codebuddy", "CodeBuddy Code",             ".codebuddy/CODEBUDDY.md",                    ".codebuddy/skills" },
};

static const Tool* FindTool(const std::string& key)
{
    for (const Tool& t : kTools)
        if (key == t.key) return &t;
    return nullptr;
}

// ---------------------------------------------------------------------------
// 基础文件操作
// ---------------------------------------------------------------------------
static bool Exists(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0;
}

static bool IsDir(const std::string& path)
{
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

static bool MkdirP(const std::string& path)
{
    if (path.empty() || IsDir(path)) return true;
    std::string cur;
    size_t i = 0;
    if (path[0] == '/') { cur = "/"; i = 1; }
    while (i <= path.size())
    {
        size_t next = path.find('/', i);
        if (next == std::string::npos) next = path.size();
        if (next > i)
        {
            cur += path.substr(i, next - i);
            if (!IsDir(cur) && ::mkdir(cur.c_str(), 0755) != 0 && !IsDir(cur))
                return false;
            cur += "/";
        }
        i = next + 1;
    }
    return IsDir(path);
}

static bool ReadFile(const std::string& path, std::string& out)
{
    FILE* f = ::fopen(path.c_str(), "rb");
    if (!f) return false;
    out.clear();
    char buf[64 * 1024];
    size_t n;
    while ((n = ::fread(buf, 1, sizeof(buf), f)) > 0)
    {
        out.append(buf, n);
        if (out.size() > kMaxFile) { ::fclose(f); return false; }
    }
    bool ok = !::ferror(f);
    ::fclose(f);
    return ok;
}

// 原子写：写同目录 .mybox.tmp -> rename 覆盖，避免半截文件
static bool WriteFileAtomic(const std::string& path, const std::string& data)
{
    std::string tmp = path + ".mybox.tmp";
    FILE* f = ::fopen(tmp.c_str(), "wb");
    if (!f) return false;
    bool ok = data.empty() || ::fwrite(data.data(), 1, data.size(), f) == data.size();
    ok = ok && ::fflush(f) == 0 && ::fsync(::fileno(f)) == 0;
    ok = ok && ::fclose(f) == 0;
    if (ok) ok = ::rename(tmp.c_str(), path.c_str()) == 0;
    if (!ok) ::remove(tmp.c_str());
    return ok;
}

static bool CopyFileTo(const std::string& from, const std::string& to)
{
    std::string data;
    if (!ReadFile(from, data)) return false;
    return WriteFileAtomic(to, data);
}

static std::vector<std::string> ListDir(const std::string& path)
{
    std::vector<std::string> out;
    DIR* d = ::opendir(path.c_str());
    if (!d) return out;
    struct dirent* e;
    while ((e = ::readdir(d)) != nullptr)
    {
        if (strcmp(e->d_name, ".") == 0 || strcmp(e->d_name, "..") == 0) continue;
        out.push_back(e->d_name);
    }
    ::closedir(d);
    std::sort(out.begin(), out.end());
    return out;
}

static std::string Join(const std::string& a, const std::string& b)
{
    if (a.empty()) return b;
    if (!b.empty() && b[0] == '/') return b;
    return a.back() == '/' ? a + b : a + "/" + b;
}

static std::string DirName(const std::string& path)
{
    size_t p = path.find_last_of('/');
    return p == std::string::npos ? std::string(".") : (p == 0 ? "/" : path.substr(0, p));
}

static std::string NowStamp()
{
    char buf[32];
    std::time_t t = ::time(nullptr);
    std::tm tmv{};
    ::localtime_r(&t, &tmv);
    ::strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", &tmv);
    return buf;
}

static std::string HomeDir()
{
    const char* h = ::getenv("HOME");
    return (h && *h) ? h : std::string();
}

// 数据目录：$MYBOX_DATA（兼容旧 $UA_DATA）优先；macOS 用 Application Support，Linux 用 XDG 惯例
static std::string DataDir()
{
    const char* env = ::getenv("MYBOX_DATA");
    if (!env || !*env) env = ::getenv("UA_DATA");
    if (env && *env) return env;
#ifdef __APPLE__
    return Join(HomeDir(), "Library/Application Support/MyBox");
#else
    return Join(HomeDir(), ".local/share/mybox");
#endif
}

static std::string SelfDir()
{
#ifdef __APPLE__
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0)
        return DirName(buf);
    return {};
#else
    char buf[4096];
    ssize_t n = ::readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (n <= 0) return {};
    buf[n] = '\0';
    return DirName(buf);
#endif
}

// prompts/ 和 skills/ 数据目录查找顺序：
//   $MYBOX_DATA/$UA_DATA → 平台默认数据目录 → <exe>/../share/mybox（make install）
//   → <exe>/data → <exe>/../shared（仓库里直接 ./cli/mybox 运行）
static bool FindDataSub(const char* sub, std::string& out)
{
    std::string self = SelfDir();
    std::vector<std::string> cands;
    std::string def = DataDir();
    if (!def.empty()) cands.push_back(Join(def, sub));
    if (!self.empty())
    {
        cands.push_back(Join(Join(DirName(self), "share/mybox"), sub));
        cands.push_back(Join(Join(self, "data"), sub));
        cands.push_back(Join(Join(DirName(self), "shared"), sub));
    }
    for (const std::string& c : cands)
        if (IsDir(c)) { out = c; return true; }
    return false;
}

// skill 目录名 / 提示词文件名的安全校验（防路径穿越，与 Windows 版一致）
static bool IsSafeName(const std::string& s, bool requireMd)
{
    if (s.empty() || s.size() > 100) return false;
    if (s[0] == '.') return false;
    if (s.find("..") != std::string::npos) return false;
    for (char c : s)
        if (!(std::isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.')) return false;
    return requireMd ? (s.size() > 3 && s.compare(s.size() - 3, 3, ".md") == 0) : true;
}

// ---------------------------------------------------------------------------
// 注入状态（$DATA/state/<key>.dat，两行：backupFile / originalExisted。
// 旧版文件还有第三行 installedAt，读取时忽略）
// ---------------------------------------------------------------------------
struct TargetState
{
    std::string backupFile;
    bool originalExisted = false;
};

static std::string StateFile(const std::string& key) { return Join(DataDir(), "state/" + key + ".dat"); }
static std::string SkillsStateFile(const std::string& key) { return Join(DataDir(), "state/" + key + "-skills.dat"); }
static std::string BackupDirFor(const std::string& key) { return Join(DataDir(), "backups/" + key); }

static bool LoadState(const std::string& key, TargetState& ts)
{
    std::string raw;
    if (!ReadFile(StateFile(key), raw)) return false;
    size_t p1 = raw.find('\n');
    if (p1 == std::string::npos) return false;
    ts.backupFile = raw.substr(0, p1);
    if (!ts.backupFile.empty() && ts.backupFile.back() == '\r') ts.backupFile.pop_back();
    ts.originalExisted = p1 + 1 < raw.size() && raw[p1 + 1] == '1';
    return true;
}

static bool SaveState(const std::string& key, const TargetState& ts)
{
    MkdirP(Join(DataDir(), "state"));
    return WriteFileAtomic(StateFile(key), ts.backupFile + "\n" + (ts.originalExisted ? "1" : "0") + "\n");
}

static bool HasState(const std::string& key) { return Exists(StateFile(key)); }

static bool DeleteState(const std::string& key)
{
    return ::remove(StateFile(key).c_str()) == 0;
}

static std::string MakeBackup(const std::string& key, const std::string& targetPath, bool fileExists)
{
    std::string dir = BackupDirFor(key);
    MkdirP(dir);
    std::string dst = Join(dir, NowStamp() + ".bak");
    if (fileExists && !CopyFileTo(targetPath, dst)) return {};
    // 限制备份数量：超了就删最旧的
    std::vector<std::string> all = ListDir(dir);
    while ((int)all.size() > kMaxBackups)
    {
        ::remove(Join(dir, all.front()).c_str());
        all.erase(all.begin());
    }
    return dst;
}

// ---------------------------------------------------------------------------
// 注入 / 卸载
// ---------------------------------------------------------------------------
static bool InstallPrompt(const Tool& t, const std::string& promptPath, std::string& err)
{
    std::string prompt;
    if (!ReadFile(promptPath, prompt)) { err = "failed to read prompt file: " + promptPath; return false; }
    if (prompt.empty()) { err = "prompt file is empty"; return false; }

    std::string target = Join(HomeDir(), t.target);
    bool existed = Exists(target);

    // 只在首次注入时记录状态（保留最原始的备份）
    if (!HasState(t.key))
    {
        TargetState ts;
        ts.originalExisted = existed;
        if (existed)
        {
            ts.backupFile = MakeBackup(t.key, target, existed);
            if (ts.backupFile.empty()) { err = "failed to backup " + target; return false; }
        }
        if (!SaveState(t.key, ts)) { err = "failed to save state"; return false; }
    }
    else
    {
        // 已有状态也要留一份当前内容的滚动备份
        MakeBackup(t.key, target, existed);
    }

    // Cursor 的 .mdc 格式需要 YAML frontmatter（与 Windows 版一致）
    std::string targetRel = t.target;
    if (targetRel.size() > 4 && targetRel.compare(targetRel.size() - 4, 4, ".mdc") == 0)
        prompt = "---\ndescription: MyBox working rules (auto-injected, global)\nglobs: \"**/*\"\nalwaysApply: true\n---\n" + prompt;

    MkdirP(DirName(target));
    if (!WriteFileAtomic(target, prompt)) { err = "failed to write " + target; return false; }
    return true;
}

static bool UninstallPrompt(const Tool& t, std::string& err)
{
    TargetState ts;
    if (!LoadState(t.key, ts)) { err = std::string("not installed via mybox (no state): ") + t.key; return false; }

    std::string target = Join(HomeDir(), t.target);
    bool ok = true;
    if (ts.originalExisted)
        ok = CopyFileTo(ts.backupFile, target);
    else
        ok = ::remove(target.c_str()) == 0;

    if (!ok) { err = "failed to restore " + target; return false; }
    DeleteState(t.key);
    return true;
}

// ---------------------------------------------------------------------------
// Skills
// ---------------------------------------------------------------------------
static std::vector<std::string> AvailableSkills()
{
    std::vector<std::string> out;
    std::string dir;
    if (FindDataSub("skills", dir))
        for (const std::string& name : ListDir(dir))
            if (IsDir(Join(dir, name)) && IsSafeName(name, false) && Exists(Join(Join(dir, name), "SKILL.md")))
                out.push_back(name);
    return out;
}

static std::vector<std::string> LoadInstalledSkills(const std::string& key)
{
    std::vector<std::string> out;
    std::string raw;
    if (!ReadFile(SkillsStateFile(key), raw)) return out;
    size_t p = 0;
    while (p < raw.size())
    {
        size_t nl = raw.find('\n', p);
        if (nl == std::string::npos) nl = raw.size();
        std::string line = raw.substr(p, nl - p);
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (!line.empty()) out.push_back(line);
        p = nl + 1;
    }
    return out;
}

static bool SaveInstalledSkills(const std::string& key, const std::vector<std::string>& names)
{
    MkdirP(Join(DataDir(), "state"));
    std::string data;
    for (const auto& n : names) data += n + "\n";
    return WriteFileAtomic(SkillsStateFile(key), data);
}

// 返回安装个数
static int InstallSkills(const Tool& t, const std::string* onlyOne, std::string& err)
{
    if (!t.skillsDir) { err = std::string(t.name) + " does not support skills"; return -1; }
    std::string src;
    if (!FindDataSub("skills", src)) { err = "skills data dir not found (set MYBOX_DATA or run make install)"; return -1; }

    std::vector<std::string> names;
    if (onlyOne) names.push_back(*onlyOne);
    else names = AvailableSkills();
    if (names.empty()) { err = "no skills available in data dir"; return -1; }

    std::string dstRoot = Join(HomeDir(), t.skillsDir);
    std::vector<std::string> installed = LoadInstalledSkills(t.key);
    int count = 0;
    for (const std::string& name : names)
    {
        std::string skillMd = Join(Join(src, name), "SKILL.md");
        if (!Exists(skillMd)) continue;
        std::string dstDir = Join(dstRoot, name);
        if (!MkdirP(dstDir)) { err = "cannot create " + dstDir; return count; }
        if (!CopyFileTo(skillMd, Join(dstDir, "SKILL.md"))) { err = "failed to write " + dstDir; return count; }
        if (std::find(installed.begin(), installed.end(), name) == installed.end())
            installed.push_back(name);
        ++count;
    }
    if (!installed.empty()) SaveInstalledSkills(t.key, installed);
    return count;
}

// 返回卸载个数
static int UninstallSkills(const Tool& t, const std::string* onlyOne, std::string& err)
{
    if (!t.skillsDir) { err = std::string(t.name) + " does not support skills"; return -1; }
    std::vector<std::string> installed = LoadInstalledSkills(t.key);
    if (installed.empty()) { err = "no skills installed for " + std::string(t.key); return 0; }

    std::string dstRoot = Join(HomeDir(), t.skillsDir);
    std::vector<std::string> remain;
    int count = 0;
    for (const std::string& name : installed)
    {
        bool removeIt = !onlyOne || *onlyOne == name;
        if (!removeIt) { remain.push_back(name); continue; }
        std::string skillDir = Join(dstRoot, name);
        std::string skillFile = Join(skillDir, "SKILL.md");
        // 只删自己装过的 SKILL.md 和空目录，不碰用户自放的文件
        ::remove(skillFile.c_str());
        ::rmdir(skillDir.c_str());
        ++count;
    }
    if (onlyOne && count == 0)
    {
        err = "skill not installed here: " + *onlyOne;
        return 0;
    }
    if (remain.empty())
        ::remove(SkillsStateFile(t.key).c_str());
    else
        SaveInstalledSkills(t.key, remain);
    return count;
}

// ---------------------------------------------------------------------------
// prompts 云同步（curl 拉 GitHub Contents API，行为与 Windows 版一致）
// ---------------------------------------------------------------------------
static bool HttpGet(const std::string& url, std::string& out)
{
    std::string cmd = "curl -sf --max-time 20 '" + url + "'";
    FILE* f = ::popen(cmd.c_str(), "r");
    if (!f) return false;
    out.clear();
    char buf[64 * 1024];
    size_t n;
    while ((n = ::fread(buf, 1, sizeof(buf), f)) > 0) out.append(buf, n);
    int rc = ::pclose(f);
    return rc == 0 && !out.empty();
}

// 从 json[from..] 找 "key":"value"，返回 value，next 回传扫描位置
static std::string JsonFindString(const std::string& json, size_t from, const char* key, size_t& next)
{
    std::string pat = "\"";
    pat += key;
    pat += "\"";
    size_t p = json.find(pat, from);
    if (p == std::string::npos) { next = from; return {}; }
    p += pat.size();
    while (p < json.size() && (json[p] == ' ' || json[p] == '\t' || json[p] == ':')) ++p;
    if (p >= json.size() || json[p] != '"') { next = p; return {}; }
    ++p;
    size_t end = p;
    while (end < json.size() && json[end] != '"')
    {
        if (json[end] == '\\' && end + 1 < json.size()) end += 2;
        else ++end;
    }
    if (end >= json.size()) { next = end; return {}; }
    next = end + 1;
    return json.substr(p, end - p);
}

static bool LoadShaCache(std::vector<std::pair<std::string, std::string>>& out)
{
    std::string raw;
    if (!ReadFile(Join(DataDir(), "prompts-sync.json"), raw)) return false;
    size_t p = 0;
    while (p < raw.size())
    {
        size_t nl = raw.find('\n', p);
        if (nl == std::string::npos) nl = raw.size();
        size_t eq = raw.find('=', p);
        if (eq < nl)
            out.push_back({ raw.substr(p, eq - p), raw.substr(eq + 1, nl - eq - 1) });
        p = nl + 1;
    }
    return true;
}

static void SaveShaCache(const std::vector<std::pair<std::string, std::string>>& cache)
{
    std::string data;
    for (const auto& e : cache) data += e.first + "=" + e.second + "\n";
    MkdirP(DataDir());
    WriteFileAtomic(Join(DataDir(), "prompts-sync.json"), data);
}

// 返回更新的文件数，-1 表示整体失败
static int SyncPrompts()
{
    std::string json;
    if (!HttpGet(kPromptsApiUrl, json)) return -1;

    std::vector<std::pair<std::string, std::string>> cache;
    LoadShaCache(cache);

    std::string promptsDir;
    if (!FindDataSub("prompts", promptsDir)) { fprintf(stderr, "prompts data dir not found (set MYBOX_DATA)\n"); return -1; }

    size_t p = 0, next;
    int changes = 0;
    while (p < json.size())
    {
        std::string name = JsonFindString(json, p, "name", next);
        if (name.empty()) break;
        p = next;
        std::string sha = JsonFindString(json, p, "sha", next);
        if (sha.empty()) break;
        p = next;
        std::string dl = JsonFindString(json, p, "download_url", next);
        if (dl.empty()) break;
        p = next;
        if (!IsSafeName(name, true)) continue;

        bool known = false;
        for (const auto& e : cache)
            if (e.first == name && e.second == sha) { known = true; break; }
        if (known) continue;

        std::string body;
        if (!HttpGet(dl, body) || body.empty()) continue;
        if (!WriteFileAtomic(Join(promptsDir, name), body)) continue;

        bool replaced = false;
        for (auto& e : cache)
            if (e.first == name) { e.second = sha; replaced = true; }
        if (!replaced) cache.push_back({ name, sha });
        printf("updated: %s\n", name.c_str());
        ++changes;
    }
    if (changes > 0) SaveShaCache(cache);
    return changes;
}

// ---------------------------------------------------------------------------
// 命令
// ---------------------------------------------------------------------------
static int CmdList()
{
    printf("%-10s %-26s %s\n", "state", "tool", "target");
    for (const Tool& t : kTools)
    {
        std::string target = Join(HomeDir(), t.target);
        bool dirOk = IsDir(Join(HomeDir(), std::string(t.target).substr(0, std::string(t.target).find('/'))));
        const char* st = "[-]";
        if (HasState(t.key)) st = "[installed]";
        else
        {
            std::string doc;
            if (ReadFile(target, doc) && !doc.empty()) st = "[content]";
        }
        if (!dirOk) st = "[not detected]";
        printf("%-12s %-26s %s\n", st, (std::string(t.name) + " (" + t.key + ")").c_str(), t.target);
    }
    return 0;
}

static int CmdInstall(const std::string& key, const std::string& fileArg)
{
    const Tool* t = FindTool(key);
    if (!t) { fprintf(stderr, "unknown tool: %s (see 'mybox list')\n", key.c_str()); return 2; }

    std::string promptPath = fileArg;
    if (promptPath.empty())
    {
        std::string promptsDir;
        if (!FindDataSub("prompts", promptsDir))
        {
            fprintf(stderr, "prompts data dir not found (set MYBOX_DATA or run make install)\n");
            return 1;
        }
        std::string own = Join(promptsDir, std::string(t->key) + ".md");
        std::string def = Join(promptsDir, "default.md");
        std::vector<std::string> mds;
        for (const std::string& n : ListDir(promptsDir))
            if (IsSafeName(n, true)) mds.push_back(n);
        if (Exists(own)) promptPath = own;
        else if (Exists(def)) promptPath = def;
        else if (mds.size() == 1) promptPath = Join(promptsDir, mds[0]);
        else
        {
            fprintf(stderr, "multiple prompts available, pick one:\n");
            for (const std::string& n : mds) fprintf(stderr, "  mybox install %s %s\n", key.c_str(), n.c_str());
            return 1;
        }
    }
    else if (!Exists(promptPath))
    {
        fprintf(stderr, "prompt file not found: %s\n", promptPath.c_str());
        return 1;
    }

    std::string err;
    if (!InstallPrompt(*t, promptPath, err)) { fprintf(stderr, "install failed: %s\n", err.c_str()); return 1; }
    printf("installed to %s (%s)\n", Join(HomeDir(), t->target).c_str(), promptPath.c_str());
    printf("uninstall anytime with: mybox uninstall %s\n", key.c_str());
    return 0;
}

static int CmdUninstall(const std::string& key)
{
    const Tool* t = FindTool(key);
    if (!t) { fprintf(stderr, "unknown tool: %s\n", key.c_str()); return 2; }
    std::string err;
    if (!UninstallPrompt(*t, err)) { fprintf(stderr, "uninstall failed: %s\n", err.c_str()); return 1; }
    printf("restored %s\n", Join(HomeDir(), t->target).c_str());
    return 0;
}

static int CmdSkills(const std::vector<std::string>& args)
{
    if (args.empty() || args[0] == "list")
    {
        std::vector<std::string> avail = AvailableSkills();
        printf("available skills: %zu (data dir)\n", avail.size());
        if (args.size() >= 2)
        {
            const Tool* t = FindTool(args[1]);
            if (!t) { fprintf(stderr, "unknown tool: %s\n", args[1].c_str()); return 2; }
            std::vector<std::string> inst = LoadInstalledSkills(t->key);
            printf("installed in %s: %zu\n", t->key, inst.size());
            for (const auto& n : inst) printf("  [x] %s\n", n.c_str());
        }
        else
        {
            for (const Tool& t : kTools)
            {
                if (!t.skillsDir) continue;
                printf("  %s: %zu installed\n", t.key, LoadInstalledSkills(t.key).size());
            }
        }
        return 0;
    }

    const std::string& op = args[0];
    if ((op == "install" || op == "uninstall") && args.size() >= 2)
    {
        const Tool* t = FindTool(args[1]);
        if (!t) { fprintf(stderr, "unknown tool: %s\n", args[1].c_str()); return 2; }
        std::string only;
        std::string* onlyPtr = nullptr;
        if (args.size() >= 3 && args[2] != "all")
        {
            only = args[2];
            if (!IsSafeName(only, false)) { fprintf(stderr, "invalid skill name: %s\n", only.c_str()); return 2; }
            onlyPtr = &only;
        }
        std::string err;
        int n = op == "install" ? InstallSkills(*t, onlyPtr, err) : UninstallSkills(*t, onlyPtr, err);
        if (n < 0) { fprintf(stderr, "skills %s failed: %s\n", op.c_str(), err.c_str()); return 1; }
        if (!err.empty()) fprintf(stderr, "warning: %s\n", err.c_str());
        printf("%s %zu skill(s) %s %s\n", op == "install" ? "installed" : "uninstalled",
               (size_t)n, op == "install" ? "into" : "from", t->key);
        return 0;
    }

    fprintf(stderr, "usage: mybox skills list [tool] | mybox skills install|uninstall <tool> [name|all]\n");
    return 2;
}

static void Usage()
{
    printf("MyBox %s — one-click prompt/skill installer for AI coding tools\n\n", kVersion);
    printf("usage:\n");
    printf("  mybox list                          show tools, detection and install state\n");
    printf("  mybox install <tool> [prompt.md]    install prompt (backup made automatically)\n");
    printf("  mybox uninstall <tool>              restore original config\n");
    printf("  mybox skills list [tool]            available / installed skills\n");
    printf("  mybox skills install <tool> [n|all] install skills into <tool>\n");
    printf("  mybox skills uninstall <tool> [n]   remove skills\n");
    printf("  mybox sync                          pull new prompts from GitHub\n");
    printf("  mybox version                       print version\n");
    printf("\ntools: ");
    for (const Tool& t : kTools) printf("%s ", t.key);
    printf("\n\ndata dir: %s (override with MYBOX_DATA)\n", DataDir().c_str());
    printf("uninstall restores the original file; skills removal never touches user-added files.\n");
}

int main(int argc, char** argv)
{
    std::vector<std::string> args(argv + 1, argv + argc);
    if (args.empty() || args[0] == "help" || args[0] == "--help" || args[0] == "-h") { Usage(); return 0; }
    if (args[0] == "version" || args[0] == "--version") { printf("mybox %s\n", kVersion); return 0; }
    if (HomeDir().empty()) { fprintf(stderr, "HOME is not set\n"); return 1; }

    const std::string& cmd = args[0];
    if (cmd == "list") return CmdList();
    if (cmd == "install" || cmd == "uninstall")
    {
        if (args.size() < 2) { fprintf(stderr, "usage: mybox %s <tool>\n", cmd.c_str()); return 2; }
        return cmd == "install" ? CmdInstall(args[1], args.size() >= 3 ? args[2] : "")
                                : CmdUninstall(args[1]);
    }
    if (cmd == "skills") return CmdSkills(std::vector<std::string>(args.begin() + 1, args.end()));
    if (cmd == "sync")
    {
        int n = SyncPrompts();
        if (n < 0) { fprintf(stderr, "sync failed (network or data dir)\n"); return 1; }
        printf("sync complete: %d prompt(s) updated\n", n);
        return 0;
    }

    fprintf(stderr, "unknown command: %s (see 'mybox help')\n", cmd.c_str());
    return 2;
}
