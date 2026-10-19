#pragma once

#include <string>
#include "capture.hpp"
#include "config.hpp"
#include "metrics.hpp"

namespace hud {

// 映像フレームと HUD パネルを結合し、指定パスへ PNG として保存する
bool RenderAndSave(const VideoFrameBuffer& frame,
                   const HudMetrics& metrics,
                   const PluginConfig& config,
                   const std::wstring& savePath);

} // namespace hud
