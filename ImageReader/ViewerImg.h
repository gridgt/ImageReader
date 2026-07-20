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
};

