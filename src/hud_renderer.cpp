#include "hud_renderer.hpp"
#include "palette.hpp"

#include <windows.h>
#include <gdiplus.h>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>

#pragma comment(lib, "gdiplus.lib")

namespace hud {

class GdiplusInitScope {
public:
    GdiplusInitScope() {
        Gdiplus::GdiplusStartupInput input;
        Gdiplus::GdiplusStartup(&m_token, &input, nullptr);
    }
    ~GdiplusInitScope() {
        Gdiplus::GdiplusShutdown(m_token);
    }
private:
    ULONG_PTR m_token = 0;
};

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0, size = 0;
    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    std::vector<uint8_t> codecBuffer(size);
    auto* pImageCodecInfo = reinterpret_cast<Gdiplus::ImageCodecInfo*>(codecBuffer.data());
    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j) {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[j].Clsid;
            return j;
        }
    }
    return -1;
}

bool RenderAndSave(const VideoFrameBuffer& frame,
                   const HudMetrics& metrics,
                   const PluginConfig& config,
                   const std::wstring& savePath) {
    if (frame.width <= 0 || frame.height <= 0 || frame.pixels.empty()) {
        return false;
    }

    GdiplusInitScope gdiScope;

    const int hudHeight = 120;
    const int totalWidth = frame.width;
    const int totalHeight = frame.height + hudHeight;

    // 出力用キャンバス Bitmap の作成
    Gdiplus::Bitmap canvas(totalWidth, totalHeight, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&canvas);

    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    // 1. 映像フレームの描画 (上部 Y=0)
    // 透明背景対策: まず純黒で塗りつぶす
    Gdiplus::SolidBrush blackBrush(Gdiplus::Color(255, 0, 0, 0));
    g.FillRectangle(&blackBrush, 0, 0, frame.width, frame.height);

    {
        // frame.pixels は BGRA 不透明 (A=255)
        Gdiplus::Bitmap frameBmp(
            frame.width,
            frame.height,
            frame.pitch,
            PixelFormat32bppARGB,
            const_cast<BYTE*>(frame.pixels.data())
        );
        g.DrawImage(&frameBmp, 0, 0, frame.width, frame.height);
    }

    // 2. HUD パネル背景の描画 (下部 Y=frame.height)
    const Palette& pal = GetPalette(config.paletteIndex);

    Gdiplus::SolidBrush bgBrush(pal.bg);
    g.FillRectangle(&bgBrush, 0, frame.height, totalWidth, hudHeight);

    // パネル境界線 (上端 1px)
    Gdiplus::Pen borderPen(pal.border, 1.0f);
    g.DrawLine(&borderPen, 0, frame.height, totalWidth, frame.height);

    // 3. 3カラムレイアウト計算
    const float padX = 24.0f;
    const float availWidth = static_cast<float>(totalWidth) - padX * 2.0f;
    const float colWidth = availWidth / 3.0f;
    const float innerPadX = 16.0f;

    // カラム区切り縦線 (Y: パネル上部+16px 〜 パネル下部-16px)
    float dividerY1 = static_cast<float>(frame.height) + 16.0f;
    float dividerY2 = static_cast<float>(totalHeight) - 16.0f;

    float div1X = padX + colWidth;
    float div2X = padX + colWidth * 2.0f;
    g.DrawLine(&borderPen, div1X, dividerY1, div1X, dividerY2);
    g.DrawLine(&borderPen, div2X, dividerY1, div2X, dividerY2);

    // フォント定義
    auto fontFamily = std::make_unique<Gdiplus::FontFamily>(L"Segoe UI");
    if (!fontFamily->IsAvailable()) {
        fontFamily = std::make_unique<Gdiplus::FontFamily>(L"Meiryo UI");
    }

    Gdiplus::Font fontLabel(fontFamily.get(), 9.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPoint);
    Gdiplus::Font fontValue(fontFamily.get(), 14.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    Gdiplus::Font fontMuted(fontFamily.get(), 9.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);

    Gdiplus::SolidBrush labelBrush(pal.label);
    Gdiplus::SolidBrush valueBrush(pal.value);
    Gdiplus::SolidBrush mutedBrush(pal.muted);

    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentNear);
    format.SetLineAlignment(Gdiplus::StringAlignmentNear);
    format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);

    // 行Y座標
    float yRowLabel = static_cast<float>(frame.height) + 18.0f;
    float yRowValue = static_cast<float>(frame.height) + 40.0f;
    float yRowMuted = static_cast<float>(frame.height) + 76.0f;

    auto DrawColumn = [&](int colIndex,
                          const std::wstring& label,
                          const std::wstring& value,
                          const std::wstring& muted) {
        float x = padX + static_cast<float>(colIndex) * colWidth;
        float w = colWidth - innerPadX;
        if (colIndex > 0) {
            x += innerPadX;
            w -= innerPadX;
        }

        Gdiplus::RectF rectLabel(x, yRowLabel, w, 20.0f);
        Gdiplus::RectF rectValue(x, yRowValue, w, 32.0f);
        Gdiplus::RectF rectMuted(x, yRowMuted, w, 22.0f);

        g.DrawString(label.c_str(), -1, &fontLabel, rectLabel, &format, &labelBrush);
        g.DrawString(value.c_str(), -1, &fontValue, rectValue, &format, &valueBrush);
        g.DrawString(muted.c_str(), -1, &fontMuted, rectMuted, &format, &mutedBrush);
    };

    // Column 0: PROJECT
    std::wostringstream ossProjectMuted;
    ossProjectMuted << metrics.videoWidth << L" x " << metrics.videoHeight
                    << L" @ " << std::fixed << std::setprecision(2) << metrics.frameRate << L" fps";

    DrawColumn(0, L"PROJECT", metrics.projectName, ossProjectMuted.str());

    // Column 1: TIMELINE
    std::wostringstream ossTimelineValue;
    ossTimelineValue << metrics.timeCode << L" (Frame " << (metrics.currentFrame + 1)
                     << L" / " << metrics.totalFrames << L")";

    std::wostringstream ossTimelineMuted;
    ossTimelineMuted << L"Selected: " << metrics.selectedObjectCount
                     << L" / Total: " << metrics.totalObjectCount << L" Objects";

    DrawColumn(1, L"TIMELINE", ossTimelineValue.str(), ossTimelineMuted.str());

    // Column 2: SESSION (@ユーザー名 を含む)
    std::wstring sessionVal = metrics.sessionDuration + L" (Active Time)";
    std::wstring userCredit = L"@" + config.userName;
    std::wstring sessionMuted = userCredit + L"  •  " + metrics.captureTimestamp;

    DrawColumn(2, L"SESSION", sessionVal, sessionMuted);

    // 4. PNG 保存
    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) < 0) {
        return false;
    }

    Gdiplus::Status saveStatus = canvas.Save(savePath.c_str(), &pngClsid, nullptr);
    return (saveStatus == Gdiplus::Ok);
}

} // namespace hud
