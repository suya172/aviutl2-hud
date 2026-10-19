#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <windows.h>
#include <gdiplus.h>

namespace hud {

struct Palette {
    std::wstring name;
    Gdiplus::Color bg;
    Gdiplus::Color border;
    Gdiplus::Color label;
    Gdiplus::Color value;
    Gdiplus::Color muted;
};

inline const std::vector<Palette>& GetPalettes() {
    static const std::vector<Palette> s_palettes = {
        // 0. ダーク (Zinc Dark - ミニマリストダーク)
        {
            L"ダーク (Dark)",
            Gdiplus::Color(255, 24, 24, 27),        // Bg: #18181B (Zinc 900)
            Gdiplus::Color(255, 39, 39, 42),        // Border: #27272A (Zinc 800)
            Gdiplus::Color(255, 161, 161, 170),    // Label: #A1A1AA (Zinc 400)
            Gdiplus::Color(255, 244, 244, 245),    // Value: #F4F4F5 (Zinc 100)
            Gdiplus::Color(255, 113, 113, 122)     // Muted: #71717A (Zinc 500)
        },
        // 1. ライト (Notion Light - クリーンホワイト)
        {
            L"ライト (Light)",
            Gdiplus::Color(255, 250, 250, 250),    // Bg: #FAFAFA (Zinc 50)
            Gdiplus::Color(255, 228, 228, 231),    // Border: #E4E4E7 (Zinc 200)
            Gdiplus::Color(255, 113, 113, 122),    // Label: #71717A (Zinc 500)
            Gdiplus::Color(255, 24, 24, 27),        // Value: #18181B (Zinc 900)
            Gdiplus::Color(255, 161, 161, 170)     // Muted: #A1A1AA (Zinc 400)
        },
        // 2. 春 (Spring / Sakura - 墨桜 & 淡桜)
        {
            L"春 (Spring / Sakura)",
            Gdiplus::Color(255, 31, 23, 26),        // Bg: #1F171A (スモーキーチャコールローズ)
            Gdiplus::Color(255, 54, 36, 43),        // Border: #36242B
            Gdiplus::Color(255, 244, 114, 182),    // Label: #F472B6 (チェリーブロッサム)
            Gdiplus::Color(255, 255, 241, 242),    // Value: #FFF1F2 (淡桜白)
            Gdiplus::Color(255, 251, 113, 133)     // Muted: #FB7185 (ローズアクセント)
        },
        // 3. 夏 (Summer / Ocean - 深海 & ターコイズ)
        {
            L"夏 (Summer / Ocean)",
            Gdiplus::Color(255, 11, 19, 32),        // Bg: #0B1320 (ディープオーシャンネイビー)
            Gdiplus::Color(255, 30, 41, 59),        // Border: #1E293B (スレート)
            Gdiplus::Color(255, 56, 189, 248),     // Label: #38BDF8 (スカイブルー)
            Gdiplus::Color(255, 240, 249, 255),    // Value: #F0F9FF (アイシーホワイト)
            Gdiplus::Color(255, 125, 211, 252)     // Muted: #7DD3FC (ターコイズミスト)
        },
        // 4. 秋 (Autumn / Amber - 琥珀 & ウォールナット)
        {
            L"秋 (Autumn / Amber)",
            Gdiplus::Color(255, 26, 22, 18),        // Bg: #1A1612 (ダークウォールナット)
            Gdiplus::Color(255, 51, 41, 33),        // Border: #332921 (アンバーブラウン)
            Gdiplus::Color(255, 245, 158, 11),     // Label: #F59E0B (ウォームアンバー)
            Gdiplus::Color(255, 254, 243, 199),    // Value: #FEF3C7 (温白色)
            Gdiplus::Color(255, 217, 119, 6)       // Muted: #D97706 (テラコッタゴールド)
        },
        // 5. 冬 (Winter / Frost - 白銀 & フロストインディゴ)
        {
            L"冬 (Winter / Frost)",
            Gdiplus::Color(255, 12, 16, 23),        // Bg: #0C1017 (凍夜スレート)
            Gdiplus::Color(255, 31, 41, 55),        // Border: #1F2937 (フロストグレー)
            Gdiplus::Color(255, 165, 180, 252),    // Label: #A5B4FC (ペールインディゴ)
            Gdiplus::Color(255, 248, 250, 252),    // Value: #F8FAFC (純雪白)
            Gdiplus::Color(255, 129, 140, 248)     // Muted: #818CF8 (フロストバイオレット)
        }
    };
    return s_palettes;
}

inline const Palette& GetPalette(int index) {
    const auto& list = GetPalettes();
    if (index < 0 || index >= static_cast<int>(list.size())) {
        return list[0];
    }
    return list[index];
}

} // namespace hud
