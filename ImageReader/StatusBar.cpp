#include "pch.h"
#include "StatusBar.h"
#include "ImgViewer.h"
#include "TextBox.h"
#include "WindowMain.h"

StatusBar::StatusBar(Ling::WinBase* win) :Ling::Node(win)
{
    setWidthPercent(100.f);
    setHeight(28.f);
    setBg(0xEEEEEEFF);
    setFlexDirection(Ling::FlexDirection::Row);
    setAlignItems(Ling::Align::Center);

    label = makeChild<Ling::Label>();
    label->setText(L"Ready");
    label->setFlexGrow(1.f);
    label->setMarginLeft(8.f);

    btn = makeChild<Ling::Button>();
    btn->setText(L"加载图像");
    btn->setWidth(68.f);
    btn->setHeight(22.f);
    btn->setBg(0x85a5ffFF);
    btn->setHoverBg(0x597ef7FF);
    btn->setColor(0xFFFFFFFF);
    btn->setHoverColor(0xFFFFFFFF);
    btn->setBorderRadius(2.f);
    btn->setMarginRight(8.f);
    btn->onClick.add([this](Ling::Button* btn) {this->onClick();});
}

StatusBar::~StatusBar()
{
}

void StatusBar::onClick()
{
	auto imgPath = getFilePath();
	if (imgPath.empty()) {
		return;
	}
	auto cur = dynamic_cast<WindowMain*>(win);
	cur->imgViewer->loadImg(imgPath);
}

std::wstring StatusBar::getFilePath()
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
	hr = fileOpen->Show(win->hwnd);
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