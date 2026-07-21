#pragma once
#include "pch.h"
#include "Node.h"
#include "tinyocr.h"
class WindowBase;
class Loader
{
public:
	~Loader();
	static void init(WindowBase* win);
	static Loader* get();
	/// <summary>
	/// 弹选择文件对话框，加载图像；与初始点击“拖拽/点击加载图像”提示的行为一致。
	/// </summary>
	void pickAndLoad();
private:
	Loader(WindowBase* win);
	void onSize(void* e);
	void onDown(void* e);
	void onPaint(void* e);
	std::wstring getFilePath();
	std::vector<unsigned char> getFileData(const std::wstring& filePath);
	winrt::Windows::Foundation::IAsyncAction read(std::wstring imgPath);
private:
	ComPtr<IDWriteTextLayout> text;
	D2D1_POINT_2F textPos;
	bool isHover{false};
	Node* node;
	tinyocr_engine_t engine;
};

