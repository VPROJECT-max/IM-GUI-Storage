#pragma once

#include "app/AppModel.h"

#include <windows.h>

#include <array>
#include <filesystem>
#include <string_view>

namespace serein {

class D3D11Renderer;

[[nodiscard]] constexpr std::array<std::wstring_view, 6> capture_file_names() noexcept {
    return {L"login.png", L"products-loading.png", L"products.png", L"selected.png", L"product-loading.png", L"product-success.png"};
}

class FrameCaptureSequence {
public:
    explicit FrameCaptureSequence(std::filesystem::path directory, bool login_only = false);

    [[nodiscard]] bool enabled() const noexcept;
    void process_after_render(AppModel& model,
                              const D3D11Renderer& renderer,
                              HWND window,
                              float blur_strength);

private:
    enum class Stage {
        login,
        discovery,
        products,
        selected,
        product_loading,
        product_success,
        complete,
    };

    [[nodiscard]] bool capture(const D3D11Renderer& renderer, std::size_t index) const;

    std::filesystem::path directory_;
    bool login_only_{false};
    Stage stage_{Stage::login};
    int settled_frames_{0};
};

}  // namespace serein
