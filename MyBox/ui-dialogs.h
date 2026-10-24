// ui-dialogs.h : 内部头文件，弹窗 Rect/Draw 导出给 ui-main.cpp
//
// 仅供 ui-*.cpp 内部使用；ui.h 对外导出的 API 保持不变。

#pragma once

#include "ui-draw.h"

// ---------------------------------------------------------------------------
// 弹窗矩形（HitTest 需要）
// ---------------------------------------------------------------------------
D2D1_RECT_F MenuRect();
D2D1_RECT_F MenuItemRect(int i);

// 主窗口右上角按钮（定义在 ui-main.cpp，弹窗元素要与它对齐）
D2D1_RECT_F ButtonRect(int id);

D2D1_RECT_F DlgRect();
D2D1_RECT_F DlgButtonRect(int b);
D2D1_RECT_F DlgPromptRect(int vis);

D2D1_RECT_F DiscAcceptRect();

D2D1_RECT_F AboutRect();
D2D1_RECT_F AboutCloseRect();
D2D1_RECT_F AboutGithubRect();
D2D1_RECT_F AboutTutorialRect();
D2D1_RECT_F AboutUpdateRect();
D2D1_RECT_F AboutUpdateBtnRect();

D2D1_RECT_F HelpRect();
D2D1_RECT_F HelpCloseRect();

D2D1_RECT_F SkillDlgRect();
D2D1_RECT_F SkillCloseRect();
D2D1_RECT_F SkillFolderRect();
void        SkillCatPills(D2D1_RECT_F out[kCatCount]);
D2D1_RECT_F SkillSearchRect();
D2D1_RECT_F SkillItemRect(int vis);
D2D1_RECT_F SkillRemoveRect(int vis);
std::vector<int> FilteredSkills();

// ---------------------------------------------------------------------------
// 弹窗绘制（Render 需要）
// ---------------------------------------------------------------------------
void DrawMenu();
void DrawDialog();
void DrawSkillMgr();
void DrawDisclaimer();
void DrawAbout();
void DrawHelp();

// Skills 管理弹窗首次打开时，重置筛选条件
void MaybeResetSkillFilter();

// 挂 WndProc 钩子接收 WM_CHAR / WM_KEYDOWN 更新搜索框
void EnsureHook(HWND hWnd);
