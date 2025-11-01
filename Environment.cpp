#include <filesystem>
#include <shlobj.h>
#include <WebView2EnvironmentOptions.h>
#include "Environment.h"
#include "WinImgReader.h"

std::unique_ptr<Environment> envIns;
//winrt::Windows::System::DispatcherQueueController controller{ nullptr };


Environment::Environment():dq{ winrt::Windows::System::DispatcherQueue::GetForCurrentThread() }
{
}

Environment::~Environment()
{

}

bool Environment::init()
{
    /*DispatcherQueueOptions options{
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_NONE
    };
    CreateDispatcherQueueController(options,
        reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(winrt::put_abi(controller)));*/

	auto env = new Environment();
	envIns.reset(env);
    WinImgReader::init();
    if (!envIns->initCOM())
    {
        return false;
    }
    if (!envIns->initDataPath())
    {
        return false;
    }
    if (!envIns->checkVersion())
    {
        return false;
    }
    if (!envIns->initEnv()) {
		return false;
    }
    return true;
}

void Environment::uninit()
{
    CoUninitialize();
}

Environment* Environment::get()
{
	return envIns.get();
}

bool Environment::initCOM()
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr))
    {
        MessageBox(NULL, L"无法初始化COM库", L"Error", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}

bool Environment::initDataPath()
{
    PWSTR pathTmp;
    auto hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &pathTmp);
    if (FAILED(hr))
    {
        CoTaskMemFree(pathTmp);
        MessageBox(NULL, L"无法得到应用数据目录", L"Error", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return false;
    }
    std::filesystem::path appDir{pathTmp};
    CoTaskMemFree(pathTmp);
    appDir /= "ImageReader";
    dataPath = appDir.wstring();
    return true;
}
bool Environment::checkVersion()
{
    std::wstring regSubKey = L"\\Microsoft\\EdgeUpdate\\Clients\\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}";
    bool hasRuntime = checkRegKey(HKEY_LOCAL_MACHINE, L"SOFTWARE\\WOW6432Node" + regSubKey);
    if (hasRuntime)
    {
        return true;
    }
    hasRuntime = checkRegKey(HKEY_CURRENT_USER, L"Software" + regSubKey);
    if (hasRuntime)
    {
        return true;
    }
    auto result = MessageBox(nullptr, L"缺少WebView2系统组件", L"系统提示",
                             MB_OKCANCEL | MB_ICONINFORMATION | MB_DEFBUTTON1);
    if (result == IDOK)
    {
        ShellExecute(0, 0, L"https://go.microsoft.com/fwlink/p/?LinkId=2124703", 0, 0, SW_SHOW);
    }
    CoUninitialize();
    return false;
}
bool Environment::checkRegKey(const HKEY &key, const std::wstring &subKey)
{
    size_t bufferSize = 20;
    std::wstring valueBuf;
    valueBuf.resize(bufferSize);
    auto valueSize = static_cast<DWORD>(bufferSize * sizeof(wchar_t));
    auto rc = RegGetValue(key, subKey.c_str(), L"pv", RRF_RT_REG_SZ,
                          nullptr, static_cast<void *>(valueBuf.data()), &valueSize);
    if (rc == ERROR_SUCCESS)
    {
        valueSize /= sizeof(wchar_t);
        valueBuf.resize(static_cast<size_t>(valueSize - 1));
        if (valueBuf.empty() || valueBuf == L"0.0.0.0")
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool Environment::initEnv()
{
    auto envReady = Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
        [this](HRESULT result, ICoreWebView2Environment *env)
        {
            if (FAILED(result))
            {
                MessageBox(NULL, L"Failed to create webview2 environment", L"Error", MB_OK | MB_ICONERROR);
                ExitProcess(-1);
            }
			this->env = env;
            WinImgReader::get()->initView(env);
            return S_OK;
        });
    auto options = Make<CoreWebView2EnvironmentOptions>();
    //options->put_AdditionalBrowserArguments(L"--allow-file-access-from-files");  没用！！
    auto hr = CreateCoreWebView2EnvironmentWithOptions(NULL, dataPath.data(), options.Get(), envReady.Get());
    if (FAILED(hr))
    {
        return false;
    }
	return true;
}
