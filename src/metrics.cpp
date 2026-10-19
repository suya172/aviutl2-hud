#include "metrics.hpp"

#include <chrono>
#include <iomanip>
#include <sstream>
#include <shlwapi.h>

namespace hud {

static std::chrono::steady_clock::time_point s_startTime;
static bool s_timerInitialized = false;

void InitializeSessionTimer() {
    s_startTime = std::chrono::steady_clock::now();
    s_timerInitialized = true;
}

static std::wstring FormatTimeCode(int frame, double fps) {
    if (fps <= 0.0) fps = 30.0;
    int totalSeconds = static_cast<int>(frame / fps);
    int remFrames = static_cast<int>(frame - (totalSeconds * fps));
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    wchar_t buf[64];
    swprintf_s(buf, L"%02d:%02d:%02d:%02d", hours, minutes, seconds, remFrames);
    return std::wstring(buf);
}

static std::wstring FormatSessionDuration() {
    if (!s_timerInitialized) {
        return L"00h 00m";
    }
    auto now = std::chrono::steady_clock::now();
    auto elapsedSec = std::chrono::duration_cast<std::chrono::seconds>(now - s_startTime).count();
    int hours = static_cast<int>(elapsedSec / 3600);
    int minutes = static_cast<int>((elapsedSec % 3600) / 60);

    wchar_t buf[64];
    swprintf_s(buf, L"%02dh %02dm", hours, minutes);
    return std::wstring(buf);
}

static std::wstring FormatCurrentTimestamp() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    wchar_t buf[64];
    swprintf_s(buf, L"%04d-%02d-%02d %02d:%02d:%02d",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return std::wstring(buf);
}

HudMetrics CollectMetrics(EDIT_SECTION* edit_section, EDIT_HANDLE* editHandle) {
    HudMetrics metrics;

    metrics.sessionDuration = FormatSessionDuration();
    metrics.captureTimestamp = FormatCurrentTimestamp();

    if (!edit_section || !edit_section->info) {
        metrics.projectName = L"Untitled.au2";
        return metrics;
    }

    const auto* info = edit_section->info;
    metrics.videoWidth = info->width;
    metrics.videoHeight = info->height;

    if (info->scale > 0) {
        metrics.frameRate = static_cast<double>(info->rate) / static_cast<double>(info->scale);
    } else {
        metrics.frameRate = 30.0;
    }

    metrics.currentFrame = info->frame;
    metrics.totalFrames = info->frame_max + 1;
    if (metrics.totalFrames < 1) metrics.totalFrames = 1;

    metrics.timeCode = FormatTimeCode(metrics.currentFrame, metrics.frameRate);

    // 選択オブジェクト数の取得
    if (edit_section->get_selected_object_num) {
        metrics.selectedObjectCount = edit_section->get_selected_object_num();
    } else {
        metrics.selectedObjectCount = 0;
    }

    // 総オブジェクト数の概算 (各レイヤーの全走査)
    int totalCount = 0;
    if (edit_section->find_object) {
        int maxLayer = (info->layer_max < 100) ? info->layer_max : 100;
        for (int layer = 0; layer <= maxLayer; ++layer) {
            int curFrame = 0;
            while (curFrame <= info->frame_max) {
                OBJECT_HANDLE obj = edit_section->find_object(layer, curFrame);
                if (!obj) break;
                totalCount++;
                if (edit_section->get_object_layer_frame) {
                    OBJECT_LAYER_FRAME olf = edit_section->get_object_layer_frame(obj);
                    if (olf.end >= curFrame) {
                        curFrame = olf.end + 1;
                    } else {
                        curFrame++;
                    }
                } else {
                    curFrame++;
                }
            }
        }
    }
    metrics.totalObjectCount = totalCount;

    // プロジェクトファイル名の取得
    std::wstring projName = L"Untitled.au2";
    if (edit_section->get_project_file && editHandle) {
        PROJECT_FILE* pf = edit_section->get_project_file(editHandle);
        if (pf && pf->get_project_file_path) {
            LPCWSTR filePath = pf->get_project_file_path();
            if (filePath && filePath[0] != L'\0') {
                LPCWSTR fileName = PathFindFileNameW(filePath);
                if (fileName) {
                    projName = fileName;
                }
            }
        }
    }
    metrics.projectName = projName;

    return metrics;
}

} // namespace hud
