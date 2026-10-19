#pragma once

#include <cstdint>
#include <vector>
#include <windows.h>
#include <aviutl2_sdk/plugin2.h>

namespace hud {

struct VideoFrameBuffer {
    std::vector<uint8_t> pixels; // BGRA 形式 (GDI+ 直接互換)
    int width = 0;
    int height = 0;
    int pitch = 0;
};

// 指定したフレームのレンダリング結果を同期的に取得する
bool CaptureFrame(EDIT_HANDLE* editHandle, int frame, VideoFrameBuffer& outBuffer);

} // namespace hud
