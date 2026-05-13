#pragma once
#include "pch.h"
#include "OCR/OcrLite.h"
#include "OCR/OcrUtils.h"
class Env
{
public:
	Env();
	~Env();
	static void init();
	static winrt::Windows::System::DispatcherQueue& getDQ();
	static OcrLite* getOcr();
private:
	static void initDQCtrl();
	void initOcr();
private:
	winrt::Windows::System::DispatcherQueue dq;
	OcrLite ocr;
};

