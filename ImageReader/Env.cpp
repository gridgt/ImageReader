
#include "pch.h"
#include "Env.h"
#include "MainWin.h"

std::unique_ptr<Env> env;

Env::Env() :dq{ winrt::Windows::System::DispatcherQueue::GetForCurrentThread() }
{
    initOcr();
}

Env::~Env()
{

}

void Env::init()
{
    initDQCtrl();
    env = std::make_unique<Env>();
	MainWin::init();
}

winrt::Windows::System::DispatcherQueue& Env::getDQ()
{
    return env->dq;
}

OcrLite* Env::getOcr()
{
    return &env->ocr;
}

void Env::initDQCtrl()
{
    auto optSize = sizeof(DispatcherQueueOptions);
    DispatcherQueueOptions options{ optSize,DQTYPE_THREAD_CURRENT,DQTAT_COM_NONE };
    static winrt::Windows::System::DispatcherQueueController controller{ nullptr };
    auto dqc = winrt::put_abi(controller);
    auto hr = CreateDispatcherQueueController(options, reinterpret_cast<ABI::Windows::System::IDispatcherQueueController**>(dqc));
    if (FAILED(hr))
    {
        MessageBox(NULL, L"无法创建DispatcherQueueController", L"系统提示", MB_OK);
        ExitProcess(-1);
    }
}

void Env::initOcr()
{
    auto coreNum = std::thread::hardware_concurrency();
    ocr.setNumThread(coreNum);
    ocr.initLogger(false, false, false);
    ocr.setGpuIndex(-1);
    bool initModelsRet = ocr.initModels("models/ch_PP-OCRv3_det_infer.onnx",
        "models/ch_ppocr_mobile_v2.0_cls_infer.onnx",
        "models/ch_PP-OCRv3_rec_infer.onnx",
        "models/ppocr_keys_v1.txt");
}
