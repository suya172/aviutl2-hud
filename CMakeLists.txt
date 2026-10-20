cmake_minimum_required(VERSION 3.20)
project(hud_screenshot_merger CXX)

# プラグイン本体 (.aux2 / .dll)
add_library(hud_screenshot_merger SHARED
    src/main.cpp
    src/capture.cpp
    src/config.cpp
    src/dialog.cpp
    src/metrics.cpp
    src/hud_renderer.cpp
    src/resource.rc
)

target_compile_features(hud_screenshot_merger PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(hud_screenshot_merger PRIVATE /utf-8)
endif()

target_include_directories(hud_screenshot_merger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/aviutl2_sdk_mirror/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hud_screenshot_merger PRIVATE
    gdiplus
    user32
    gdi32
    comdlg32
    ole32
    shlwapi
    shell32
    dwmapi
)

# UIテスト用スタンドアロン実行ファイル (hud_test.exe)
add_executable(hud_test
    tests/test_renderer.cpp
    src/hud_renderer.cpp
)

target_compile_features(hud_test PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(hud_test PRIVATE /utf-8)
endif()

target_include_directories(hud_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/aviutl2_sdk_mirror/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(hud_test PRIVATE
    gdiplus
    user32
    gdi32
    ole32
    shlwapi
)
