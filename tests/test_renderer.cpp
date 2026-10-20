#include <filesystem>
#include <iostream>
#include <vector>
#include <windows.h>
#include <gdiplus.h>

#include "capture.hpp"
#include "config.hpp"
#include "hud_renderer.hpp"
#include "metrics.hpp"

int main() {
    std::cout << "[hud_test] Starting UI test rendering..." << std::endl;

    // 1. 1920x1080 の黒背景ダミーフレームバッファを作成
    const int width = 1920;
    const int height = 1080;
    const int pitch = width * 4;
    size_t totalBytes = static_cast<size_t>(pitch) * height;

    hud::VideoFrameBuffer frame;
    frame.width = width;
    frame.height = height;
    frame.pitch = pitch;
    frame.pixels.resize(totalBytes, 0);

    // BGRA 各ピクセルを不透明な黒 (B=16, G=16, R=16, A=255) に初期化
    for (size_t i = 0; i < totalBytes; i += 4) {
        frame.pixels[i + 0] = 16;  // B (わずかに深いチャコール)
        frame.pixels[i + 1] = 16;  // G
        frame.pixels[i + 2] = 16;  // R
        frame.pixels[i + 3] = 255; // A (不透明)
    }

    // 2. ダミーのメトリクスデータを用意
    hud::HudMetrics metrics;
    metrics.projectName = L"sample.aup2";
    metrics.videoWidth = width;
    metrics.videoHeight = height;
    metrics.frameRate = 30.00;
    metrics.currentFrame = 320; // Frame 321 / 418
    metrics.totalFrames = 418;
    metrics.totalObjectCount = 3;
    metrics.timeCode = L"00:00:10:20";
    metrics.sessionDuration = L"00h 00m";
    metrics.captureTimestamp = L"2026-10-20 10:25:00";

    // 3. 設定データ
    hud::PluginConfig config;
    config.userName = L"AviUtl2 user";
    config.paletteIndex = 0; // ダークテーマ
    config.openAfterSave = false;

    // 4. 出力先ディレクトリの作成と合成レンダリング
    const std::filesystem::path outputDir = "tests/output";
    std::error_code ec;
    std::filesystem::create_directories(outputDir, ec);

    const std::wstring outputPath = (outputDir / "test_output.png").wstring();
    bool success = hud::RenderAndSave(frame, metrics, config, outputPath);

    if (success) {
        std::cout << "[hud_test] Successfully generated test image: tests/output/test_output.png" << std::endl;
        return 0;
    } else {
        std::cerr << "[hud_test] Failed to render and save test image." << std::endl;
        return 1;
    }
}
