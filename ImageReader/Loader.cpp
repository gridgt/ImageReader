#include "pch.h"
#include "Loader.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"

static std::unique_ptr<Loader> ins;

Loader::Loader(WindowBase* win)
{
	auto d2d = D2D::get();
	loader = win->root->createChild("loader");
	loader->on("sizeChange", [this](void* e) {this->onSize(e);});
	loader->on("mouseDown", [this](void* e) {this->onDown(e);});
	loader->initSurface();
	text = d2d->createTextLayout(L"拖拽/点击加载图像", FLT_MAX, FLT_MAX);
}

Loader::~Loader()
{
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
	loader->setPosSize(0.f, y, win->w, h);
	text->SetFontSize(26.f * win->dpi, {0,INT_MAX});
	DWRITE_TEXT_METRICS metrics;
	text->GetMetrics(&metrics);
	textPos.x = (win->w - metrics.width) / 2;
	textPos.y = (h - metrics.height) / 2;
	paint();
}

void Loader::onDown(void* e)
{
	auto path = getFilePath();
	auto a = 1;
}

void Loader::paint()
{
	auto [s, d2d] = loader->paintStart();
	ComPtr<ID2D1SolidColorBrush> textBrush;
	ColorA color(0xAAAAAAff);
	d2d->CreateSolidColorBrush(color.getD2DColor(), textBrush.GetAddressOf());
	d2d->DrawTextLayout(textPos, text.Get(), textBrush.Get());
	s->EndDraw();
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