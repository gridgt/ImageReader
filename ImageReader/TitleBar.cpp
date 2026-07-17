#include "pch.h"
#include "TitleBar.h"
#include "WindowBase.h"
#include "D2D.h"
#include "Node.h"

static std::unique_ptr<TitleBar> ins;

TitleBar::TitleBar(WindowBase* win)
{
	btn0 = win->root->createChild(L"btn0");
	btn1 = win->root->createChild(L"btn1");
	btn2 = win->root->createChild(L"btn2");

	btn0->initSurface();
	btn1->initSurface();
	btn2->initSurface();


	//btn0->setBackgroundColor(0xEEEEEEFF);
	//btn1->setBackgroundColor(0xEEEEEEFF);
	//btn2->setBackgroundColor(0xEEEEEEFF);

	btn0->onSizeChange([this](auto& e) {this->onSize(e);});
	btn1->onSizeChange([this](auto& e) {this->onSize(e);});
	btn2->onSizeChange([this](auto& e) {this->onSize(e);});

	btn0->onMouseEnter([this](auto& e) {this->onEnter(e);});
	btn1->onMouseEnter([this](auto& e) {this->onEnter(e);});
	btn2->onMouseEnter([this](auto& e) {this->onEnter(e);});


	btn0->onMouseLeave([this](auto& e) {this->onLeave(e);});
	btn1->onMouseLeave([this](auto& e) {this->onLeave(e);});
	btn2->onMouseLeave([this](auto& e) {this->onLeave(e);});


	auto d2d = D2D::get();
	icon0 = d2d->createTextLayout(L"\ue74c", FLT_MAX, FLT_MAX);
	icon0->SetFontFamilyName(L"iconfont", { 0,INT_MAX });
	icon1 = d2d->createTextLayout(L"\ue74c", FLT_MAX, FLT_MAX);
	icon1->SetFontFamilyName(L"iconfont", { 0,INT_MAX });
	icon2 = d2d->createTextLayout(L"\ue74c", FLT_MAX, FLT_MAX);
	icon2->SetFontFamilyName(L"iconfont", { 0,INT_MAX });
}

TitleBar::~TitleBar()
{
}

void TitleBar::init(WindowBase* win)
{
	ins.reset(new TitleBar(win));

}
void TitleBar::onSize(const EventArg& e)
{
	auto node = dynamic_cast<Node*>(e.target);
	auto win = node->win;
	// 全部使用逻辑像素（DIPs）
	auto btnW{ 32.f }, btnH{ 30.f };
	node->visual.Size({ btnW,btnH });
	DWRITE_TEXT_METRICS metrics;
	if (node == btn0) {
		btn0->resizeSurface();
		node->visual.Offset({ win->w - btnW * 3,0.f,0.f });
		icon0->GetMetrics(&metrics);
		icon0->SetFontSize(12.f, { 0,INT_MAX });
		iconPos0.x = (btnW - metrics.width) / 2;
		iconPos0.y = (btnH - metrics.height) / 2;
	}
	else if (node == btn1) {
		btn1->resizeSurface();
		node->visual.Offset({ win->w - btnW * 2,0.f,0.f });
		icon1->GetMetrics(&metrics);
		iconPos1.x = (btnW - metrics.width) / 2;
		iconPos1.y = (btnH - metrics.height) / 2;
	}
	else if (node == btn2) {
		btn2->resizeSurface();
		node->visual.Offset({ win->w - btnW,0.f,0.f });
		icon1->GetMetrics(&metrics);
		iconPos1.x = (btnW - metrics.width) / 2;
		iconPos1.y = (btnH - metrics.height) / 2;
	}
	paint(node);
}

void TitleBar::onEnter(const MouseEventArg& e)
{
	auto node = dynamic_cast<Node*>(e.target);
	if (node == btn0) {
		//btn0->setBackgroundColor(0xE0E0E0FF);
		paint(btn0);
	}
	else if (node == btn1) {
		//btn1->setBackgroundColor(0xE0E0E0FF);
		paint(btn1);
	}
	else if (node == btn2) {
		//btn2->setBackgroundColor(0xF44336ff);
		paint(btn2);
	}
}

void TitleBar::onLeave(const EventArg& e)
{
	//auto node = dynamic_cast<Node*>(e.target);
	//if (node == btn0) {
	//	btn0->setBackgroundColor(0xEEEEEEFF);
	//}
	//else if (node == btn1) {
	//	btn1->setBackgroundColor(0xEEEEEEFF);
	//}
	//else if (node == btn2) {
	//	btn2->setBackgroundColor(0xEEEEEEFF);
	//}
}

void TitleBar::paint(Node* btn)
{
	auto [s, d2d] = btn->paintStart();
	if (btn->id == L"btn0") {
		if (btn->isHover) {
			auto size = btn->surface.Size();
			auto r{ 2 };
			D2D1_ROUNDED_RECT rr = { {0,0,size.Width,size.Height}, r,r };
			ComPtr<ID2D1SolidColorBrush> bgBrush;
			ColorA color(0xE0E0E0FF);
			d2d->CreateSolidColorBrush(color.getD2DColor(), bgBrush.GetAddressOf());
			d2d->FillRoundedRectangle(rr, bgBrush.Get());
		}
		ComPtr<ID2D1SolidColorBrush> textBrush;
		ColorA color(0x000000ff);
		d2d->CreateSolidColorBrush(color.getD2DColor(), textBrush.GetAddressOf());
		d2d->DrawTextLayout(iconPos0, icon0.Get(), textBrush.Get());
	}
	s->EndDraw();
}
