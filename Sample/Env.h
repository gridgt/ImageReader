#pragma once
#include "pch.h"
#include "OCR/OcrLite.h"
#include "OCR/OcrUtils.h"
class Window;
class Env
{
public:
	Env();
	~Env();
	static void init();
	static OcrLite* getOcr();
	static std::filesystem::path getDataPath();
	static ICoreWebView2Environment* getWebViewEnv();
	static winrt::Windows::System::DispatcherQueue& getDispatcherQueue();
public:
private:
	void checkRuntimeVersion();
	void initDataPath();
	void initWebViewEnv();
	static std::vector<int> getExeVer();
	static void initDispatcherQueueCtrl();
	HRESULT onEnvReady(HRESULT result, ICoreWebView2Environment* env);
private:
	std::filesystem::path dataPath; 
	ComPtr<ICoreWebView2Environment> webViewEnv;
	std::unique_ptr<Window> mainWindow;
	winrt::Windows::System::DispatcherQueue dq;
	OcrLite ocr;
};

