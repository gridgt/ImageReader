#include "pch.h"
#include "Env.h"
#include "Window.h"
std::unique_ptr<Env> env;
Env::Env() :dq{ winrt::Windows::System::DispatcherQueue::GetForCurrentThread() }
{
    auto coreNum = std::thread::hardware_concurrency();
    ocr.setNumThread(coreNum);
    ocr.initLogger(false,false, false);
    ocr.setGpuIndex(-1);
    bool initModelsRet = ocr.initModels("models/ch_PP-OCRv3_det_infer.onnx",
        "models/ch_ppocr_mobile_v2.0_cls_infer.onnx", 
        "models/ch_PP-OCRv3_rec_infer.onnx", 
        "models/ppocr_keys_v1.txt");
}
Env::~Env(){
}
void Env::init()
{
    Env::initDispatcherQueueCtrl();
	env = std::make_unique<Env>();
	env->checkRuntimeVersion();
    env->initDataPath();
    env->initWebViewEnv();
}

OcrLite* Env::getOcr()
{
    return &env->ocr;
}

void Env::initDispatcherQueueCtrl()
{
    DispatcherQueueOptions options{
        sizeof(DispatcherQueueOptions),
        DQTYPE_THREAD_CURRENT,
        DQTAT_COM_NONE
    };
    static winrt::Windows::System::DispatcherQueueController controller{ nullptr };
    auto hr = CreateDispatcherQueueController(options,
        reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(winrt::put_abi(controller)));
    if (FAILED(hr))
    {
        MessageBox(NULL, L"无法创建DispatcherQueueController", L"系统提示", MB_OK);
        ExitProcess(-1);
    }
}

void Env::checkRuntimeVersion() {
    LPWSTR versionInfo = nullptr;
    HRESULT hr = GetAvailableCoreWebView2BrowserVersionString(nullptr, &versionInfo);
    if (FAILED(hr) || !versionInfo) {
        MessageBox(nullptr, L"WebView2环境初始化失败", L"系统提示", MB_OK);
        ExitProcess(-1);
    }
    std::wstringstream ss(versionInfo);
    //std::wstring version{ L"9.0.3600.80" };
    CoTaskMemFree(versionInfo);
    std::vector<int> versions;
    std::wstring segment;
    while (std::getline(ss, segment, L'.')) {
        if (!segment.empty()) {
            versions.push_back(std::stoi(segment)); // 宽字符转整数，结尾处有非数字字符也没有问题
        }
    }
    //109.0.1518.78这是 Windows 7/8/8.1 的最后一个支持版本，同时也是 Windows 10/11 的稳定基础版本。
    //115.0.1901.183仅支持Win10/11，可以利用更多新特性，Chromium 115也是一个非常稳定的长期基线。
    std::vector<int> minVersion = { 109,0,1518,78 };   
    if (versions < minVersion) {
        MessageBox(nullptr, L"请安装版本不低于109.0.1518.78的WebView2运行时", L"系统提示", MB_OK);
        ExitProcess(-1);
    }
}

std::filesystem::path Env::getDataPath()
{
    return env->dataPath;
}
ICoreWebView2Environment* Env::getWebViewEnv()
{
    return env->webViewEnv.Get();
}

winrt::Windows::System::DispatcherQueue& Env::getDispatcherQueue()
{
    return env->dq;
}

void Env::initDataPath()
{
    PWSTR pathTmp;
    auto hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &pathTmp);
    dataPath.assign(pathTmp);
    CoTaskMemFree(pathTmp);
    dataPath.append("Sample");
}

void Env::initWebViewEnv()
{
    auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
    options->put_AdditionalBrowserArguments(L"--disable-web-security --allow-file-access-from-files --allow-file-access");
    auto envReadyCB = Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(this, &Env::onEnvReady);
    CreateCoreWebView2EnvironmentWithOptions(nullptr, dataPath.c_str(), options.Get(), envReadyCB.Get());
}

HRESULT Env::onEnvReady(HRESULT result, ICoreWebView2Environment* env)
{
    if (FAILED(result)) {
        MessageBox(nullptr, L"WebView2环境初始化失败", L"系统提示", MB_OK);
        ExitProcess(-1);
    }
    webViewEnv = env;

    Window::create(L"https://app.localhost/index.html");
    return S_OK;
}

std::vector<int> Env::getExeVer()
{
    std::vector<int> version;
    std::vector<wchar_t> exePath(MAX_PATH);
    if (GetModuleFileName(nullptr, exePath.data(), static_cast<DWORD>(exePath.size())) == 0) {
        return version;
    }
    DWORD dummy;
    DWORD versionSize = GetFileVersionInfoSize(exePath.data(), &dummy);
    if (versionSize == 0) {
        return version;
    }
    std::vector<BYTE> versionData(versionSize);
    if (!GetFileVersionInfo(exePath.data(), 0, versionSize, versionData.data())) {
        return version;
    }
    VS_FIXEDFILEINFO* fileInfo = nullptr;
    UINT fileInfoSize = 0;
    if (!VerQueryValue(versionData.data(), L"\\", reinterpret_cast<void**>(&fileInfo), &fileInfoSize)) {
        return version;
    }
    int major = (fileInfo->dwFileVersionMS >> 16) & 0xFFFF;
    int minor = fileInfo->dwFileVersionMS & 0xFFFF;
    int patch = (fileInfo->dwFileVersionLS >> 16) & 0xFFFF;
    int build = fileInfo->dwFileVersionLS & 0xFFFF;

    version.push_back(major);
    version.push_back(minor);
    version.push_back(patch);
    version.push_back(build);
    return version;
}