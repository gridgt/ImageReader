#include "pch.h"
#include "Loader.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"
#include <iostream>
#include <fstream>
#include <wincodec.h>
#include "ViewerImg.h"
#include "Util.h"
#include "App.h"

static std::unique_ptr<Loader> ins;

Loader::Loader(WindowBase* win)
{
	auto d2d = D2D::get();
	node = win->root->createChild("loader");
	node->on("sizeChange", [this](void* e) {this->onSize(e);});
	node->on("mouseDown", [this](void* e) {this->onDown(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initSurface();
	text = d2d->createTextLayout(L"拖拽/点击加载图像", FLT_MAX, FLT_MAX);
}

Loader::~Loader()
{
	tinyocr_engine_destroy(engine);
}

void Loader::init(WindowBase* win)
{
	ins.reset(new Loader(win));

}
Loader* Loader::get()
{
	return ins.get();
}
void Loader::onSize(void* e)
{
	auto win = WindowMain::get();
	auto y{ 30.f * win->dpi }, h{ win->h - y - 22 * win->dpi };
	node->setPosSize(0.f, y, win->w, h);
	text->SetFontSize(26.f * win->dpi, {0,INT_MAX});
	DWRITE_TEXT_METRICS metrics;
	text->GetMetrics(&metrics);
	textPos.x = (win->w - metrics.width) / 2;
	textPos.y = (h - metrics.height) / 2;
	node->paint();
}

void Loader::onDown(void* e)
{
	imgPath = getFilePath();
	if (imgPath.empty()) {
		return;
	}
	node->hide();
	ViewerImg::init(WindowMain::get(), imgPath);
	//initEngine();
	read();
}

void Loader::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);
	ComPtr<ID2D1SolidColorBrush> textBrush;
	ColorA color(0xAAAAAAff);
	ctx->CreateSolidColorBrush(color.getD2DColor(), textBrush.GetAddressOf());
	ctx->DrawTextLayout(textPos, text.Get(), textBrush.Get());
}

std::wstring Loader::getFilePath()
{
	IFileOpenDialog* fileOpen;
	auto hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&fileOpen));
	if (FAILED(hr)) return L"";
	COMDLG_FILTERSPEC fileTypes[] = {
		{ L"png", L"*.png" },
		{ L"jpeg", L"*.jpg;*.jpeg" },
		{ L"all files", L"*.*" }
	};
	hr = fileOpen->SetFileTypes(ARRAYSIZE(fileTypes), fileTypes);
	if (FAILED(hr)) {
		fileOpen->Release();
		return L"";
	}
	hr = fileOpen->Show(NULL);
	if (FAILED(hr)) {
		fileOpen->Release();
		return L"";
	}
	IShellItem* pItem;
	hr = fileOpen->GetResult(&pItem);
	if (FAILED(hr)) {
		fileOpen->Release();
		return L"";
	}
	PWSTR pszFilePath;
	hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
	if (FAILED(hr)) {
		pItem->Release();
		fileOpen->Release();
		return L"";
	}
	std::wstring result{ pszFilePath };
	CoTaskMemFree(pszFilePath);
	pItem->Release();
	fileOpen->Release();
	return result;
}

std::vector<unsigned char> Loader::getFileData(const std::wstring& filePath)
{
	std::vector<unsigned char> imgData;
	auto ifs = std::ifstream(filePath, std::ios::binary);
	if (!ifs.is_open()) {
		return imgData;
	}
	ifs.seekg(0, std::ios::end);
	size_t fileSize = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	imgData.resize(fileSize);
	ifs.read(reinterpret_cast<char*>(imgData.data()), fileSize);
	ifs.close();
	return imgData;
}

winrt::Windows::Foundation::IAsyncAction Loader::read()
{
	co_await winrt::resume_background();
	auto engine = tinyocr_engine_create();
	tinyocr_ocr_model_paths_t paths = {
	"./models/PP-OCRv6_tiny_det.param",
	"./models/PP-OCRv6_tiny_det.bin",
	"./models/PP-OCRv6_tiny_rec.param",
	"./models/PP-OCRv6_tiny_rec.bin",
	"./models/PP-OCRv6_vocab_tiny.txt",
	"./models/PP-LCNet_x1_0_textline_ori.param",
	"./models/PP-LCNet_x1_0_textline_ori.bin"
	};
	if (tinyocr_engine_load_model(engine, &paths, nullptr) != 0) {
		tinyocr_engine_destroy(engine);
		co_return;
	}
	auto data = getFileData(imgPath);
	if (data.empty()) {
		co_return;
	}
	tinyocr_text_box_t* boxes = nullptr;
	int box_count = 0;
	tinyocr_text_line_t* lines = nullptr;
	int line_count = 0;
	if (tinyocr_engine_recognize_buffer(engine, data.data(), (int)data.size(), &boxes, &box_count, &lines, &line_count) != 0)
	{
		tinyocr_engine_destroy(engine);
		co_return;
	}	
	std::vector<ComPtr<ID2D1PathGeometry>> pathes;
	auto d2d = D2D::get();
	for (int i = 0; i < box_count; i++) {
		//std::cout << "Box " << i << ": points("
		//	<< boxes[i].points[0] << "," << boxes[i].points[1] << "; "
		//	<< boxes[i].points[2] << "," << boxes[i].points[3] << "; "
		//	<< boxes[i].points[4] << "," << boxes[i].points[5] << "; "
		//	<< boxes[i].points[6] << "," << boxes[i].points[7] << "), "
		//	<< "isVertical: " << boxes[i].is_vertical << ", "
		//	<< "score: " << boxes[i].score << std::endl;
		auto path = d2d->createPath(boxes[i].points);
		pathes.push_back(std::move(path));
		auto wstr = Util::convertToWStr(lines[i].text);
		log(wstr);
		//writeConsoleW(L"Recognized Text: ");
		//writeConsoleW(wstr);
		//writeConsoleW(L"\n");
		for (size_t j = 0; j < lines[i].anchor_count; j++)
		{
			auto anchors = lines[i].anchors[j];
			log(L"pos:{}",anchors);
			auto a = 1;
			//writeConsoleW(std::to_wstring(lines[i].anchors[j]));
			//writeConsoleW(L" ");
		}
		//writeConsoleW(L"\n");
	}
	tinyocr_free_text_boxes(boxes, box_count);
	tinyocr_free_text_lines(lines, line_count);
	tinyocr_engine_destroy(engine);
	co_await winrt::resume_foreground(App::get()->dq);
	ViewerImg::get()->setPathes(pathes);
}

void Loader::initEngine()
{
	if (!engine) return;
	engine = tinyocr_engine_create();
	tinyocr_ocr_model_paths_t paths = {
	"./models/PP-OCRv6_tiny_det.param",
	"./models/PP-OCRv6_tiny_det.bin",
	"./models/PP-OCRv6_tiny_rec.param",
	"./models/PP-OCRv6_tiny_rec.bin",
	"./models/PP-OCRv6_vocab_tiny.txt",
	"./models/PP-LCNet_x1_0_textline_ori.param",
	"./models/PP-LCNet_x1_0_textline_ori.bin"
	};
	if (tinyocr_engine_load_model(engine, &paths, nullptr) != 0) {
		tinyocr_engine_destroy(engine);
		return;
	}
}
