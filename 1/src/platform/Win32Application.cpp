#include "platform/Win32Application.h"

#include "app/AppModel.h"
#include "app/Configuration.h"
#include "integration/MockServices.h"
#include "platform/FrameCapture.h"
#include "platform/WindowsCredentialStore.h"
#include "render/D3D11Renderer.h"
#include "render/TextureStore.h"
#include "ui/LoaderUI.h"
#include "ui/Theme.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <windowsx.h>
#include <shellapi.h>

#include <memory>
#include <filesystem>
#include <optional>
#include <algorithm>
#include <cmath>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

namespace serein {
namespace {

constexpr wchar_t window_class_name[] = L"SereinLoaderWindow";

void apply_rounded_window_region(const HWND window) {
    HRGN region = CreateRoundRectRgn(0, 0, Win32Application::client_width + 1,
                                    Win32Application::client_height + 1, 18, 18);
    if (region != nullptr && SetWindowRgn(window, region, TRUE) == 0) {
        DeleteObject(region);
    }
}

void copy_draw_data_header(ImDrawData& target, const ImDrawData& source) {
    target.Valid = source.Valid;
    target.DisplayPos = source.DisplayPos;
    target.DisplaySize = source.DisplaySize;
    target.FramebufferScale = source.FramebufferScale;
    target.OwnerViewport = source.OwnerViewport;
    target.Textures = source.Textures;
}

void render_blurred_product_frame(const ImDrawData* source,
                                  ImDrawList* foreground,
                                  const float blur_strength,
                                  const D3D11Renderer& renderer) {
    ImDrawData scene;
    copy_draw_data_header(scene, *source);
    for (int index = 0; index < source->CmdListsCount; ++index) {
        ImDrawList* list = source->CmdLists[index];
        if (list != foreground) {
            scene.AddDrawList(list);
        }
    }
    ImGui_ImplDX11_RenderDrawData(&scene);
    renderer.blur_back_buffer(blur_strength);

    ImDrawData overlay;
    copy_draw_data_header(overlay, *source);
    overlay.AddDrawList(foreground);
    ImGui_ImplDX11_RenderDrawData(&overlay);
}

std::optional<std::filesystem::path> capture_directory_from_command_line() {
    int argument_count = 0;
    wchar_t** arguments = CommandLineToArgvW(GetCommandLineW(), &argument_count);
    if (arguments == nullptr) {
        return std::nullopt;
    }
    std::optional<std::filesystem::path> result;
    for (int index = 1; index + 1 < argument_count; ++index) {
        if (std::wstring_view{arguments[index]} == L"--capture-dir") {
            result = std::filesystem::path{arguments[index + 1]};
            break;
        }
    }
    LocalFree(arguments);
    return result;
}

bool capture_login_only_from_command_line() {
    int argument_count = 0;
    wchar_t** arguments = CommandLineToArgvW(GetCommandLineW(), &argument_count);
    if (arguments == nullptr) {
        return false;
    }
    bool result = false;
    for (int index = 1; index < argument_count; ++index) {
        if (std::wstring_view{arguments[index]} == L"--capture-login") {
            result = true;
            break;
        }
    }
    LocalFree(arguments);
    return result;
}

}  // namespace

Win32Application::Win32Application(HINSTANCE instance) noexcept : instance_(instance) {}

Win32Application::~Win32Application() = default;

int Win32Application::run(const int show_command) {
    if (!create_window(show_command)) {
        return 1;
    }

    D3D11Renderer renderer;
    if (!renderer.initialize(window_)) {
        DestroyWindow(window_);
        return 2;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    const char* regular_font = "C:/Windows/Fonts/segoeui.ttf";
    const char* semibold_font = "C:/Windows/Fonts/seguisb.ttf";
    if (io.Fonts->AddFontFromFileTTF(regular_font, 15.0F) == nullptr) {
        io.Fonts->AddFontDefault();
    }
    if (io.Fonts->AddFontFromFileTTF(semibold_font, 18.0F) == nullptr) {
        io.Fonts->AddFontDefault();
    }
    if (io.Fonts->AddFontFromFileTTF(semibold_font, 30.0F) == nullptr) {
        io.Fonts->AddFontDefault();
    }
    if (io.Fonts->AddFontFromFileTTF(regular_font, 22.0F) == nullptr) {
        io.Fonts->AddFontDefault();
    }
    ui::Theme::apply();
    if (!ImGui_ImplWin32_Init(window_) || !ImGui_ImplDX11_Init(renderer.device(), renderer.context())) {
        ImGui::DestroyContext();
        renderer.shutdown();
        DestroyWindow(window_);
        return 3;
    }

    auto authenticator = std::make_shared<MockAuthenticator>();
    auto product_loader = std::make_shared<MockProductLoader>();
    const std::optional<std::filesystem::path> capture_directory = capture_directory_from_command_line();
    std::shared_ptr<ICredentialStore> credential_store;
    if (!capture_directory.has_value()) {
        credential_store = std::make_shared<WindowsCredentialStore>();
    }
    AppModel model{make_default_config(), authenticator, product_loader, credential_store};
    TextureStore textures;
    wchar_t executable_path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, executable_path, MAX_PATH);
    const std::filesystem::path asset_root = std::filesystem::path{executable_path}.parent_path();
    ui::LoaderUI loader_ui{model, textures, renderer.device(), asset_root};
    FrameCaptureSequence capture_sequence{capture_directory.value_or(std::filesystem::path{}),
                                           capture_login_only_from_command_line()};

    MSG message{};
    bool running = true;
    while (running) {
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != FALSE) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
            if (message.message == WM_QUIT) {
                running = false;
            }
        }
        if (!running) {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        model.update(io.DeltaTime);
        const ui::LoaderFrameResult frame = loader_ui.render(window_, io.DeltaTime);

        ImGui::Render();
        constexpr float clear_color[]{0.024F, 0.022F, 0.025F, 1.0F};
        renderer.begin_frame(clear_color);
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (frame.blur_strength > 0.001F) {
            render_blurred_product_frame(draw_data, ImGui::GetForegroundDrawList(), frame.blur_strength, renderer);
        } else {
            ImGui_ImplDX11_RenderDrawData(draw_data);
        }
        capture_sequence.process_after_render(model, renderer, window_, frame.blur_strength);
        renderer.present();
        const BYTE window_alpha = static_cast<BYTE>(std::lround(std::clamp(frame.window_opacity, 0.0F, 1.0F) * 255.0F));
        SetLayeredWindowAttributes(window_, 0, window_alpha, LWA_ALPHA);
        if (frame.close_requested) {
            PostMessageW(window_, WM_CLOSE, 0, 0);
        }
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    textures.clear();
    ImGui::DestroyContext();
    renderer.shutdown();
    if (window_ != nullptr) {
        DestroyWindow(window_);
        window_ = nullptr;
    }
    UnregisterClassW(window_class_name, instance_);
    return 0;
}

bool Win32Application::create_window(const int show_command) {
    WNDCLASSEXW window_class{};
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_CLASSDC;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance_;
    window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    window_class.lpszClassName = window_class_name;
    if (RegisterClassExW(&window_class) == 0) {
        return false;
    }

    const DWORD style = WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX;
    RECT window_rect{0, 0, client_width, client_height};
    const UINT dpi = GetDpiForSystem();
    AdjustWindowRectExForDpi(&window_rect, style, FALSE, 0, dpi);
    const int width = window_rect.right - window_rect.left;
    const int height = window_rect.bottom - window_rect.top;

    POINT origin{};
    const HMONITOR monitor = MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitor_info{sizeof(monitor_info)};
    GetMonitorInfoW(monitor, &monitor_info);
    const int x = monitor_info.rcWork.left + ((monitor_info.rcWork.right - monitor_info.rcWork.left) - width) / 2;
    const int y = monitor_info.rcWork.top + ((monitor_info.rcWork.bottom - monitor_info.rcWork.top) - height) / 2;

    window_ = CreateWindowExW(WS_EX_APPWINDOW | WS_EX_LAYERED,
                              window_class_name,
                              L"Serein",
                              style,
                              x,
                              y,
                              width,
                              height,
                              nullptr,
                              nullptr,
                              instance_,
                              this);
    if (window_ == nullptr) {
        UnregisterClassW(window_class_name, instance_);
        return false;
    }
    apply_rounded_window_region(window_);
    SetLayeredWindowAttributes(window_, 0, 255, LWA_ALPHA);
    ShowWindow(window_, show_command);
    UpdateWindow(window_);
    return true;
}

LRESULT CALLBACK Win32Application::window_proc(HWND window, const UINT message, const WPARAM w_param, const LPARAM l_param) {
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
        return TRUE;
    }
    Win32Application* application = nullptr;
    if (message == WM_NCCREATE) {
        const auto* create = reinterpret_cast<CREATESTRUCTW*>(l_param);
        application = static_cast<Win32Application*>(create->lpCreateParams);
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(application));
    } else {
        application = reinterpret_cast<Win32Application*>(GetWindowLongPtrW(window, GWLP_USERDATA));
    }
    if (application != nullptr) {
        return application->handle_message(window, message, w_param, l_param);
    }
    return DefWindowProcW(window, message, w_param, l_param);
}

LRESULT Win32Application::handle_message(HWND window, const UINT message, const WPARAM w_param, const LPARAM l_param) {
    switch (message) {
    case WM_NCHITTEST:
        return HTCLIENT;
    case WM_DPICHANGED: {
        const auto* suggested = reinterpret_cast<RECT*>(l_param);
        SetWindowPos(window, nullptr, suggested->left, suggested->top, client_width, client_height, SWP_NOZORDER | SWP_NOACTIVATE);
        apply_rounded_window_region(window);
        return 0;
    }
    case WM_SYSCOMMAND:
        if ((w_param & 0xFFF0U) == SC_MAXIMIZE || (w_param & 0xFFF0U) == SC_SIZE) {
            return 0;
        }
        break;
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        break;
    }
    return DefWindowProcW(window, message, w_param, l_param);
}

}  // namespace serein
