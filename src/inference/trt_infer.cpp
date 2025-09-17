#include <string>
#include <vector>
#include "../common/logging.cpp"

#ifdef USE_TRT
// Include your TensorRT headers here and implement real inference.
#endif

struct Detection {
    float x, y, w, h, score;
    std::string label;
};

std::vector<Detection> run_inference_stub(const std::string& /*frame_bytes*/) {
    // pretend we detected 2 boxes
    return {
        {0.1f, 0.2f, 0.3f, 0.2f, 0.92f, "person"},
        {0.5f, 0.4f, 0.2f, 0.2f, 0.81f, "bicycle"}
    };
}
