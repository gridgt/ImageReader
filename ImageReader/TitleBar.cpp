#include "pch.h"
#include "TitleBar.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"

static std::unique_ptr<TitleBar> ins;

TitleBar::TitleBar(WindowBase* win)
{
	win->on("maximize", [this](void* ptr) { this->onMaximize(); });
	win->on("restore", [this](void* ptr) { this->onRestore(); });
	win->on("cursor", [this](void* e) {this->onCursor(e);});
	auto d2d = D2D::get();
	node = win->root->createChild("titleBar");
	node->on("sizeChange", [this](void* e) {this->onBarSize(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initSurface();
	title = d2d->createTextLayout(win->title, FLT_MAX, FLT_MAX);
	std::wstring code;
	for (size_t i = 0; i < 3; i++)
	{
		auto id = std::format("btn{}", i);
		auto btn = node->createChild(id);
		btn->on("sizeChange", [this](void* e) {this->onSize(e);});
		btn->on("mouseEnter", [this](void* e) {this->onEnter(e);});
		btn->on("mouseLeave", [this](void* e) {this->onLeave(e);});
		btn->on("mouseDown", [this](void* e) {this->onDown(e);});
		btn->on("paint", [this](void* e) {this->onBtnPaint(e);});
		btn->initSurface();
		btns.push_back(btn);
		if (i == 0) {
			code = L"\ue6e8";
		}
		else if (i == 1) {
			code = L"\ue6e5";
		}
		else if (i == 2) {
			code = L"\ue6e7";
		}
		auto icon = d2d->createTextLayout(code, FLT_MAX, FLT_MAX);
		icon->SetFontFamilyName(L"icon", { 0,INT_MAX });
		icons.push_back(std::move(icon));
		poss.push_back({ 0.f,0.f });
	}
}

void TitleBar::onBarSize(void* e)
{
	auto& dpi = node->win->dpi;
	auto h{ 30.f * dpi };
	node->setPosSize(0.f, 0.f, (float)(node->win->w), h);
	title->SetFontSize(13.f * dpi, { 0,INT_MAX });
	DWRITE_TEXT_METRICS metrics;
	title->GetMetrics(&metrics);
	titlePos.x = 12 * dpi;
	titlePos.y = (h - metrics.height) / 2;
	node->paint();
}

TitleBar::~TitleBar()
{
}

void TitleBar::init(WindowBase* win)
{
	ins.reset(new TitleBar(win));

}
TitleBar* TitleBar::get()
{
	return ins.get();
}
void TitleBar::onSize(void* e)
{
	auto btn = static_cast<Node*>(e);
	auto it = std::find(btns.begin(), btns.end(), btn);
	size_t index = std::distance(btns.begin(), it);
	auto win = btn->win;
	auto btnW{ 34.f * win->dpi }, btnH{ 30.f * win->dpi };
	btn->setPosSize(win->w - btnW * (3 - index), 0.f, btnW, btnH);
	icons[index]->SetFontSize(12.f * win->dpi, {0,INT_MAX});
	DWRITE_TEXT_METRICS metrics;
	icons[index]->GetMetrics(&metrics);
	poss[index].x = (btnW - metrics.width) / 2;
	poss[index].y = (btnH - metrics.height) / 2;
	btn->paint();
}

void TitleBar::onEnter(void* e)
{
	auto tuplePtr = static_cast<std::tuple<float, float, Node*>*>(e);
	auto btn = std::get<2>(*tuplePtr);
	auto it = std::find(btns.begin(), btns.end(), btn);
	hoverIndex = std::distance(btns.begin(), it);
	btn->paint();
}

void TitleBar::onLeave(void* e)
{
	auto btn = static_cast<Node*>(e);
	hoverIndex = -1;
	btn->paint();
}

void TitleBar::onDown(void* e)
{
	auto tuplePtr = static_cast<std::tuple<float, float,bool, Node*>*>(e);
	auto btn = std::get<3>(*tuplePtr);
	auto it = std::find(btns.begin(), btns.end(), btn);
	size_t index = std::distance(btns.begin(), it);
	auto win = btn->win;
	if (index == 2) {
		ExitProcess(0);
	}
	else if (index == 1) {
		if (IsZoomed(btn->win->hwnd)) {
			ShowWindow(btn->win->hwnd, SW_RESTORE);
		}
		else {
			ShowWindow(btn->win->hwnd, SW_SHOWMAXIMIZED);
		}
	}
	else if (index == 0) {
		ShowWindow(btn->win->hwnd, SW_MINIMIZE);
	}
}

void TitleBar::onCursor(void* e)
{
	auto nodeHover = WindowMain::get()->nodeHover;
	if (nodeHover && nodeHover->id.starts_with("btn")) {
		SetCursor(LoadCursor(nullptr, IDC_HAND));
		auto flag = static_cast<bool*>(e);
		*flag = true;
	}
}

void TitleBar::onBtnPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto [btn, ctx] = *tuplePtr;
	auto it = std::find(btns.begin(), btns.end(), btn);
	size_t index = std::distance(btns.begin(), it);
	if (index == hoverIndex) {
		auto size = btn->surface.Size();
		D2D1_RECT_F rr = { 0,0,size.Width,size.Height };
		ComPtr<ID2D1SolidColorBrush> bgBrush;
		ColorA color(index == 2 ? 0xE81123FF : 0xE0E0E0FF);
		ctx->CreateSolidColorBrush(color.getD2DColor(), bgBrush.GetAddressOf());
		ctx->FillRectangle(rr, bgBrush.Get());
	}
	uint32_t colorVal{ 0x888888ff };
	if (index == hoverIndex) {
		if (index == 2) {
			colorVal = 0xFFFFFFff;
		}
		else {
			colorVal = 0x333333ff;
		}
	}
	ComPtr<ID2D1SolidColorBrush> textBrush;
	ColorA color(colorVal);
	ctx->CreateSolidColorBrush(color.getD2DColor(), textBrush.GetAddressOf());
	ctx->DrawTextLayout(poss[index], icons[index].Get(), textBrush.Get());
}

void TitleBar::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);
	auto size = node->surface.Size();
	D2D1_RECT_F r = { 0,0,size.Width,size.Height };
	ComPtr<ID2D1SolidColorBrush> bgBrush;
	ctx->CreateSolidColorBrush(ColorA(0xEEEEEEFF).getD2DColor(), bgBrush.GetAddressOf());
	ctx->FillRectangle(r, bgBrush.Get());
	ComPtr<ID2D1SolidColorBrush> textBrush;
	ctx->CreateSolidColorBrush(ColorA(0x666666FF).getD2DColor(), textBrush.GetAddressOf());
	ctx->DrawTextLayout(titlePos, title.Get(), textBrush.Get());
}

void TitleBar::onMaximize()
{
	auto d2d = D2D::get();
	auto icon = d2d->createTextLayout(L"\ue6e9", FLT_MAX, FLT_MAX);
	icon->SetFontFamilyName(L"icon", { 0,INT_MAX });
	icons[1] = icon;
}

void TitleBar::onRestore()
{
	auto d2d = D2D::get();
	auto icon = d2d->createTextLayout(L"\ue6e5", FLT_MAX, FLT_MAX);
	icon->SetFontFamilyName(L"icon", { 0,INT_MAX });
	icons[1] = icon;
}
