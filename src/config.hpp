#pragma once

#include <string>
#include <windows.h>

namespace hud {

struct PluginConfig {
    std::wstring userName = L"AviUtl2 user";
    bool openAfterSave = true;
    int paletteIndex = 0; // 0: ダーク, 1: ライト, 2: 春, 3: 夏, 4: 秋, 5: 冬
};

// 設定のロード・セーブ
PluginConfig LoadConfig();
void SaveConfig(const PluginConfig& config);

// 設定モーダルダイアログを表示
void ShowConfigDialog(HWND parentWnd, HINSTANCE hInst);

} // namespace hud
