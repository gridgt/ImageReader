#include <tinyocr/tiny_ocr.h>

#include <cstdio>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

// Тест-харнесс: распознать test_ru.bgr (raw BGR, top-down, без паддинга)
// моделью из ImageReader/Res (сейчас там eslav PP-OCRv5 rec).
int main(int argc, char** argv) {
    const char* dir = (argc > 1) ? argv[1] : "H:/MyProject/ImageReader";
    const int w = 900, h = 140;

    std::string bgrPath = std::string(dir) + "/ocr_models/test_ru.bgr";
    std::ifstream f(bgrPath, std::ios::binary);
    if (!f) { std::fprintf(stderr, "no %s\n", bgrPath.c_str()); return 1; }
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), {});
    f.close();

    std::string resDir = std::string(dir) + "/ImageReader/Res";
    tinyocr::Options opts;
    opts.det_model_path = resDir + "/PP-OCRv6_det_tiny.onnx";
    opts.rec_model_path = resDir + "/PP-OCRv6_rec_tiny.onnx";
    opts.return_word_box = true;
    opts.return_single_char_box = true;

    tinyocr::TinyOcr ocr(opts);
    auto res = ocr.run(data.data(), w, h, 3);
    std::printf("lines=%zu  det=%.1fms rec=%.1fms\n",
                res.lines.size(), res.det_elapse_ms, res.rec_elapse_ms);
    for (const auto& L : res.lines) {
        std::printf("[%4.2f] %s\n", L.score, L.text.c_str());
    }
    return 0;
}
