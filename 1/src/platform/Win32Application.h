#pragma once

#include <windows.h>

namespace serein {

class Win32Application {
public:
    static constexpr int client_width = 806;
    static constexpr int client_height = 468;

    explicit Win32Application(HINSTANCE instance) noexcept;
    ~Win32Application();

    Win32Application(const Win32Application&) = delete;
    Win32Application& operator=(const Win32Application&) = delete;

    [[nodiscard]] int run(int show_command);

private:
    [[nodiscard]] bool create_window(int show_command);
    [[nodiscard]] static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);
    [[nodiscard]] LRESULT handle_message(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

    HINSTANCE instance_{};
    HWND window_{};
};

}  // namespace serein
