#include "src/api/pipelines/ocr.h"
#include "ocrAPI.h"
void ocrInit()
{
    PaddleOCRParams params;
    params.device = "cpu"; // 推理时使用GPU。请确保编译时添加 -DWITH_GPU=ON 选项，否则使用CPU。
	params.doc_orientation_classify_model_name = absl::nullopt;
    params.doc_orientation_classify_model_dir = absl::nullopt;
    params.use_doc_orientation_classify = false;  // 不使用文档方向分类模型。
    params.use_doc_unwarping = false; // 不使用文本图像矫正模型。
    params.use_textline_orientation = false; // 不使用文本行方向分类模型。
    params.text_detection_model_dir = "PP-OCRv5_server_det_infer"; // 使用 PP-OCRv5_server_det 模型进行检测。
    params.text_recognition_model_dir = "PP-OCRv5_server_rec_infer"; // 使用 PP-OCRv5_server_rec 模型进行识别。
    std::vector<std::unique_ptr<BaseCVResult>> outputs = PaddleOCR(params).Predict("1.png");
    for (std::unique_ptr<BaseCVResult>& output : outputs) {
        output->Print();
        output->SaveToImg("./output/");
        output->SaveToJson("./output/");
    }
}