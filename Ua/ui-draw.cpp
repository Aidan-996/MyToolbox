// ui-draw.cpp : Direct2D 全局资源、字体、绘图原语、SVG logo
//

#include "ui-draw.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "shlwapi.lib")

// ---------------------------------------------------------------------------
// D2D 资源（在头文件中 extern 声明，这里是真实定义）
// ---------------------------------------------------------------------------
ComPtr<ID2D1Factory> g_d2d;
ComPtr<IDWriteFactory> g_dw;
ComPtr<ID2D1HwndRenderTarget> g_rt;
ComPtr<ID2D1SolidColorBrush> g_brush;
ComPtr<IDWriteTextFormat> g_fTitle, g_fLabel, g_fTab, g_fTabSel, g_fIcon, g_fGlyph, g_fLetter, g_fWrap;

ComPtr<ID2D1SvgDocument> g_logos[kItemCount];
bool g_logoTried[kItemCount];

void ResetLogos()
{
    for (int i = 0; i < kItemCount; ++i)
    {
        g_logos[i].Reset();
        g_logoTried[i] = false;
    }
}

// ---------------------------------------------------------------------------
// 字体与渲染目标
// ---------------------------------------------------------------------------
static ComPtr<IDWriteTextFormat> MakeFormat(float size, bool strong, const wchar_t* family = nullptr)
{
    ComPtr<IDWriteTextFormat> f;
    if (FAILED(g_dw->CreateTextFormat(family ? family : kFontFamily, nullptr,
        strong ? DWRITE_FONT_WEIGHT_BOLD : DWRITE_FONT_WEIGHT_NORMAL,     // Tahoma 只有常规和粗体
        DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size, L"zh-cn", &f)))
        return nullptr;
    f->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    DWRITE_TRIMMING trim{ DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
    ComPtr<IDWriteInlineObject> sign;
    if (SUCCEEDED(g_dw->CreateEllipsisTrimmingSign(f.Get(), &sign)))
        f->SetTrimming(&trim, sign.Get());
    return f;
}

bool CreateFactories()
{
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, g_d2d.GetAddressOf())))
        return false;
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(g_dw.GetAddressOf()))))
        return false;

    g_fTitle = MakeFormat(12.5f, true);
    g_fLabel = MakeFormat(11.5f, false);
    g_fTab = MakeFormat(12.5f, false);
    g_fTabSel = MakeFormat(12.5f, true);
    g_fIcon = MakeFormat(12.f, false, kIconFamily);
    g_fGlyph = MakeFormat(15.f, false, kIconFamily);
    g_fLetter = MakeFormat(14.f, true);
    g_fWrap = MakeFormat(11.5f, false);
    if (g_fWrap) g_fWrap->SetWordWrapping(DWRITE_WORD_WRAPPING_WRAP);
    for (auto* f : { g_fTab.Get(), g_fTabSel.Get(), g_fIcon.Get(), g_fGlyph.Get(), g_fLetter.Get() })
        if (f) f->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    return g_fTitle && g_fLabel && g_fTab && g_fTabSel && g_fIcon && g_fGlyph && g_fLetter && g_fWrap;
}

bool CreateTarget(HWND hWnd)
{
    if (g_rt) return true;
    RECT rc;
    GetClientRect(hWnd, &rc);
    float dpi = 96.f * g_scale;
    D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED), dpi, dpi);
    if (FAILED(g_d2d->CreateHwndRenderTarget(props,
        D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)), &g_rt)))
        return false;
    g_rt->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);   // 透明背景下不能用 ClearType
    if (FAILED(g_rt->CreateSolidColorBrush(C(P.text), &g_brush)))
    {
        g_rt.Reset();
        return false;
    }
    ResetLogos();
    return true;
}

// 让 app.cpp 的 WM_SIZE / WM_DPICHANGED 能操作渲染目标
// ---------------------------------------------------------------------------
// 这些函数暴露给 app.cpp，用来避免在 app.cpp 里直接碰 D2D 对象
// ---------------------------------------------------------------------------
void ResizeTarget(UINT w, UINT h)
{
    if (g_rt) g_rt->Resize(D2D1::SizeU(w, h));
}

void SetTargetDpi(float dpi)
{
    if (g_rt) g_rt->SetDpi(dpi, dpi);
}

// ---------------------------------------------------------------------------
// 绘制基础
// ---------------------------------------------------------------------------
void Fill(D2D1_RECT_F r, D2D1_COLOR_F c, float radius)
{
    g_brush->SetColor(c);
    g_rt->FillRoundedRectangle(D2D1::RoundedRect(r, radius, radius), g_brush.Get());
}

void Stroke(D2D1_RECT_F r, D2D1_COLOR_F c, float radius)
{
    g_brush->SetColor(c);
    D2D1_RECT_F b = R(r.left + 0.5f, r.top + 0.5f, r.right - 0.5f, r.bottom - 0.5f);
    g_rt->DrawRoundedRectangle(D2D1::RoundedRect(b, radius, radius), g_brush.Get(), 1.f);
}

static bool IsCjk(wchar_t ch)
{
    return (ch >= 0x2E80 && ch <= 0x9FFF) || (ch >= 0xF900 && ch <= 0xFAFF) || (ch >= 0xFF00 && ch <= 0xFFEF);
}

void Text(const std::wstring& s, IDWriteTextFormat* f, D2D1_RECT_F r, D2D1_COLOR_F c,
    DWRITE_TEXT_ALIGNMENT align)
{
    UINT32 len = (UINT32)s.size();
    ComPtr<IDWriteTextLayout> layout;
    if (len == 0 || FAILED(g_dw->CreateTextLayout(s.c_str(), len, f, r.right - r.left, r.bottom - r.top, &layout)))
        return;
    layout->SetTextAlignment(align);

    // 中文字符显式交给雅黑，不让系统按 Tahoma 的回退规则挑字体（回退结果在不同电脑上可能是宋体）
    if (f != g_fIcon.Get() && f != g_fGlyph.Get())
    {
        for (UINT32 i = 0; i < len;)
        {
            if (!IsCjk(s[i])) { ++i; continue; }
            UINT32 start = i;
            while (i < len && IsCjk(s[i])) ++i;
            layout->SetFontFamilyName(kCjkFamily, { start, i - start });
        }
    }

    g_brush->SetColor(c);
    g_rt->DrawTextLayout(D2D1::Point2F(r.left, r.top), layout.Get(), g_brush.Get(), D2D1_DRAW_TEXT_OPTIONS_CLIP);
}

// 从资源加载 SVG logo。需要 Win10 1703+ 的 ID2D1DeviceContext5，拿不到时返回 nullptr，由调用方画首字母代替
ID2D1SvgDocument* Logo(int i)
{
    if (g_logoTried[i]) return g_logos[i].Get();
    g_logoTried[i] = true;

    ComPtr<ID2D1DeviceContext5> dc;
    HRSRC res = FindResourceW(hInst, MAKEINTRESOURCEW(kItems[i].logo), RT_RCDATA);
    HGLOBAL h = res ? LoadResource(hInst, res) : nullptr;
    if (!h || FAILED(g_rt.As(&dc)))
        return nullptr;

    std::string svg(static_cast<const char*>(LockResource(h)), SizeofResource(hInst, res));
    char color[8];
    sprintf_s(color, "#%06X", P.text & 0xFFFFFFu);   // 单色 logo 写的是 currentColor，换成当前主题的文字色
    for (size_t p; (p = svg.find("currentColor")) != std::string::npos;)
        svg.replace(p, 12, color);

    ComPtr<IStream> stream;
    stream.Attach(SHCreateMemStream(reinterpret_cast<const BYTE*>(svg.data()), (UINT)svg.size()));
    if (stream)
        dc->CreateSvgDocument(stream.Get(), D2D1::SizeF(24.f, 24.f), &g_logos[i]);
    return g_logos[i].Get();
}

void DrawSvg(ID2D1SvgDocument* doc, D2D1_RECT_F r)
{
    ComPtr<ID2D1DeviceContext5> dc;
    if (FAILED(g_rt.As(&dc))) return;
    D2D1_MATRIX_3X2_F old;
    g_rt->GetTransform(&old);
    float s = (r.right - r.left) / 24.f;
    g_rt->SetTransform(D2D1::Matrix3x2F::Scale(s, s) * D2D1::Matrix3x2F::Translation(r.left, r.top) * old);
    dc->DrawSvgDocument(doc);
    g_rt->SetTransform(old);
}

// 按资源 ID 加载 SVG（独立于 kItems 的 Logo()），用于 Ua 自身的 logo
static ComPtr<ID2D1SvgDocument> g_uaLogo;
static bool g_uaLogoTried = false;

ID2D1SvgDocument* LoadSvgById(int resId)
{
    if (resId != IDR_LOGO_UA) return nullptr;    // 现在只支持 Ua logo
    if (g_uaLogoTried) return g_uaLogo.Get();
    g_uaLogoTried = true;

    ComPtr<ID2D1DeviceContext5> dc;
    HRSRC res = FindResourceW(hInst, MAKEINTRESOURCEW(resId), RT_RCDATA);
    HGLOBAL h = res ? LoadResource(hInst, res) : nullptr;
    if (!h || FAILED(g_rt.As(&dc)))
        return nullptr;

    std::string svg(static_cast<const char*>(LockResource(h)), SizeofResource(hInst, res));
    ComPtr<IStream> stream;
    stream.Attach(SHCreateMemStream(reinterpret_cast<const BYTE*>(svg.data()), (UINT)svg.size()));
    if (stream)
        dc->CreateSvgDocument(stream.Get(), D2D1::SizeF(18.f, 18.f), &g_uaLogo);
    return g_uaLogo.Get();
}

void DrawSvgSized(ID2D1SvgDocument* doc, D2D1_RECT_F r, float viewBoxSize)
{
    ComPtr<ID2D1DeviceContext5> dc;
    if (FAILED(g_rt.As(&dc))) return;
    D2D1_MATRIX_3X2_F old;
    g_rt->GetTransform(&old);
    float s = (r.right - r.left) / viewBoxSize;
    g_rt->SetTransform(D2D1::Matrix3x2F::Scale(s, s) * D2D1::Matrix3x2F::Translation(r.left, r.top) * old);
    dc->DrawSvgDocument(doc);
    g_rt->SetTransform(old);
}

// ---------------------------------------------------------------------------
// 测量字符串宽度（支持 CJK 字体回退，用于分类标签动态分配宽度）
// ---------------------------------------------------------------------------
float MeasureText(const wchar_t* s, IDWriteTextFormat* f)
{
    if (!s || !*s) return 0.f;
    UINT32 len = (UINT32)wcslen(s);
    ComPtr<IDWriteTextLayout> layout;
    if (FAILED(g_dw->CreateTextLayout(s, len, f, 1000.f, 100.f, &layout)))
        return 0.f;
    for (UINT32 i = 0; i < len;)
    {
        if (!IsCjk(s[i])) { ++i; continue; }
        UINT32 start = i;
        while (i < len && IsCjk(s[i])) ++i;
        layout->SetFontFamilyName(kCjkFamily, { start, i - start });
    }
    DWRITE_TEXT_METRICS m{};
    if (FAILED(layout->GetMetrics(&m))) return 0.f;
    return m.widthIncludingTrailingWhitespace;
}
