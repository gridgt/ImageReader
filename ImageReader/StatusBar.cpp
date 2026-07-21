#include "pch.h"
#include "StatusBar.h"
#include "WindowBase.h"
#include "WindowMain.h"
#include "D2D.h"
#include "Node.h"

static std::unique_ptr<StatusBar> ins;

StatusBar::StatusBar(WindowBase* win)
{
	node = win->root->createChild("statusBar");
	node->on("sizeChange", [this](void* e) {this->onSize(e);});
	node->on("paint", [this](void* e) {this->onPaint(e);});
	node->initSurface();
	
}

StatusBar::~StatusBar()
{
}

void StatusBar::init(WindowBase* win)
{
	if (!ins.get()) {
		ins.reset(new StatusBar(win));
	}
}

StatusBar* StatusBar::get()
{
	return ins.get();
}

void StatusBar::showRecognizing()
{
	setText(L"正在识别图像中的文字...");
}

void StatusBar::showRecognizeDone(long long elapsedMs)
{
	setText(std::format(L"图像识别完成，耗时：{}毫秒", elapsedMs));
}

void StatusBar::setText(const std::wstring& t)
{
	if (t == text) return;
	text = t;
	buildLayout();
	node->paint();
}

void StatusBar::onSize(void* e)
{
	auto win = WindowMain::get();
	auto h{ 22.f * win->dpi };
	auto y{ win->h - h };
	node->setPosSize(0.f, y, win->w, h);
	// 尺寸/DPI 变化时需要按新字号重建 layout
	buildLayout();
	node->paint();
}

void StatusBar::buildLayout()
{
	auto win = WindowMain::get();
	auto d2d = D2D::get();
	// 用容器宽度作为 layout 的最大宽度；高度不限（单行即可）
	auto size = node->visual.Size();
	layout = d2d->createTextLayout(text, std::max(0.f, size.x), FLT_MAX);
	layout->SetFontSize(12.f * win->dpi, { 0, INT_MAX });
	DWRITE_TEXT_METRICS metrics{};
	layout->GetMetrics(&metrics);
	// 左侧内边距 8 * dpi；垂直居中
	auto padding = 8.f * win->dpi;
	textPos.x = padding;
	textPos.y = (size.y - metrics.height) / 2.f;
}

void StatusBar::onPaint(void* e)
{
	auto tuplePtr = static_cast<std::tuple<Node*, ID2D1DeviceContext*>*>(e);
	auto ctx = std::get<1>(*tuplePtr);
	auto size = node->visual.Size();
	ComPtr<ID2D1SolidColorBrush> bgBrush;
	ctx->CreateSolidColorBrush(ColorA(0xe0e0e0FF).getD2DColor(), bgBrush.GetAddressOf());
	D2D1_RECT_F rr{ 0.f, 0.f, size.x, size.y };
	ctx->FillRectangle(rr, bgBrush.Get());
	if (!layout) return;
	ComPtr<ID2D1SolidColorBrush> textBrush;
	ctx->CreateSolidColorBrush(ColorA(0x333333FF).getD2DColor(), textBrush.GetAddressOf());
	ctx->DrawTextLayout(textPos, layout.Get(), textBrush.Get());
}
