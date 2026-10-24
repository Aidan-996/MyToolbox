// ui-main.cpp : 主界面布局、卡片绘制、命中测试与 Render 入口
//

#include "ui-draw.h"
#include "ui-dialogs.h"

// ---------------------------------------------------------------------------
// 布局：标签栏、右上角按钮、条目卡片
// ---------------------------------------------------------------------------

// 右上角按钮：设置 / 最小化 / 关闭，id 为 kHoverGear..kHoverClose（弹窗里也用，声明在 ui-dialogs.h）
D2D1_RECT_F ButtonRect(int id)
{
    float right = kW - kPad - 4.f - (kHoverClose - id) * 32.f;
    return R(right - 30.f, kPad + 4.f, right, kPad + 32.f);
}

// 分段式标签（segmented control）：头部第二行，两段等宽
static D2D1_RECT_F TabRect(int i)
{
    float x = kPad + 4.f + i * 108.f;
    return R(x, 58.f, x + 104.f, 88.f);
}

struct Slot
{
    D2D1_RECT_F rect;
    int item;
};

// 当前标签对应分组的条目：编辑器 4 列竖版卡片，工具 2 列横版卡片
static void Layout(std::vector<Slot>& slots)
{
    float y = kTop;
    bool editor = Group(g_tab) == kGroupEditor;
    int cols = editor ? 4 : 2;
    float ch = editor ? kCardVH : kCardH;
    float cw = (kW - 2 * kPad - (cols - 1) * kGap) / cols;
    int col = 0;
    for (int i = 0; i < kItemCount; ++i)
    {
        if (kItems[i].group != Group(g_tab)) continue;
        float x = kPad + col * (cw + kGap);
        slots.push_back({ R(x, y, x + cw, y + ch), i });
        if (++col == cols) { col = 0; y += ch + kGap; }
    }
}

// ---------------------------------------------------------------------------
// 命中测试
// ---------------------------------------------------------------------------
int HitTest(float x, float y)
{
    if (g_disclaimer)
    {
        if (InRect(DiscAcceptRect(), x, y)) return kHoverAccept;
        return -2;  // block everything else
    }
    if (g_aboutOpen)
    {
        if (InRect(AboutCloseRect(), x, y)) return kHoverAboutClose;
        if (!InRect(AboutRect(), x, y)) return kHoverDlgOut;
        if (InRect(AboutGithubRect(), x, y)) return kHoverAboutGithub;
        if (InRect(AboutTutorialRect(), x, y)) return kHoverAboutTutorial;
        // 更新链接只有在真的发现新版本时才可点击
        if (UpdateAvailable() && InRect(AboutUpdateRect(), x, y)) return kHoverAboutUpdate;
        return kHoverDlgBg;
    }
    if (g_helpOpen)
    {
        if (InRect(HelpCloseRect(), x, y)) return kHoverHelpClose;
        if (!InRect(HelpRect(), x, y)) return kHoverDlgOut;
        return kHoverDlgBg;
    }
    if (g_skillMgr)
    {
        D2D1_RECT_F d = SkillDlgRect();
        if (!InRect(d, x, y)) return kHoverDlgOut;
        if (InRect(SkillCloseRect(), x, y)) return kHoverSkillClose;
        if (InRect(SkillFolderRect(), x, y)) return kHoverSkillFolder;
        // 分类标签
        D2D1_RECT_F pills[kCatCount];
        SkillCatPills(pills);
        for (int i = 0; i < kCatCount; ++i)
            if (InRect(pills[i], x, y)) return kHoverSkillCat + i;
        // 搜索框（点一下不干啥，键盘输入由钩子处理）
        if (InRect(SkillSearchRect(), x, y)) return kHoverSkillSearch;
        // 过滤后的 skill 列表
        std::vector<int> filtered = FilteredSkills();
        int filteredCount = (int)filtered.size();
        int maxScroll = std::max(0, filteredCount - kSkillMaxVisible);
        int startIdx = std::min(std::max(0, g_skillScroll), maxScroll);
        int visible = std::min(filteredCount - startIdx, kSkillMaxVisible);
        for (int v = 0; v < visible; ++v)
        {
            int idxFilt = startIdx + v;
            if (idxFilt >= filteredCount) break;
            int idx = filtered[idxFilt];
            if (!InRect(SkillItemRect(v), x, y)) continue;
            if (InRect(SkillRemoveRect(v), x, y)) return kHoverSkillRemove + idx;
            return kHoverSkillMgr + idx;
        }
        return kHoverDlgBg;
    }
    if (g_dlg >= 0)
    {
        D2D1_RECT_F d = DlgRect();
        if (!InRect(d, x, y)) return kHoverDlgOut;
        for (int b = 0; b < kDlgButtonCount; ++b)
            if (InRect(DlgButtonRect(b), x, y)) return kHoverDlg + b;
        // 提示词列表
        int visible = std::min((int)g_prompts.size(), kDlgMaxVisible);
        for (int v = 0; v < visible; ++v)
            if (InRect(DlgPromptRect(v), x, y)) return kHoverPrompt + g_promptScroll + v;
        return kHoverDlgBg;
    }
    if (g_menuOpen && InRect(MenuRect(), x, y))
    {
        for (int i = 0; i < kMenuCount; ++i)
            if (kMenu[i].cmd && InRect(MenuItemRect(i), x, y)) return kHoverMenu + i;
        return kHoverMenuBg;
    }
    for (int i = 0; i < kTabCount; ++i)
        if (InRect(TabRect(i), x, y)) return i;
    for (int id = kHoverHelpBtn; id <= kHoverClose; ++id)
        if (InRect(ButtonRect(id), x, y)) return id;
    std::vector<Slot> slots;
    Layout(slots);
    for (const Slot& s : slots)
        if (InRect(s.rect, x, y)) return kHoverCard + s.item;
    return -1;
}

// ---------------------------------------------------------------------------
// 条目卡片绘制（标签页主体内容）
// ---------------------------------------------------------------------------
static void DrawHeader()
{
    // 品牌区：logo + MyBox 字标（此区域 HitTest 返回 -1，可拖动窗口）
    ID2D1SvgDocument* mark = LoadSvgById(IDR_LOGO_UA);
    D2D1_RECT_F logo = R(kPad + 2.f, 18.f, kPad + 28.f, 44.f);
    if (mark) DrawSvgSized(mark, logo, 24.f);
    else Text(L"M", g_fLetter.Get(), logo, C(P.accent), DWRITE_TEXT_ALIGNMENT_CENTER);
    Text(L"MyBox", g_fTitle.Get(), R(kPad + 34.f, 16.f, kPad + 150.f, 44.f), C(P.text));

    // 分段式标签容器 + 两段
    Fill(R(kPad, 54.f, kPad + 224.f, 92.f), Ov(0.06f), 10.f);
    for (int i = 0; i < kTabCount; ++i)
    {
        D2D1_RECT_F r = TabRect(i);
        if (i == g_tab)
        {
            // 选中段：强调色浅底胶囊
            Fill(r, C(P.accent, P.dark ? 0.22f : 0.16f), 8.f);
            Text(T(i == 0 ? S_TAB_EDITOR : S_TAB_TOOLS), g_fTabSel.Get(), r, C(P.text), DWRITE_TEXT_ALIGNMENT_CENTER);
        }
        else
        {
            if (i == g_hover) Fill(r, Ov(0.05f), 8.f);
            Text(T(i == 0 ? S_TAB_EDITOR : S_TAB_TOOLS), g_fTab.Get(), r, C(i == g_hover ? P.text : P.sub), DWRITE_TEXT_ALIGNMENT_CENTER);
        }
    }

    // 右上角窗口按钮：帮助 / 设置 / 最小化 / 关闭
    const wchar_t* glyphs[] = { L"\uE897", L"\uE713", L"\uE921", L"\uE8BB" };
    for (int id = kHoverHelpBtn; id <= kHoverClose; ++id)
    {
        D2D1_RECT_F r = ButtonRect(id);
        bool hot = g_hover == id || (id == kHoverGear && g_menuOpen);
        bool closeHot = hot && id == kHoverClose;
        if (hot) Fill(r, closeHot ? C(P.closeRed) : Ov(0.08f), 6.f);
        Text(glyphs[id - kHoverHelpBtn], g_fIcon.Get(), r, C(closeHot ? 0xFFFFFF : hot ? P.text : P.sub), DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    // 内容区分隔线
    g_brush->SetColor(Ov(0.08f));
    g_rt->DrawLine(D2D1::Point2F(kPad, kTop - 10.f), D2D1::Point2F(kW - kPad, kTop - 10.f), g_brush.Get(), 1.f);
}

// 需要暴露给 ui-dialogs.cpp 的 DrawDialog 使用（画弹窗标题的小 logo）
void DrawIcon(int i, D2D1_RECT_F tile)
{
    const Item& it = kItems[i];
    ID2D1SvgDocument* doc = it.logo ? Logo(i) : nullptr;

    if (doc && it.framed)
    {
        DrawSvg(doc, tile);
        return;
    }

    Fill(tile, Ov(0.08f), 6.f);
    if (doc)
    {
        float inset = (tile.right - tile.left) * 0.16f;
        DrawSvg(doc, R(tile.left + inset, tile.top + inset, tile.right - inset, tile.bottom - inset));
    }
    else if (it.glyph)
    {
        Text(it.glyph, g_fGlyph.Get(), tile, C(P.text), DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    else
    {
        Text(std::wstring(1, it.name[0]), g_fLetter.Get(), tile, C(P.text), DWRITE_TEXT_ALIGNMENT_CENTER);
    }
}

static const wchar_t* ItemName(int i)
{
    if (kItems[i].group == kGroupTools)
    {
        if (wcscmp(kItems[i].name, L"Skills \u7BA1\u7406") == 0) return T(S_TOOLS_SKILLS);
        if (wcscmp(kItems[i].name, L"\u4F7F\u7528\u6559\u7A0B") == 0) return T(S_TOOLS_TUTORIAL);
        if (wcscmp(kItems[i].name, L"\u63D0\u793A\u8BCD\u6587\u4EF6\u5939") == 0) return T(S_TOOLS_PROMPTS);
        if (wcscmp(kItems[i].name, L"\u6570\u636E\u76EE\u5F55") == 0) return T(S_TOOLS_DATA);
        if (wcscmp(kItems[i].name, L"\u68C0\u67E5\u66F4\u65B0") == 0) return T(S_TOOLS_UPDATE);
        if (wcscmp(kItems[i].name, L"\u6EE1\u8840\u4E2D\u8F6C\u7AD9") == 0) return T(S_TOOLS_RELAY);
        if (wcscmp(kItems[i].name, L"GitHub \u4ED3\u5E93") == 0) return T(S_TOOLS_REPO);
        if (wcscmp(kItems[i].name, L"\u95EE\u9898\u53CD\u9988") == 0) return T(S_TOOLS_ISSUES);
    }
    return kItems[i].name;
}

static const wchar_t* ItemSub(int i)
{
    if (kItems[i].group == kGroupTools)
    {
        if (wcscmp(kItems[i].name, L"Skills \u7BA1\u7406") == 0) return T(S_TOOLS_SKILLS_SUB);
        if (wcscmp(kItems[i].name, L"\u4F7F\u7528\u6559\u7A0B") == 0) return T(S_TOOLS_TUTORIAL_SUB);
        if (wcscmp(kItems[i].name, L"\u63D0\u793A\u8BCD\u6587\u4EF6\u5939") == 0) return T(S_TOOLS_PROMPTS_SUB);
        if (wcscmp(kItems[i].name, L"\u6570\u636E\u76EE\u5F55") == 0) return T(S_TOOLS_DATA_SUB);
        if (wcscmp(kItems[i].name, L"\u68C0\u67E5\u66F4\u65B0") == 0) return T(S_TOOLS_UPDATE_SUB);
        if (wcscmp(kItems[i].name, L"\u6EE1\u8840\u4E2D\u8F6C\u7AD9") == 0) return T(S_TOOLS_RELAY_SUB);
        if (wcscmp(kItems[i].name, L"GitHub \u4ED3\u5E93") == 0) return T(S_TOOLS_REPO_SUB);
        if (wcscmp(kItems[i].name, L"\u95EE\u9898\u53CD\u9988") == 0) return T(S_TOOLS_ISSUES_SUB);
    }
    return kItems[i].sub;
}

static void DrawCard(const Slot& s)
{
    const Item& it = kItems[s.item];
    D2D1_RECT_F r = s.rect;
    bool hot = g_hover == kHoverCard + s.item;

    Fill(r, Ov(hot ? 0.09f : 0.05f), 10.f);
    Stroke(r, Ov(hot ? 0.16f : 0.10f), 10.f);

    // 编辑器页：竖版卡片（图标居中在上，名称 + 注入状态在下）
    if (it.group == kGroupEditor)
    {
        float cw = r.right - r.left;
        D2D1_RECT_F tile = R(r.left + (cw - 36.f) / 2, r.top + 10.f, r.left + (cw + 36.f) / 2, r.top + 46.f);
        DrawIcon(s.item, tile);

        if (it.key && g_state[s.item] == kInstalled)
        {
            D2D1_POINT_2F c = D2D1::Point2F(tile.right - 1, tile.bottom - 1);
            g_brush->SetColor(C(kGreen, 0.22f));
            g_rt->FillEllipse(D2D1::Ellipse(c, 8.5f, 8.5f), g_brush.Get());
            D2D1_ELLIPSE ring = D2D1::Ellipse(c, 6.f, 6.f);
            g_brush->SetColor(C(P.bg));
            g_rt->FillEllipse(ring, g_brush.Get());
            g_brush->SetColor(Ov(hot ? 0.10f : 0.05f));
            g_rt->FillEllipse(ring, g_brush.Get());
            g_brush->SetColor(C(kGreen));
            g_rt->FillEllipse(D2D1::Ellipse(ring.point, 4.5f, 4.5f), g_brush.Get());
        }

        Text(ItemName(s.item), g_fTitle.Get(), R(r.left + 4, r.top + 50, r.right - 4, r.top + 67), C(P.text), DWRITE_TEXT_ALIGNMENT_CENTER);

        if (it.key)
        {
            const wchar_t* st;
            if (!Supported(s.item))                      st = T(S_NOT_SUPPORTED);
            else if (!ToolDetected(s.item))              st = T(S_TOOL_NOT_FOUND);
            else if (g_state[s.item] == kInstalled)      st = T(S_INJECTED);
            else if (g_state[s.item] == kActiveNoState)  st = T(S_FILE_HAS_CONTENT);
            else                                         st = T(S_NOT_INJECTED);
            Text(st, g_fLabel.Get(), R(r.left + 4, r.top + 67, r.right - 4, r.top + 81), C(P.sub), DWRITE_TEXT_ALIGNMENT_CENTER);
        }
        return;
    }

    // 工具页：横版卡片（图标在左，名称 + 副标题在右）
    float cy = (r.top + r.bottom) / 2;
    D2D1_RECT_F tile = R(r.left + 11, cy - 17, r.left + 45, cy + 17);
    DrawIcon(s.item, tile);

    // 已注入：logo 右下角一个绿点，外圈柔光晕 + 底色隔离圈
    if (it.key && g_state[s.item] == kInstalled)
    {
        D2D1_POINT_2F c = D2D1::Point2F(tile.right - 1, tile.bottom - 1);
        g_brush->SetColor(C(kGreen, 0.22f));
        g_rt->FillEllipse(D2D1::Ellipse(c, 8.5f, 8.5f), g_brush.Get());
        D2D1_ELLIPSE ring = D2D1::Ellipse(c, 6.f, 6.f);
        g_brush->SetColor(C(P.bg));
        g_rt->FillEllipse(ring, g_brush.Get());
        g_brush->SetColor(Ov(hot ? 0.10f : 0.05f));
        g_rt->FillEllipse(ring, g_brush.Get());
        g_brush->SetColor(C(kGreen));
        g_rt->FillEllipse(D2D1::Ellipse(ring.point, 4.5f, 4.5f), g_brush.Get());
    }

    float tx = tile.right + 12, tr = r.right - 30;
    const wchar_t* sub = ItemSub(s.item);
    if (sub[0])
    {
        Text(ItemName(s.item), g_fTitle.Get(), R(tx, cy - 17, tr, cy), C(P.text));
        Text(sub, g_fLabel.Get(), R(tx, cy + 1, tr, cy + 16), C(P.sub));
    }
    else
    {
        Text(ItemName(s.item), g_fTitle.Get(), R(tx, cy - 9, tr, cy + 9), C(P.text));
    }
    Text(L"\uE76C", g_fIcon.Get(), R(tr, r.top, r.right - 8, r.bottom), C(hot ? P.text : P.sub), DWRITE_TEXT_ALIGNMENT_CENTER);   // ChevronRight
}

// ---------------------------------------------------------------------------
// 渲染入口
// ---------------------------------------------------------------------------
void Render(HWND hWnd)
{
    if (!CreateTarget(hWnd)) return;
    EnsureHook(hWnd);

    // 每次打开 Skills 管理弹窗时清空筛选，恢复到"全部"
    MaybeResetSkillFilter();

    g_rt->BeginDraw();
    g_rt->SetTransform(D2D1::Matrix3x2F::Identity());
    g_rt->Clear(D2D1::ColorF(0, 0, 0, 0));

    D2D1_SIZE_F sz = g_rt->GetSize();
    // Mica 模式下窗口背景由 DWM 绘制，这里保持透明；否则用主题色填不透明底
    if (!g_micaActive)
    {
        g_brush->SetColor(C(P.bg));
        g_rt->FillRectangle(R(0, 0, sz.width, sz.height), g_brush.Get());
    }
    g_brush->SetColor(Ov(0.14f));
    g_rt->DrawRectangle(R(0.5f, 0.5f, sz.width - 0.5f, sz.height - 0.5f), g_brush.Get(), 1.f);

    DrawHeader();

    std::vector<Slot> slots;
    Layout(slots);
    for (const Slot& s : slots)
        DrawCard(s);

    if (g_menuOpen)
        DrawMenu();
    if (g_dlg >= 0)
        DrawDialog();
    if (g_skillMgr)
        DrawSkillMgr();
    if (g_aboutOpen)
        DrawAbout();
    if (g_helpOpen)
        DrawHelp();
    if (g_disclaimer)
        DrawDisclaimer();

    // 底部：版本小胶囊
    {
        wchar_t buf[64];
        _snwprintf_s(buf, _TRUNCATE, L"v%s", T(S_VERSION));
        float w = MeasureText(buf, g_fLabel.Get()) + 22.f;
        D2D1_RECT_F chip = R(kW - kPad - w, kH - kStatusH - 5.f, kW - kPad + 2.f, kH - 5.f);
        Fill(chip, Ov(0.05f), 9.f);
        Text(buf, g_fLabel.Get(), chip, C(P.sub), DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    if (!g_toast.empty())
    {
        D2D1_RECT_F t = R(kPad + 40, kH - kPad - 40, kW - kPad - 40, kH - kPad - 8);
        Fill(t, C(P.panel, 0.98f), 16.f);
        Stroke(t, Ov(0.18f), 16.f);
        // 左侧强调色竖条，给出视觉锚点
        Fill(R(t.left + 12, t.top + 10, t.left + 15.5f, t.bottom - 10), C(P.accent), 2.f);
        Text(g_toast, g_fTab.Get(), t, C(P.text), DWRITE_TEXT_ALIGNMENT_CENTER);
    }

    if (g_rt->EndDraw() == D2DERR_RECREATE_TARGET)
    {
        g_brush.Reset();
        g_rt.Reset();
    }
}
