#pragma once

#include <string>
#include <windows.h>

namespace hud {

// 「名前を付けて保存」コモンダイアログを表示し、保存先パスを取得する
bool PromptSavePath(HWND parentWnd, const std::wstring& defaultFilename, std::wstring& outPath);

} // namespace hud
