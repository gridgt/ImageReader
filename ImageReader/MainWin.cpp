#include "pch.h"
#include "MainWin.h"
#include "Render.h"
#include "Btn.h"
#include "Img.h"

std::unique_ptr<MainWin> win;

MainWin::MainWin()
{

}

MainWin::~MainWin()
{

}

void MainWin::init()
{
	win = std::make_unique<MainWin>();
	win->setPos();
	win->createWin();
	win->render = std::make_unique<Render>(win.get());
	win->btnMini = std::make_unique<Btn>(L"\ue6e8", 2, win.get());
	win->btnMax = std::make_unique<Btn>(L"\ue6e5", 1, win.get());
	win->btnRestore = std::make_unique<Btn>(L"\ue6e9", 1, win.get());
	win->btnRestore->isVisible = false;
	win->btnClose = std::make_unique<Btn>(L"\ue6e7", 0, win.get());
	win->img = std::make_unique<Img>(win.get());
	win->onSize(win->w, win->h);
	win->show();
}

LRESULT MainWin::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto self = reinterpret_cast<MainWin*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	if (!self) return DefWindowProc(hwnd, msg, wParam, lParam);
	if (msg == WM_SIZE) {
		if (wParam == SIZE_MAXIMIZED) self->onMaximize();
		if (wParam == SIZE_RESTORED) self->onRestore();
		self->onSize(LOWORD(lParam), HIWORD(lParam));
	}
	else if (msg == WM_NCHITTEST) {
		POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		ScreenToClient(hwnd, &pt);
		return self->onHitTest(pt);
	}
	else if (msg == WM_MOUSEMOVE) {
		if (self->isMouseDown) {
			self->onMouseDrag(LOWORD(lParam), HIWORD(lParam));
		}
		else {
			self->onMouseMove(LOWORD(lParam), HIWORD(lParam));
		}
	}
	else if (msg == WM_MOUSELEAVE) {
		self->onMouseLeave();
	}
	else if (msg == WM_LBUTTONDOWN) {
		self->onMouseDown(LOWORD(lParam), HIWORD(lParam));
	}
	else if (msg == WM_LBUTTONUP) {
		self->onMouseUp(LOWORD(lParam), HIWORD(lParam));
	}
	else if (msg == WM_GETMINMAXINFO) {
		MINMAXINFO* mmi = (PMINMAXINFO)lParam;
		self->onMinMaxInfo(mmi);
	}
	else if (msg == WM_PAINT)
	{
		self->onPaint();
	}
	else if (msg == WM_DROPFILES) {
		self->onDropFiles((HDROP)wParam);
	}
	else if (msg == WM_DESTROY) {
		self->onDestroy();
	}
	else if (msg == WM_DPICHANGED) {
		self->onDpiChange(LOWORD(wParam));
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void MainWin::createWin()
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"ImageReader";
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(100));
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);
	auto style = WS_POPUP | WS_MAXIMIZEBOX | WS_MINIMIZEBOX;
	hwnd = CreateWindowEx(NULL, wcex.lpszClassName, wcex.lpszClassName, style, x, y, w, h, NULL, NULL, wcex.hInstance, NULL);
	DragAcceptFiles(hwnd, TRUE);
	onDpiChange(GetDpiForWindow(hwnd));
	if (!hwnd)
	{
		MessageBox(NULL, L"创建窗口失败", L"系统提示", MB_OK);
		ExitProcess(-1);
	}
	SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	setShadow();
	enableAlpha();
}

void MainWin::setShadow()
{
	MARGINS margins = { 1,1,1,1 };
	DwmExtendFrameIntoClientArea(hwnd, &margins);
	int value = 2;
	DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &value, sizeof(value));
	DwmSetWindowAttribute(hwnd, DWMWA_ALLOW_NCPAINT, &value, sizeof(value));
}

void MainWin::setPos()
{
	w = 1200;
	h = 800;
	RECT workAreaRect;
	BOOL getWorkAreaSuccess = SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0); //工作区矩形
	int workAreaWidth = workAreaRect.right - workAreaRect.left;
	int workAreaHeight = workAreaRect.bottom - workAreaRect.top;
	x = workAreaRect.left + (workAreaWidth - w) / 2;
	y = workAreaRect.top + (workAreaHeight - h) / 2;
}

void MainWin::enableAlpha()
{
	HRGN region = CreateRectRgn(0, 0, -1, -1);
	DWM_BLURBEHIND bb = { 0 };
	bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
	bb.hRgnBlur = region;
	bb.fEnable = TRUE;
	DwmEnableBlurBehindWindow(hwnd, &bb);
	DeleteObject(region);
}

void MainWin::show()
{
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
}

void MainWin::onPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	if(render.get()) render->paint();
	EndPaint(hwnd, &ps);
}

void MainWin::onSize(int w, int h)
{
	this->w = w;
	this->h = h;
	if (render.get()) { 
		render->changeSize(); 
		btnClose->changeSize();
		btnMax->changeSize();
		btnRestore->changeSize();
		btnMini->changeSize();
		img->changeSize();
	}
}

void MainWin::onMaximize()
{
	btnMax->isVisible = false;
	btnMax->isHover = false;
	btnRestore->isVisible = true;
}

void MainWin::onRestore()
{
	btnRestore->isVisible = false;
	btnRestore->isHover = false;
	btnMax->isVisible = true;
}

void MainWin::onMinMaxInfo(MINMAXINFO* mmi)
{
	RECT workAreaRect;
	BOOL getWorkAreaSuccess = SystemParametersInfo(SPI_GETWORKAREA, 0, &workAreaRect, 0);
	mmi->ptMaxPosition.x = workAreaRect.left; //最大化时，窗口坐标
	mmi->ptMaxPosition.y = workAreaRect.top;
	mmi->ptMaxSize.x = workAreaRect.right - workAreaRect.left; //最大化时，窗口尺寸
	mmi->ptMaxSize.y = workAreaRect.bottom - workAreaRect.top;
	mmi->ptMinTrackSize.x = 680; //窗口最小可拖拽尺寸。
	mmi->ptMinTrackSize.y = 500;
}

void MainWin::onMouseMove(int x, int y)
{
	if (!isMouseTracking) { //类的私有变量
		TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = hwnd;
		TrackMouseEvent(&tme); //开始跟踪鼠标离开事件
		isMouseTracking = true;
	}
	btnClose->hitTest(x, y);
	btnMax->hitTest(x, y);
	btnRestore->hitTest(x, y);
	btnMini->hitTest(x, y);
	img->changeCursor(x, y);
}

void MainWin::onMouseDrag(int x, int y)
{

}

void MainWin::onMouseDown(int x, int y)
{
	isMouseDown = true;
	if (btnClose->isHover) {
		PostMessage(hwnd, WM_CLOSE, 0, 0);
	}
	else if (btnMax->isHover) {
		ShowWindow(hwnd, SW_SHOWMAXIMIZED);
	}
	else if (btnRestore->isHover) {
		ShowWindow(hwnd, SW_RESTORE);
	}
	else if (btnMini->isHover) {
		ShowWindow(hwnd, SW_SHOWMINIMIZED);
	}
}

void MainWin::onMouseUp(int x, int y)
{
	isMouseDown = false;
}

void MainWin::onMouseLeave()
{
	isMouseTracking = false;
	btnClose->hitTest(-1, -1);
	btnMax->hitTest(-1, -1);
	btnRestore->hitTest(-1, -1);
	btnMini->hitTest(-1, -1);
}

void MainWin::onDpiChange(int dpi)
{
	this->dpi = dpi / 96.0f;
	if(render.get()) render->changeSize();
}

LRESULT MainWin::onHitTest(const POINT& pt)
{
	RECT rect;
	GetClientRect(hwnd, &rect);
	const int w = 5;  // 边框宽度
	if (pt.x < w && pt.y < w) return HTTOPLEFT; //左上角    
	if (pt.x >= rect.right - w && pt.y < w) return HTTOPRIGHT; // 右上角    
	if (pt.x < w && pt.y >= rect.bottom - w) return HTBOTTOMLEFT;// 左下角    
	if (pt.x >= rect.right - w && pt.y >= rect.bottom - w) return HTBOTTOMRIGHT; // 右下角    
	if (pt.y < w) return HTTOP;// 上边    
	if (pt.y >= rect.bottom - w) return HTBOTTOM;// 下边    
	if (pt.x < w) return HTLEFT;// 左边    
	if (pt.x >= rect.right - w) return HTRIGHT;// 右边
	if (pt.y < render->headerRect.bottom && pt.x < btnMini->rect.left) return HTCAPTION;
	return HTCLIENT;
}

void MainWin::onDestroy()
{
	DragAcceptFiles(hwnd, FALSE);
	PostQuitMessage(0);
}
void MainWin::onDropFiles(HDROP hDrop)
{
	POINT pt;
	DragQueryPoint(hDrop, &pt);
	ScreenToClient(hwnd, &pt);
	if (pt.y > render->headerRect.bottom) {
		DragFinish(hDrop);
		return;
	}
	UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
	UINT pathLen = DragQueryFile(hDrop, 0, NULL, 0);
	std::wstring path(pathLen + 1, L'\0');
	DragQueryFile(hDrop, 0, path.data(), pathLen + 1);
	path.resize(pathLen);
	DragFinish(hDrop); 
	img->load(path);
}