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
private:
	Loader(WindowBase* win);
	void onSize(void* e);
	void onDown(void* e);
	void onPaint(void* e);
	std::wstring getFilePath();
	std::vector<unsigned char> getFileData(const std::wstring& filePath);
	winrt::Windows::Foundation::IAsyncAction read();
	void initEngine();
private:
	ComPtr<IDWriteTextLayout> text;
	D2D1_POINT_2F textPos;
	bool isHover{false};
	Node* node;
	tinyocr_engine_t engine;
	std::wstring imgPath;
};

