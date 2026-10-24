// ui.h : Direct2D 绘制、命中测试
//

#pragma once

#include "config.h"

#include <vector>

// ---------------------------------------------------------------------------
// 初始化与渲染
// ---------------------------------------------------------------------------
bool CreateFactories();
bool CreateTarget(HWND hWnd);
void ResetLogos();          // 切换主题/语言时清空 SVG logo 缓存
void Render(HWND hWnd);

// 由 app.cpp 的窗口消息处理调用（在 WM_SIZE / WM_DPICHANGED 里）
void ResizeTarget(UINT w, UINT h);
void SetTargetDpi(float dpi);

// ---------------------------------------------------------------------------
// 命中测试
// ---------------------------------------------------------------------------
int HitTest(float x, float y);
bool DlgEnabled(int b);

// Skills 管理弹窗当前筛选下可见的 skill 序号（app.cpp 滚轮钳制用）
std::vector<int> FilteredSkills();
