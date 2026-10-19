#include "capture.hpp"
#include <cstring>
#include <memory>
#include <atomic>

namespace hud {

struct RenderContext {
    VideoFrameBuffer* pBuffer = nullptr;
    HANDLE hEvent = nullptr;
    std::atomic<bool> success{false};
    std::atomic<bool> completed{false};
    std::atomic<bool> timedOut{false};
};

static void RenderVideoCallback(void* param, int frame, const void* buffer, int width, int height, int pitch) {
    auto* ctx = static_cast<RenderContext*>(param);
    if (!ctx) return;

    // 呼び出し側がタイムアウト済みで破棄フラグが立っている場合は安全に自己破棄
    if (ctx->timedOut.load()) {
        if (ctx->hEvent) {
            CloseHandle(ctx->hEvent);
        }
        delete ctx;
        return;
    }

    // 入力パラメータの厳格なバリデーション
    if (!ctx->pBuffer || !buffer || width <= 0 || height <= 0 || pitch <= 0) {
        ctx->completed.store(true);
        if (ctx->hEvent) {
            SetEvent(ctx->hEvent);
        }
        return;
    }

    // バッファオーバーフロー対策: pitch が横1ライン分のピクセルサイズを満たしているか検証
    if (pitch < width * 4) {
        ctx->completed.store(true);
        if (ctx->hEvent) {
            SetEvent(ctx->hEvent);
        }
        return;
    }

    try {
        size_t totalBytes = static_cast<size_t>(pitch) * height;
        ctx->pBuffer->pixels.resize(totalBytes);
        ctx->pBuffer->width = width;
        ctx->pBuffer->height = height;
        ctx->pBuffer->pitch = pitch;

        // RGBA (AviUtl2 PIXEL_RGBA: R, G, B, A) から BGRA への安全な変換
        const uint8_t* srcRow = static_cast<const uint8_t*>(buffer);
        uint8_t* dstRow = ctx->pBuffer->pixels.data();

        for (int y = 0; y < height; ++y) {
            const uint8_t* src = srcRow + static_cast<size_t>(y) * pitch;
            uint8_t* dst = dstRow + static_cast<size_t>(y) * pitch;
            for (int x = 0; x < width; ++x) {
                uint8_t r = src[x * 4 + 0];
                uint8_t g = src[x * 4 + 1];
                uint8_t b = src[x * 4 + 2];

                dst[x * 4 + 0] = b;   // Blue
                dst[x * 4 + 1] = g;   // Green
                dst[x * 4 + 2] = r;   // Red
                dst[x * 4 + 3] = 255; // 不透明化
            }
        }

        ctx->success.store(true);
    } catch (const std::bad_alloc&) {
        ctx->success.store(false);
    }

    ctx->completed.store(true);
    if (ctx->hEvent) {
        SetEvent(ctx->hEvent);
    }
}

bool CaptureFrame(EDIT_HANDLE* editHandle, int frame, VideoFrameBuffer& outBuffer) {
    if (!editHandle || !editHandle->rendering_scene_video) {
        return false;
    }

    HANDLE hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!hEvent) {
        return false;
    }

    // ワーカースレッドとのライフサイクル競合を防ぐためヒープに割り当て
    auto* ctx = new RenderContext();
    ctx->pBuffer = &outBuffer;
    ctx->hEvent = hEvent;

    bool requestOk = editHandle->rendering_scene_video(frame, ctx, RenderVideoCallback);
    if (!requestOk) {
        CloseHandle(hEvent);
        delete ctx;
        return false;
    }

    // レンダリング完了を待機 (最大30秒)
    DWORD waitResult = WaitForSingleObject(hEvent, 30000);

    bool isSuccess = false;
    if (waitResult == WAIT_OBJECT_0 && ctx->completed.load()) {
        isSuccess = ctx->success.load();
        CloseHandle(hEvent);
        delete ctx;
    } else {
        // タイムアウトまたは待機エラー: コールバック側で後から安全に解放させる
        ctx->timedOut.store(true);
        isSuccess = false;
    }

    return isSuccess;
}

} // namespace hud
