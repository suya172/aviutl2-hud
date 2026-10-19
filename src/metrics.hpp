#pragma once

#include <cstdint>
#include <string>
#include <windows.h>
#include <aviutl2_sdk/plugin2.h>

namespace hud {

struct HudMetrics {
    std::wstring projectName;       // プロジェクトファイル名 (未保存時は "Untitled.au2")
    int videoWidth = 0;             // 解像度 横
    int videoHeight = 0;            // 解像度 縦
    double frameRate = 0.0;         // フレームレート
    int currentFrame = 0;           // 現在フレーム
    int totalFrames = 0;            // 総フレーム数
    int selectedObjectCount = 0;    // 選択中のオブジェクト数
    int totalObjectCount = 0;       // 総オブジェクト数
    std::wstring timeCode;          // タイムコード ("00:03:24:12")
    std::wstring sessionDuration;   // 起動からの経過時間 ("01h 24m")
    std::wstring captureTimestamp;  // 撮影日時 ("YYYY-MM-DD HH:MM:SS")
};

// プラグインロード時にセッションタイマーを開始
void InitializeSessionTimer();

// EDIT_SECTION および EDIT_HANDLE からメトリクスを収集
HudMetrics CollectMetrics(EDIT_SECTION* edit_section, EDIT_HANDLE* editHandle);

} // namespace hud
