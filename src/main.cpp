#include <atomic>
#include <cstdint>
#include <thread>
#include <windows.h>
#include <objbase.h>
#include <shellapi.h>
#include <aviutl2_sdk/plugin2.h>

#include "capture.hpp"
#include "config.hpp"
#include "dialog.hpp"
#include "hud_renderer.hpp"
#include "metrics.hpp"

#define EXPORT extern "C" __declspec(dllexport)

static HOST_APP_TABLE* g_host = nullptr;
static EDIT_HANDLE* g_editHandle = nullptr;
static HINSTANCE g_hInst = nullptr;
static std::atomic<int> g_activeWorkers{0};

static COMMON_PLUGIN_TABLE g_pluginTable = {
    .name = L"HUD Screenshot Merger",
    .information = L"HUD Screenshot Merger v0.1.0",
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        g_hInst = hinstDLL;
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        // DLL アンロード時のスレッド競合防止: 実行中ワーカーが完了するまで待機
        int retries = 50;
        while (g_activeWorkers.load() > 0 && --retries >= 0) {
            Sleep(20);
        }
    }
    return TRUE;
}

EXPORT auto RequiredVersion() -> uint32_t {
    return 2010301; // AviUtl2 2.1.3a 以降
}

EXPORT auto GetCommonPluginTable() -> COMMON_PLUGIN_TABLE* {
    return &g_pluginTable;
}

static void OnCaptureMenuClicked(EDIT_SECTION* edit_section) {
    if (!g_editHandle || !edit_section) {
        return;
    }

    // 1. 編集セクション（編集ロック）内でメトリクス情報を即座に収集
    hud::HudMetrics metrics = hud::CollectMetrics(edit_section, g_editHandle);

    // 2. 編集セクションを速やかに終了させ、別スレッドで保存ダイアログとレンダリングを実行
    g_activeWorkers.fetch_add(1);
    std::thread([metrics = std::move(metrics)]() {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

        // 編集ロックが完全に解放されるのを待機
        Sleep(50);

        HWND parentWnd = g_editHandle->get_host_app_window ? g_editHandle->get_host_app_window() : nullptr;

        // 設定の読み込み
        hud::PluginConfig config = hud::LoadConfig();

        // (1) 保存ダイアログ表示
        SYSTEMTIME st;
        GetLocalTime(&st);
        wchar_t defaultName[MAX_PATH];
        swprintf_s(defaultName, L"progress_%04d%02d%02d_%02d%02d%02d.png",
                   st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

        std::wstring savePath;
        if (!hud::PromptSavePath(parentWnd, defaultName, savePath)) {
            if (SUCCEEDED(hr)) CoUninitialize();
            g_activeWorkers.fetch_sub(1);
            return; // ユーザーがキャンセル
        }

        // (2) レンダリング済み映像フレームの取得
        hud::VideoFrameBuffer frameBuffer;
        if (!hud::CaptureFrame(g_editHandle, metrics.currentFrame, frameBuffer)) {
            MessageBoxW(parentWnd, L"映像フレームのレンダリング取得に失敗しました。", L"HUD Screenshot Merger", MB_OK | MB_ICONERROR);
            if (SUCCEEDED(hr)) CoUninitialize();
            g_activeWorkers.fetch_sub(1);
            return;
        }

        // (3) HUD レンダリング・画像結合・PNG 保存
        if (!hud::RenderAndSave(frameBuffer, metrics, config, savePath)) {
            MessageBoxW(parentWnd, L"画像の結合または保存に失敗しました。", L"HUD Screenshot Merger", MB_OK | MB_ICONERROR);
            if (SUCCEEDED(hr)) CoUninitialize();
            g_activeWorkers.fetch_sub(1);
            return;
        }

        // (4) 保存完了後に画像を開く (設定で有効な場合)
        if (config.openAfterSave) {
            ShellExecuteW(nullptr, L"open", savePath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        } else {
            MessageBoxW(parentWnd, L"進捗画像を保存しました。", L"HUD Screenshot Merger", MB_OK | MB_ICONINFORMATION);
        }

        if (SUCCEEDED(hr)) {
            CoUninitialize();
        }
        g_activeWorkers.fetch_sub(1);
    }).detach();
}

static void OnConfigMenuClicked(HWND hwnd, HINSTANCE dll_hinst) {
    hud::ShowConfigDialog(hwnd, dll_hinst ? dll_hinst : g_hInst);
}

EXPORT auto RegisterPlugin(HOST_APP_TABLE* host) -> void {
    g_host = host;

    // 編集ハンドルを取得
    if (host->create_edit_handle) {
        g_editHandle = host->create_edit_handle();
    }

    // セッション経過時間タイマーの開始
    hud::InitializeSessionTimer();

    // 1. プレビュー画面 / 編集メニューに登録
    if (host->register_edit_menu) {
        host->register_edit_menu(L"進捗画像をキャプチャして保存...", OnCaptureMenuClicked);
    }

    // 2. ファイル -> エクスポート メニューに登録
    if (host->register_export_menu) {
        host->register_export_menu(L"進捗画像をキャプチャして保存...", OnCaptureMenuClicked);
    }

    // 3. タイムライン・オブジェクト右クリックメニューに登録 (どこからでも呼べるよう網羅)
    if (host->register_layer_menu) {
        host->register_layer_menu(L"進捗画像をキャプチャして保存...", OnCaptureMenuClicked);
    }
    if (host->register_object_menu) {
        host->register_object_menu(L"進捗画像をキャプチャして保存...", OnCaptureMenuClicked);
    }

    // 4. 設定 -> プラグイン メニューに登録
    if (host->register_config_menu) {
        host->register_config_menu(L"HUD Screenshot Merger", OnConfigMenuClicked);
    }
}
