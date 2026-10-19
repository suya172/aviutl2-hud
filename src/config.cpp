#include "config.hpp"
#include "palette.hpp"
#include "resource.h"

#include <shlwapi.h>

namespace hud {

static std::wstring GetIniFilePath() {
    HMODULE hModule = nullptr;
    GetModuleHandleExW(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&GetIniFilePath),
        &hModule
    );

    wchar_t path[MAX_PATH] = {0};
    GetModuleFileNameW(hModule, path, MAX_PATH);
    PathRemoveFileSpecW(path);
    PathAppendW(path, L"hud_screenshot_merger.ini");
    return std::wstring(path);
}

PluginConfig LoadConfig() {
    PluginConfig config;
    std::wstring iniPath = GetIniFilePath();

    wchar_t userBuf[256] = {0};
    GetPrivateProfileStringW(L"Settings", L"UserName", L"AviUtl2 user", userBuf, 256, iniPath.c_str());
    config.userName = userBuf;

    config.openAfterSave = (GetPrivateProfileIntW(L"Settings", L"OpenAfterSave", 1, iniPath.c_str()) != 0);
    config.paletteIndex = GetPrivateProfileIntW(L"Settings", L"PaletteIndex", 0, iniPath.c_str());

    if (config.paletteIndex < 0 || config.paletteIndex >= 6) {
        config.paletteIndex = 0;
    }

    return config;
}

void SaveConfig(const PluginConfig& config) {
    std::wstring iniPath = GetIniFilePath();

    WritePrivateProfileStringW(L"Settings", L"UserName", config.userName.c_str(), iniPath.c_str());
    WritePrivateProfileStringW(L"Settings", L"OpenAfterSave", config.openAfterSave ? L"1" : L"0", iniPath.c_str());
    WritePrivateProfileStringW(L"Settings", L"PaletteIndex", std::to_wstring(config.paletteIndex).c_str(), iniPath.c_str());
}

static INT_PTR CALLBACK ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_INITDIALOG: {
        // 現在の設定をロードして各コントロールに反映
        PluginConfig cfg = LoadConfig();

        SetDlgItemTextW(hDlg, IDC_EDIT_USERNAME, cfg.userName.c_str());

        HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_PALETTE);
        const auto& palettes = GetPalettes();
        for (const auto& pal : palettes) {
            SendMessageW(hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(pal.name.c_str()));
        }
        SendMessageW(hCombo, CB_SETCURSEL, cfg.paletteIndex, 0);

        CheckDlgButton(hDlg, IDC_CHECK_OPEN_AFTER_SAVE, cfg.openAfterSave ? BST_CHECKED : BST_UNCHECKED);
        return TRUE;
    }

    case WM_COMMAND: {
        int id = LOWORD(wParam);
        if (id == IDOK) {
            wchar_t userBuf[256] = {0};
            GetDlgItemTextW(hDlg, IDC_EDIT_USERNAME, userBuf, 256);
            int selPalette = static_cast<int>(SendDlgItemMessageW(hDlg, IDC_COMBO_PALETTE, CB_GETCURSEL, 0, 0));
            bool checkOpen = (IsDlgButtonChecked(hDlg, IDC_CHECK_OPEN_AFTER_SAVE) == BST_CHECKED);

            PluginConfig cfg;
            cfg.userName = userBuf;
            if (cfg.userName.empty()) {
                cfg.userName = L"AviUtl2 user";
            }
            cfg.paletteIndex = (selPalette >= 0 && selPalette < 6) ? selPalette : 0;
            cfg.openAfterSave = checkOpen;
            SaveConfig(cfg);

            EndDialog(hDlg, IDOK);
            return TRUE;
        } else if (id == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    }

    return FALSE;
}

void ShowConfigDialog(HWND parentWnd, HINSTANCE hInst) {
    DialogBoxParamW(
        hInst,
        MAKEINTRESOURCEW(IDD_CONFIG_DIALOG),
        parentWnd,
        ConfigDlgProc,
        0
    );
}

} // namespace hud
