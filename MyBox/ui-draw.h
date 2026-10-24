// ui-draw.h : 内部共享头文件
//
// 聚合 ui-draw.cpp / ui-dialogs.cpp / ui-main.cpp 需要的 D2D 全局与绘图原语。
// 不对外暴露，仅供 ui-*.cpp 内部使用。

#pragma once

#include "state.h"
#include "inject.h"
#include "ui.h"

#include <shlwapi.h>
#include <d2d1_3.h>
#include <dwrite.h>
#include <wrl/client.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using Microsoft::WRL::ComPtr;

// ---------------------------------------------------------------------------
// D2D 资源（定义在 ui-draw.cpp）
// ---------------------------------------------------------------------------
extern ComPtr<ID2D1Factory> g_d2d;
extern ComPtr<IDWriteFactory> g_dw;
extern ComPtr<ID2D1HwndRenderTarget> g_rt;
extern ComPtr<ID2D1SolidColorBrush> g_brush;
extern ComPtr<IDWriteTextFormat> g_fTitle, g_fLabel, g_fTab, g_fTabSel, g_fIcon, g_fGlyph, g_fLetter, g_fWrap;

// logo 文档跟渲染目标和主题绑定（单色 logo 要换颜色），两者变化时清空重建
extern ComPtr<ID2D1SvgDocument> g_logos[kItemCount];
extern bool g_logoTried[kItemCount];

// ---------------------------------------------------------------------------
// 颜色 / 矩形帮助（内联以避免跨 TU 调用开销）
// ---------------------------------------------------------------------------
inline D2D1_COLOR_F C(UINT32 rgb, float a = 1.f) { return D2D1::ColorF(rgb, a); }
inline D2D1_RECT_F R(float l, float t, float r, float b) { return D2D1::RectF(l, t, r, b); }

// 界面叠加色：深色主题用白色叠加，浅色主题用黑色叠加（同样的透明度在浅色背景上显得更重，所以打个折）
inline D2D1_COLOR_F Ov(float a) { return P.dark ? C(0xFFFFFF, a) : C(0x000000, a * 0.6f); }

// 点是否在矩形内（命中测试用）
inline bool InRect(D2D1_RECT_F r, float x, float y)
{
    return x >= r.left && x < r.right && y >= r.top && y < r.bottom;
}

// ---------------------------------------------------------------------------
// 绘图原语
// ---------------------------------------------------------------------------
void Fill(D2D1_RECT_F r, D2D1_COLOR_F c, float radius);
void Stroke(D2D1_RECT_F r, D2D1_COLOR_F c, float radius);
void Text(const std::wstring& s, IDWriteTextFormat* f, D2D1_RECT_F r, D2D1_COLOR_F c,
    DWRITE_TEXT_ALIGNMENT align = DWRITE_TEXT_ALIGNMENT_LEADING);
ID2D1SvgDocument* Logo(int i);
ID2D1SvgDocument* LoadSvgById(int resId);
void DrawSvg(ID2D1SvgDocument* doc, D2D1_RECT_F r);
void DrawSvgSized(ID2D1SvgDocument* doc, D2D1_RECT_F r, float viewBoxSize);
float MeasureText(const wchar_t* s, IDWriteTextFormat* f);
