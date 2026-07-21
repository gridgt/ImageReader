#pragma once
#include "pch.h"
#include "Node.h"
class WindowBase;
class ViewerImg
{
public:
	~ViewerImg();
	static void init(WindowBase* win,const std::wstring& path);
	static ViewerImg* get();
	void setPathes(const std::map<int, std::vector<float>>& boxPoints,const std::map<int, std::vector<float>>& charPoints);
	/// <summary>
	/// 外部（如 ViewerText）同步选区。传入负值等同于清空选区。
	/// startBox/endBox 与 charLines 的 key 一致；startChar/endChar 是各自 box 内的“分隔线索引”，
	/// 等价于 caret 字符位置（字符 j 被选中当且仅当 min(sChar,eChar) <= j < max(sChar,eChar)）。
	/// </summary>
	void setSelection(int startBox, int startChar, int endBox, int endChar);
	/// <summary>
	/// ViewerImg 的右边界（== ViewerText 的左边界）。用于让 ViewerText 与本 Viewer 的宽度联动。
	/// </summary>
	float getSplitX() const { return splitX; }
private:
	ViewerImg(WindowBase* win, const std::wstring& path);
	void onSize(void* e);
	void onDown(void* e);
	void onUp(void* e);
	void onMove(void* e);
	void onPaint(void* e);
	void onCursor(void* e);
	/// <summary>
	/// 将窗口坐标转换为图像坐标下最近的 box + 分隔线索引
	/// </summary>
	bool hitTest(float wx, float wy, int& boxIdx, int& charIdx);
	void paintAssist(ID2D1DeviceContext* ctx);
	/// <summary>
	/// 把当前选区同步到 ViewerText（选区无效时清空 ViewerText 选区）
	/// </summary>
	void syncSelectionToText();
private:
	D2D1_POINT_2F pos;
	float scale{1.0f};
	bool isHover{ false }, isMouseDown{false};
	Node* node;
	ComPtr<ID2D1Bitmap> bitmap;
	std::vector<ComPtr<ID2D1PathGeometry>> pathes;
	std::map<int, std::vector<std::pair<D2D1_POINT_2F, D2D1_POINT_2F>>> charLines;
	// 文本选区，Char 索引是 charLines[box] 中的“分隔线”索引，
	// 字符 j 被选中当且仅当 min(startChar,endChar) <= j < max(startChar,endChar)
	int selStartBox{ -1 }, selStartChar{ -1 };
	int selEndBox{ -1 }, selEndChar{ -1 };
	// 上一次 mouseDown 的时间戳（GetTickCount64），用于双击判定
	ULONGLONG lastDownTick{ 0 };
	// —— 分隔栏（拖拽右边缘改变 ViewerImg / ViewerText 宽度）——
	// ViewerImg 的右边界（客户区绝对 x）。初始 = 窗口宽 - 360，后续可被拖拽修改。
	float splitX{ -1.f };
	// 上一次“窗口右侧到 splitX 的距离”，用于窗口 resize 时保持右侧文本区宽度不变
	float rightGap{ 360.f };
	bool isHoverSplit{ false };
	bool isDraggingSplit{ false };
	float dragStartMouseX{ 0.f };
	float dragStartSplitX{ 0.f };
	// 命中带宽度（分割线左右各 5 * dpi）
	float splitHitHalfWidth() const;
	bool isPosOnSplit(float wx, float wy) const;
	void applySplitX(float newX);
	/// <summary>
	/// 右键弹出的上下文菜单（客户区坐标）
	/// </summary>
	void showContextMenu(float clientX, float clientY);
};

