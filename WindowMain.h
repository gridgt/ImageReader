#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <memory>

#include <filesystem>
#include <shlobj.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <windows.h>
#include <windowsx.h>
#include <wrl.h>
#include <WebView2.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Data.Json.h>

#include <DispatcherQueue.h>
#include <winrt/base.h>
#include <windows.ui.composition.interop.h>
#include <winrt/Windows.UI.Composition.Desktop.h>
#include <winrt/Windows.Media.Ocr.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>             
#include <winrt/Windows.Graphics.Imaging.h>    
#include <winrt/Windows.Storage.Streams.h>     

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

using namespace winrt::Windows::Media::Ocr;
using namespace winrt::Windows::Graphics::Imaging;
using namespace winrt::Windows::Storage::Streams;
using namespace winrt::Windows::Data::Json;
using namespace Microsoft::WRL;

#define MSG_BACK_ID WM_APP + 10

class WebView;
class Message;
class WindowMain
{
	public:
		WindowMain();
		~WindowMain();
		static void init();
		static void initView(ICoreWebView2Environment* env);
		static WindowMain* get();
		void hittest(Message* msg);
		void minimize(Message* msg);
		void maximize(Message* msg);
		void close(Message* msg);
		void restore(Message* msg);
		void show(Message* msg);
		virtual void exec(Message* msg);
		void on(Message* msg);
		void off(Message* msg);
	public:
		HWND hwnd;
		ComPtr<ICoreWebView2_22> webview;
	protected:
		void createWindow();
	protected:
		int w{ 500 }, h{500};
		float dpr;
		std::unordered_map<winrt::hstring, std::vector<Message*>> eventTargets;
		ComPtr<ICoreWebView2Controller> webviewCtrl;
	private:
		void onFileDrop(HDROP hDrop);
		void createTessAPI();
		winrt::Windows::Foundation::IAsyncAction readImg(const std::wstring path);
		void createCompCtrl();

		static LRESULT CALLBACK winMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
		void setMinMaxInfo(LPMINMAXINFO lpMMI);
		void onSize(UINT param);
		HRESULT ctrlReady(HRESULT result, ICoreWebView2CompositionController* ctrlComp);
		void bindCompCtrlToHwnd();

		void addRequestFilter();
		void addMsgReceiver();
		void addDomLoader();
		JsonObject getParam(ICoreWebView2WebMessageReceivedEventArgs* args);
		HRESULT resRequested(ICoreWebView2* webview, ICoreWebView2WebResourceRequestedEventArgs* args);
		std::wstring getContentType(const std::wstring& fileName);
	private:
		winrt::Windows::UI::Composition::Compositor compositor{ nullptr };
		winrt::Windows::UI::Composition::Desktop::DesktopWindowTarget winTarget{ nullptr };
		winrt::Windows::UI::Composition::ContainerVisual rootVisual{ nullptr };
		winrt::Windows::UI::Composition::ContainerVisual webviewVisual{ nullptr };
		ComPtr<ICoreWebView2CompositionController> ctrlComp;
		bool isMouseTracking{ false };
		tesseract::TessBaseAPI* tessAPI;
};

