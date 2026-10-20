# HUD Screenshot Merger (進捗画像キャプチャ)

AviUtl ExEdit2（AviUtl2）の現在のプレビューフレームとプロジェクトの編集情報（HUD）を1枚の画像に合成・出力する汎用プラグイン（`.aux2`）です。  

<!-- スクリーンショットを配置する場合は以下のコメントを外してパスを指定してください -->
<!-- ![出力サンプル](docs/sample.png) -->

---

## 導入方法

### 方法1：パッケージからインストール（推奨）
1. [Releases](../../releases) ページから最新版の `hud_screenshot_merger-vX.X.X.au2pkg.zip` をダウンロードします。
2. ダウンロードした `.au2pkg.zip` ファイルを解凍せず、そのまま **AviUtl2 のプレビュー画面へドラッグ＆ドロップ** してください。

### 方法2：手動で配置する場合
ZIPファイルを解凍し、各ファイルを以下のフォルダ（`%AppData%\aviutl2` または `C:\ProgramData\aviutl2` 配下）へ配置してください。
* `Plugin/hud_screenshot_merger.aux2` → `Plugin` フォルダ
* `Language/English.hud_screenshot_merger.aul2` → `Language` フォルダ

## 使い方
1. メニューバーの **「ファイル」→「エクスポート」→「進捗画像をキャプチャして保存」** を選択します。
2. 保存ダイアログが表示されるので、任意の保存先を指定して保存してください。