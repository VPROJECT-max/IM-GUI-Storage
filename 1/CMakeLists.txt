cmake_minimum_required(VERSION 3.25)

project(SereinLoader VERSION 0.5.0 LANGUAGES CXX)

option(SEREIN_BUILD_APP "Build the Windows Serein loader" ON)
option(SEREIN_BUILD_TESTS "Build Serein unit tests" ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(SEREIN_BUILD_TESTS)
    enable_testing()

    add_executable(serein_model_tests
        tests/AppModelTests.cpp
        src/app/AppModel.cpp
        src/app/Configuration.cpp
        src/integration/MockServices.cpp
    )
    target_include_directories(serein_model_tests PRIVATE src)
    target_compile_features(serein_model_tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(serein_model_tests PRIVATE /W4 /permissive- /EHsc)
    endif()
    add_test(NAME serein.model COMMAND serein_model_tests)

    add_executable(serein_animation_tests tests/AnimationTests.cpp)
    target_include_directories(serein_animation_tests PRIVATE src)
    target_compile_features(serein_animation_tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(serein_animation_tests PRIVATE /W4 /permissive- /EHsc)
    endif()
    add_test(NAME serein.animation COMMAND serein_animation_tests)

    add_executable(serein_ui_contract_tests tests/UiContractTests.cpp)
    target_include_directories(serein_ui_contract_tests PRIVATE src)
    target_compile_features(serein_ui_contract_tests PRIVATE cxx_std_20)
    if(MSVC)
        target_compile_options(serein_ui_contract_tests PRIVATE /W4 /permissive- /EHsc)
    endif()
    add_test(NAME serein.ui_contract COMMAND serein_ui_contract_tests)

    add_executable(serein_asset_capture_tests
        tests/AssetCaptureTests.cpp
        src/app/Configuration.cpp
    )
    target_include_directories(serein_asset_capture_tests PRIVATE src)
    target_compile_features(serein_asset_capture_tests PRIVATE cxx_std_20)
    target_compile_definitions(serein_asset_capture_tests PRIVATE SEREIN_SOURCE_DIR=L"${CMAKE_CURRENT_SOURCE_DIR}")
    if(MSVC)
        target_compile_options(serein_asset_capture_tests PRIVATE /W4 /permissive- /EHsc)
    endif()
    add_test(NAME serein.asset_capture COMMAND serein_asset_capture_tests)

    if(WIN32)
        add_executable(serein_platform_contract_tests
            tests/PlatformContractTests.cpp
            src/platform/ProcessLauncher.cpp
            src/platform/WindowsCredentialStore.cpp
            src/render/D3D11Renderer.cpp
            src/render/TextureStore.cpp
        )
        target_include_directories(serein_platform_contract_tests PRIVATE src)
        target_compile_features(serein_platform_contract_tests PRIVATE cxx_std_20)
        target_link_libraries(serein_platform_contract_tests PRIVATE crypt32 d3d11 d3dcompiler dxgi ole32 shell32 windowscodecs)
        if(MSVC)
            target_compile_options(serein_platform_contract_tests PRIVATE /W4 /permissive- /EHsc)
        endif()
        add_test(NAME serein.platform_contract COMMAND serein_platform_contract_tests)
    endif()
endif()

if(SEREIN_BUILD_APP)
    if(NOT WIN32)
        message(FATAL_ERROR "The Serein loader application targets Windows only.")
    endif()
    include(cmake/Dependencies.cmake)

    add_executable(Serein WIN32
        src/main.cpp
        src/app/AppModel.cpp
        src/app/Configuration.cpp
        src/integration/MockServices.cpp
        src/platform/Win32Application.cpp
        src/platform/FrameCapture.cpp
        src/platform/ProcessLauncher.cpp
        src/platform/WindowsCredentialStore.cpp
        src/render/D3D11Renderer.cpp
        src/render/TextureStore.cpp
        src/ui/Theme.cpp
        src/ui/Components.cpp
        src/ui/LoaderUI.cpp
        src/ui/screens/AuthScreen.cpp
        src/ui/screens/ProductScreen.cpp
        src/ui/screens/SelectedProductPanel.cpp
        src/ui/screens/SettingsOverlay.cpp
        src/ui/screens/ProductLoadOverlay.cpp
    )
    target_include_directories(Serein PRIVATE src)
    target_compile_features(Serein PRIVATE cxx_std_20)
    target_compile_definitions(Serein PRIVATE
        UNICODE
        _UNICODE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
        IMGUI_DEFINE_MATH_OPERATORS
        SEREIN_VERSION="${PROJECT_VERSION}"
    )
    target_link_libraries(Serein PRIVATE
        serein_imgui
        d3d11
        d3dcompiler
        dxgi
        windowscodecs
        dwmapi
        crypt32
        ole32
        shell32
    )
    if(MSVC)
        target_compile_options(Serein PRIVATE /W4 /permissive- /EHsc /utf-8)
    endif()
    add_custom_command(TARGET Serein POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
            "$<TARGET_FILE_DIR:Serein>/Assets"
        COMMENT "Copying Serein runtime assets"
    )
endif()
