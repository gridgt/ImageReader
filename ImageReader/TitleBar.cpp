#include "pch.h"
#include "TitleBar.h"
#include "WindowBase.h"
#include "Node.h"

static std::unique_ptr<TitleBar> ins;

TitleBar::TitleBar(WindowBase* win)
{
	btn0 = win->root->createChild();
	btn1 = win->root->createChild();
	btn2 = win->root->createChild();

	btn0->initSurface();
	btn1->initSurface();
	btn2->initSurface();


	btn0->setBackgroundColor(0xEEEEEEFF);
	btn1->setBackgroundColor(0xEEEEEEFF);
	btn2->setBackgroundColor(0xEEEEEEFF);

	btn0->onSizeChange([this](auto& e) {this->onSize(e);});
	btn1->onSizeChange([this](auto& e) {this->onSize(e);});
	btn2->onSizeChange([this](auto& e) {this->onSize(e);});

	btn0->onMouseEnter([this](auto& e) {this->onEnter(e);});
	btn1->onMouseEnter([this](auto& e) {this->onEnter(e);});
	btn2->onMouseEnter([this](auto& e) {this->onEnter(e);});


	btn0->onMouseLeave([this](auto& e) {this->onLeave(e);});
	btn1->onMouseLeave([this](auto& e) {this->onLeave(e);});
	btn2->onMouseLeave([this](auto& e) {this->onLeave(e);});
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
	auto btnW{ 32.f * win->dpi }, btnH{30.f * win->dpi };
	if (node == btn0) {
		node->visual.Offset({ win->w - btnW * 3,0.f,0.f });
		node->visual.Size({ btnW,btnH });
	}
	else if (node == btn1) {
		node->visual.Offset({ win->w - btnW * 2,0.f,0.f });
		node->visual.Size({ btnW,btnH });
	}
	else if (node == btn2) {
		node->visual.Offset({ win->w - btnW,0.f,0.f });
		node->visual.Size({ btnW,btnH });
	}
}

void TitleBar::onEnter(const MouseEventArg& e)
{
	auto node = dynamic_cast<Node*>(e.target);
	if (node == btn0) {
		btn0->setBackgroundColor(0xE0E0E0FF);
	}
	else if (node == btn1) {
		btn1->setBackgroundColor(0xE0E0E0FF);
	}
	else if (node == btn2) {
		btn2->setBackgroundColor(0xF44336ff);
	}
}

void TitleBar::onLeave(const EventArg& e)
{
	auto node = dynamic_cast<Node*>(e.target);
	if (node == btn0) {
		btn0->setBackgroundColor(0xEEEEEEFF);
	}
	else if (node == btn1) {
		btn1->setBackgroundColor(0xEEEEEEFF);
	}
	else if (node == btn2) {
		btn2->setBackgroundColor(0xEEEEEEFF);
	}
}
