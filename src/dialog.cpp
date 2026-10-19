#include "dialog.hpp"
#include <commdlg.h>
#include <vector>

namespace hud {

bool PromptSavePath(HWND parentWnd, const std::wstring& defaultFilename, std::wstring& outPath) {
    // 長いパス (Long Paths) に対応するため 32768 文字バッファを確保
    constexpr size_t BUFFER_SIZE = 32768;
    std::vector<wchar_t> szFile(BUFFER_SIZE, L'\0');

    if (!defaultFilename.empty() && defaultFilename.length() < BUFFER_SIZE) {
        wcscpy_s(szFile.data(), BUFFER_SIZE, defaultFilename.c_str());
    }

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = parentWnd;
    ofn.lpstrFile = szFile.data();
    ofn.nMaxFile = static_cast<DWORD>(BUFFER_SIZE);
    ofn.lpstrFilter = L"PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = nullptr;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.lpstrDefExt = L"png";
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn)) {
        outPath = szFile.data();
        return true;
    }

    return false;
}

} // namespace hud
