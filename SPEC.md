# HUD Screenshot Merger (進捗画像キャプチャ) 仕様書 (SPEC.md)

- **プロジェクト名:** HUD Screenshot Merger (進捗画像キャプチャ)
- **プロジェクトID:** `hud-screenshot-merger`
- **モジュール名:** `hud_screenshot_merger.aux2`
- **対象バージョン:** AviUtl2 2.1.3a 以降
- **更新日:** 2026-10-19

---

## 1. Overview (プロジェクト概要)

### 1.1 目的
AviUtl2 (ExEdit2) のタイムライン上でワンクリック操作を行うだけで、現在プレビュー表示されている動画フレームを **AviUtl2 公式 SDK のレンダリングエンジン (`rendering_scene_video`) から未圧縮 RGBA ピクセルとして直接取得** し、その下部に Figma / Notion を彷彿とさせるモダン・ミニマリストな「HUD（情報表示パネル）」を美しく合成した PNG 画像を出力する汎用プラグイン (`.aux2`)。

動画制作の進捗状況（作業時間、タイムライン位置、選択オブジェクト数、プロジェクト解像度等）を、作品の美麗なワンシーンとともに記録・SNS（X/Discord等）に共有できるようにする。

### 1.2 コアバリュー
1. **ピュアレンダリング（SDKネイティブ取得）**:
   - OS依存の画面キャプチャ（`PrintWindow`）を使用せず、AviUtl2 内部のレンダラが出力した本来の映像解像度（例: 1920x1080）の RGBA ピクセル配列を直接取得。
   - ウィンドウの重なり、DPIスケーリング、DirectX/DWMの描画欠落、操作ハンドルやUI枠の映り込みが原理的に皆無。
2. **ワンクリック実行**: オブジェクト右クリックメニューから即座にレンダリング＆保存ダイアログを起動。
3. **洗練されたタイポグラフィと引き算のデザイン**: 有彩色を排除し、白〜濃灰（Zinc系）のコントラストと余白（Whitespace）で情報階層を構成。
4. **設定駆動な開発サイクル**: `aviutl2-cli` (`au2`) と CMake による一貫したビルド・テスト・パッケージングの実現。

---

## 2. Architecture & Tech Stack (システムアーキテクチャ・技術スタック)

### 2.1 技術スタック
| 項目 | 選定技術 | 補足 / 理由 |
| :--- | :--- | :--- |
| **開発言語** | C++20 | MSVC 2022 (`/std:c++20`), モダン記法・型安全性 |
| **文字コード** | UTF-8 (`/utf-8`) / UTF-16 (`std::wstring`) | Win32 Unicode API (`W` 関数) と UTF-8 ソースの統一 |
| **ビルドシステム** | CMake (3.24 以上) | クロス環境・依存管理・IDE 連携 |
| **開発・配置 CLI** | `aviutl2-cli` (`au2`) | シンボリックリンク配置、プロファイル管理、パッケージング |
| **プラグイン SDK** | `aviutl2_sdk_mirror` | `external/aviutl2_sdk_mirror` (submodule) または FetchContent |
| **映像フレーム取得** | `HOST_APP_TABLE::rendering_scene_video` | AviUtl2 内部レンダラから直接 `PIXEL_RGBA` バッファを取得 |
| **画像処理・描画** | GDI+ (`Gdiplus.h`, `gdiplus.lib`) | 高品質アンチエイリアステキスト描画、PNG エンコード |
| **GUI・システム API** | Win32, Comdlg32 / COM (`IFileSaveDialog`) | メインウィンドウ特定 (`get_host_app_window`)、保存ダイアログ |

### 2.2 システムアーキテクチャ図
```
[ AviUtl2 Timeline Object ]
           │ (Right-click Menu Trigger)
           ▼
[ main.cpp (RegisterPlugin / Callback) ]
           │
           ├──► [ metrics.cpp ] ──────────────► プロジェクト・タイムライン・セッション作業時間収集
           │
           ├──► [ dialog.cpp ] ───────────────► 保存ダイアログ表示 (親HWND: host->get_host_app_window())
           │
           ├──► [ capture.cpp ] ──────────────► host->rendering_scene_video(currentFrame)
           │                                            │ (RGBA生ピクセルバッファ取得 / 同期待機)
           └───────────────────────────────────────────►▼
                                               [ hud_renderer.cpp ]
                                                        │
                                                        ├─► 映像フレームと HUD 領域の結合
                                                        ├─► GDI+ によるモダン HUD 描画
                                                        └─► PNG ファイルへ書き出し
```

---

## 3. UI/UX Specifications (UIデザイン仕様)

### 3.1 デザイン思想
- **引き算のデザイン (Subtractive Design)**: 派手なアクセントカラー（青・緑・赤など）を一切排し、Zinc 系グレースケールの階層のみで構成。
- **Notion / Figma ライクな情報設計**: 3カラムの均等グリッド、見出しラベル（小さく淡いグレー）と数値データ（大きく明るい白灰）のコントラスト。
- **厳密なアライメントと余白**: 24px の外周パディング、1px の極薄セクション境界線、ラベルと値の縦軸揃え。

### 3.2 カラーパレット (ARGB 値)
| 要素名 | 色名称 / 役割 | ARGB (Hex) | RGB (10進) |
| :--- | :--- | :--- | :--- |
| **HUD Background** | Zinc 900 (パネル背景) | `0xFF18181B` | `24, 24, 27` |
| **HUD Border / Divider** | Zinc 800 (外枠線・1px仕切り線) | `0xFF27272A` | `39, 39, 42` |
| **Label Text (Key)** | Zinc 400 (小見出し・項目名) | `0xFFA1A1AA` | `161, 161, 170` |
| **Value Text (Value)** | Zinc 100 (データ・数値表示) | `0xFFF4F4F5` | `244, 244, 245` |
| **Muted Text / Unit** | Zinc 500 (単位・サブ表記) | `0xFF71717A` | `113, 113, 122` |

### 3.3 タイポグラフィとサイズ
- **フォントファミリ**: `Segoe UI`（フォールバック: `Meiryo UI`, `sans-serif`）
- **文字品質**: `TextRenderingHintClearTypeGridFit` (GDI+)
- **フォント階層**:
  1. **Section Header (ラベル)**: `9pt` (Bold), 大文字統一 (`PROJECT`, `TIMELINE`, `SESSION`), 色: `Zinc 400`
  2. **Primary Value (主数値・項目)**: `15pt` (SemiBold / Regular), 色: `Zinc 100`
  3. **Secondary Value (補足・解像度等)**: `10pt` (Regular), 色: `Zinc 500`

### 3.4 HUD パネルのレイアウト計算ルール
- **高さ固定**: HUD パネル全体の高さは `120px`。
- **幅追従**: 動画のプロジェクト解像度（`VideoWidth`）と厳密に一致させる（例: 幅 1920px）。
- **全体キャンバスサイズ**: `Width = VideoWidth`, `Height = VideoHeight + 120px`（例: 1920 x 1200）。
- **パディング**: 上下左右 `24px`。
- **セクション分割**: 横方向に 3 等分（`ColWidth = (VideoWidth - 24 * 2) / 3`）。
- **区切り線**: カラム 1 と 2 の間、カラム 2 と 3 の間に `1px` の縦線 (`Color: 0xFF27272A`) を描画（上下パディング `16px` を設ける）。

#### 3 カラムの配置内容
```
+---------------------------------------------------------------------------------------------------------+
| [Rendered Frame (1920 x 1080 Native Video Buffer)]                                                     |
|                                                                                                         |
+---------------------------------------------------------------------------------------------------------+
| (HUD Panel Height: 120px)                                                                               |
|  [PROJECT]                           |  [TIMELINE]                          |  [SESSION]                        |
|  MyAwesomeVideo.au2                  |  00:03:24:12 (Frame 6,132 / 10,800)  |  02h 45m (Active Time)           |
|  1920 x 1080 @ 60.00 fps             |  Selected: 3 / Total: 42 Objects     |  2026-10-19 16:30:00             |
+---------------------------------------------------------------------------------------------------------+
```

---

## 4. Directory Structure (ディレクトリ構成)

```text
aviutl2-hud/
├── .aviutl2-cli/                  # au2 管理キャッシュ・スキーマ
├── .git/                          # Git リポジトリ
├── .gitignore                     # Git 除外設定
├── CMakeLists.txt                 # ビルド定義 (C++20, GDI+, SDK)
├── SPEC.md                        # 本仕様書
├── aviutl2.toml                   # au2 プロジェクト・デプロイ定義
├── external/                      # 外部サブモジュール
│   └── aviutl2_sdk_mirror/        # AviUtl2 SDK
├── i18n/                          # 言語リソース
│   └── English.hud_screenshot_merger.aul2
├── package/                       # 配布用パッケージメタデータ
│   ├── package.ini
│   └── package.txt
└── src/                           # ソースコード
    ├── capture.hpp                # SDK による映像フレームレンダリング取得
    ├── capture.cpp
    ├── dialog.hpp                 # 「名前を付けて保存」コモンダイアログ
    ├── dialog.cpp
    ├── hud_renderer.hpp           # GDI+ による HUD 描画 & 画像結合
    ├── hud_renderer.cpp
    ├── metrics.hpp                # 編集情報 (時間、オブジェクト数等) 取得
    ├── metrics.cpp
    └── main.cpp                   # プラグインエントリポイント (RegisterPlugin)
```

---

## 5. Modules & API (主要モジュールとAPI設計)

### 5.1 `main.cpp`
プラグインのライフサイクルとメニュー登録を担当。

- **グローバル保持**:
  - `HOST_APP_TABLE* g_host`: `RegisterPlugin` 時に取得（コールバック内での API 呼び出しに使用）。
  - `std::chrono::steady_clock::time_point g_sessionStartTime`: プラグインロード時の時刻。
- **エクスポート関数**:
  - `RequiredVersion()`: `uint32_t` 返却（`2010301` = 2.1.3a）。
  - `GetCommonPluginTable()`: プラグイン名 `L"HUD Screenshot Merger"`、説明文のポインタ返却。
  - `RegisterPlugin(HOST_APP_TABLE* host)`:
    - `g_host = host;`
    - `metrics::InitializeSessionTimer();`
    - `host->register_object_menu(L"進捗画像をキャプチャして保存...", OnCaptureMenuClicked);`
- **メニューコールバック (`OnCaptureMenuClicked(EDIT_SECTION* edit_section)`)**:
  1. `metrics::Collect(edit_section)` で現在のフレーム番号、プロジェクト名、解像度、fps、オブジェクト数を収集。
  2. `dialog::PromptSavePath(g_host->get_host_app_window(), defaultName, savePath)` で保存先パスを取得（キャンセル時は終了）。
  3. `capture::CaptureFrame(g_host, metrics.currentFrame, frameBuffer)` で純粋な映像フレームを取得。
  4. `hud_renderer::RenderAndSave(frameBuffer, metrics, savePath)` で HUD 描画・画像結合・PNG 保存を実行。

### 5.2 `capture.hpp` / `capture.cpp`
AviUtl2 のレンダリングエンジンから現在フレームのピクセルデータを直接取得。

- **データ構造**:
  ```cpp
  struct VideoFrameBuffer {
      std::vector<uint8_t> pixels; // PIXEL_RGBA (4 bytes per pixel)
      int width = 0;
      int height = 0;
      int pitch = 0;
  };
  ```
- **関数**: `bool CaptureFrame(HOST_APP_TABLE* host, int frame, VideoFrameBuffer& outBuffer)`
- **処理ロジック**:
  1. 同期オブジェクト（`std::promise<bool>` または Win32 Event）を用意。
  2. レンダリング完了コールバック構造体を設定:
     ```cpp
     struct RenderContext {
         VideoFrameBuffer* pBuffer;
         HANDLE hEvent;
         bool success;
     };
     ```
  3. **`host->rendering_scene_video(frame, &context, CallbackProc)`** を呼び出し。
  4. コールバック (`CallbackProc`):
     - 引数で渡された `buffer`（`PIXEL_RGBA` 形式）、`width`、`height`、`pitch` を `outBuffer->pixels` に `memcpy`。
     - `context->success = true;` をセットし、イベントをシグナル化。
  5. 呼び出し元スレッドで `WaitForSingleObject(hEvent, 5000)` により完了を待機。
  6. 取得完了したバッファを返却。

### 5.3 `metrics.hpp` / `metrics.cpp`
HUD に表示する各種情報の取得・管理を担当。

- **構造体**:
  ```cpp
  struct HudMetrics {
      std::wstring projectName;       // プロジェクトファイル名 (未保存時は "Untitled.au2")
      int videoWidth = 0;             // 解像度 横
      int videoHeight = 0;            // 解像度 縦
      double frameRate = 0.0;         // フレームレート
      int currentFrame = 0;           // 現在フレーム
      int totalFrames = 0;            // 総フレーム数
      int selectedObjectCount = 0;    // 選択中のオブジェクト数
      int totalObjectCount = 0;       // 総オブジェクト数
      std::wstring sessionDuration;   // 起動からの経過時間 ("01h 24m")
      std::wstring captureTimestamp;  // 撮影日時 ("YYYY-MM-DD HH:MM:SS")
  };
  ```
- **関数**:
  - `void InitializeSessionTimer()`: プラグインロード時のタイムスタンプを記録。
  - `HudMetrics Collect(EDIT_SECTION* edit_section)`:
    - `edit_section` からプロジェクト名、解像度、フレームレート、現在フレーム、総フレーム数を抽出。
    - 選択中オブジェクトのインデックスをカウント。
    - 経過時間・現在日時文字列を生成。

### 5.4 `hud_renderer.hpp` / `hud_renderer.cpp`
GDI+ を用いた映像フレームと HUD パネルの結合、文字描画、PNG 出力を担当。

- **関数**: `bool RenderAndSave(const VideoFrameBuffer& frame, const HudMetrics& metrics, const std::wstring& saveFilePath)`
- **処理ロジック**:
  1. `Gdiplus::GdiplusStartup` で GDI+ を初期化。
  2. 映像フレーム（RGBA）から `Gdiplus::Bitmap`（`PixelFormat32bppARGB` または `PixelFormat32bppPARGB`）を生成。
  3. 出力用キャンバス `Gdiplus::Bitmap`（幅: `frame.width`, 高さ: `frame.height + 120`）を作成。
  4. `Gdiplus::Graphics` を生成し、高品質描画設定を適用:
     - `SmoothingModeAntiAlias`
     - `InterpolationModeHighQualityBicubic`
     - `TextRenderingHintClearTypeGridFit`
  5. Y=0 に映像フレームを等倍描画。
  6. Y=`frame.height` に HUD 背景（`0xFF18181B`）を塗りつぶし。
  7. 境界線（Y=`frame.height`）および 2 本のカラム区切り線を 1px の `0xFF27272A` で描画。
  8. 各カラムのラベル（Zinc 400）および値（Zinc 100）を正確な X/Y 座標に配置描画。
  9. PNG エンコーダ（CLSID）を取得し、指定パスへファイル保存。

### 5.5 `dialog.hpp` / `dialog.cpp`
Windows コモンファイルダイアログのラッパー。

- **関数**: `bool PromptSavePath(HWND parentWnd, const std::wstring& defaultFilename, std::wstring& outPath)`
- **処理ロジック**:
  - `IFileSaveDialog` (COM) または `GetSaveFileNameW` を使用。
  - 親ウィンドウには `host->get_host_app_window()` の HWND を指定（AviUtl2 本体の手前にモーダル表示）。
  - 初期ファイル名案: `progress_YYYYMMDD_HHMMSS.png`。
  - フィルタ: `PNG Image (*.png)\0*.png\0All Files (*.*)\0*.*\0`。
  - ユーザーが [保存] を選択した場合は `true` とフルパスを返却。キャンセル時は `false`。

---

## 6. Build Pipeline (ビルド・デプロイ手順)

### 6.1 `CMakeLists.txt` 設計
```cmake
cmake_minimum_required(VERSION 3.24)
project(hud_screenshot_merger CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC UTF-8 オプション
if(MSVC)
    add_compile_options(/utf-8)
endif()

# SDK インクルードパス
target_include_directories(hud_screenshot_merger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/aviutl2_sdk_mirror/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# 共有ライブラリ (DLL) 定義
add_library(hud_screenshot_merger SHARED
    src/main.cpp
    src/capture.cpp
    src/dialog.cpp
    src/metrics.cpp
    src/hud_renderer.cpp
)

# リンクライブラリ (GDI+, Win32 標準)
target_link_libraries(hud_screenshot_merger PRIVATE
    gdiplus
    user32
    gdi32
    comdlg32
    ole32
    shlwapi
)
```

### 6.2 `aviutl2.toml` 設定
```toml
#:schema ./.aviutl2-cli/aviutl2.schema.json
[project]
id = "hud-screenshot-merger"
name = "HUD Screenshot Merger"
version = "0.2.0"

[development]
aviutl2_version = "latest"

[preview]
profile = "release"
include = ["hud_screenshot_merger-aux2", "English-hud_screenshot_merger-aul2"]

[release]
profile = "release"
include = ["hud_screenshot_merger-aux2", "English-hud_screenshot_merger-aul2"]
output_dir = "release"
package_template = "package/package.txt"
package_id = "hud-screenshot-merger"
package_name = "HUD Screenshot Merger"
package_information = "{name} v{version}"
uninstall_subfolder_file = true
zip_name = "{name}-v{version}"

[artifacts.English-hud_screenshot_merger-aul2]
destination = "Language/English.hud_screenshot_merger.aul2"
source = "./i18n/English.hud_screenshot_merger.aul2"

[artifacts.hud_screenshot_merger-aux2]
destination = "Plugin/hud_screenshot_merger/hud_screenshot_merger.aux2"

[artifacts.hud_screenshot_merger-aux2.profiles.debug]
build = [
    "cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug",
    "cmake --build build --config Debug --target hud_screenshot_merger"
]
source = "build/Debug/hud_screenshot_merger.dll"

[artifacts.hud_screenshot_merger-aux2.profiles.release]
build = [
    "cmake -S . -B build -DCMAKE_BUILD_TYPE=Release",
    "cmake --build build --config Release --target hud_screenshot_merger"
]
source = "build/Release/hud_screenshot_merger.dll"
```

### 6.3 開発ワークフローコマンド
1. **初期環境構築**:
   ```powershell
   au2 prepare
   ```
2. **日常ビルド・成果物反映（本体は起動しない）**:
   ```powershell
   au2 dev --skip-start
   ```
3. **AviUtl2 を起動して実機テスト**:
   ```powershell
   au2 dev
   ```
4. **プレビュー環境構築**:
   ```powershell
   au2 preview --skip-start
   ```
5. **配布用パッケージ (`.au2pkg.zip`) の生成**:
   ```powershell
   au2 release
   ```

---

## 7. 開発・検証マイルストーン

1. **Phase 1: 環境整備とスケルトンビルド**
   - `aviutl2.toml` および `CMakeLists.txt` の整備。
   - `main.cpp` の最小スケルトン作成と右クリックメニュー表示確認。
2. **Phase 2: フレームレンダリング取得とダイアログの実装**
   - `capture.cpp`: `host->rendering_scene_video` による RGBA バッファ取得と同期待機。
   - `dialog.cpp`: `IFileSaveDialog` による保存先選択（親: `host->get_host_app_window()`）。
3. **Phase 3: GDI+ HUD レンダラーと画像合成**
   - `hud_renderer.cpp`: Zinc カラーパレット、タイポグラフィ、余白計算に基づく描画。
   - RGBA バッファと HUD 領域の結合・PNG 保存処理。
4. **Phase 4: メトリクス収集の連携と仕上げ**
   - `metrics.cpp`: `EDIT_SECTION` からのフレーム数・選択オブジェクト数・セッション時間取得。
   - 実機総合テスト・エラーハンドリング・`au2 release` パッケージ生成。
