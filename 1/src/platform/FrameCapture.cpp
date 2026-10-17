#include "platform/FrameCapture.h"

#include "render/D3D11Renderer.h"

#include <windows.h>

#include <utility>

namespace serein {

FrameCaptureSequence::FrameCaptureSequence(std::filesystem::path directory, const bool login_only)
    : directory_(std::move(directory)), login_only_(login_only) {
    if (!directory_.empty()) {
        std::error_code error;
        std::filesystem::create_directories(directory_, error);
        if (error) {
            directory_.clear();
        }
    }
}

bool FrameCaptureSequence::enabled() const noexcept {
    return !directory_.empty() && stage_ != Stage::complete;
}

void FrameCaptureSequence::process_after_render(AppModel& model,
                                                const D3D11Renderer& renderer,
                                                HWND window,
                                                const float blur_strength) {
    if (!enabled()) {
        return;
    }
    ++settled_frames_;
    if (login_only_) {
        if (stage_ == Stage::login && settled_frames_ >= 30 && capture(renderer, 0U)) {
            stage_ = Stage::complete;
            PostMessageW(window, WM_CLOSE, 0, 0);
        }
        return;
    }
    switch (stage_) {
    case Stage::login:
        if (settled_frames_ >= 30 && capture(renderer, 0U)) {
            model.set_credentials("review.user", "serein-demo");
            static_cast<void>(model.submit_login());
            stage_ = Stage::discovery;
            settled_frames_ = 0;
        }
        break;
    case Stage::discovery:
        if (settled_frames_ >= 20 && capture(renderer, 1U)) {
            stage_ = Stage::products;
            settled_frames_ = 0;
        }
        break;
    case Stage::products:
        if (model.screen() == Screen::product_selection && blur_strength <= 0.001F && capture(renderer, 2U)) {
            static_cast<void>(model.select_product(0U));
            stage_ = Stage::selected;
            settled_frames_ = 0;
        }
        break;
    case Stage::selected:
        if (settled_frames_ >= 70 && capture(renderer, 3U)) {
            static_cast<void>(model.begin_product_load());
            stage_ = Stage::product_loading;
            settled_frames_ = 0;
        }
        break;
    case Stage::product_loading:
        if (settled_frames_ >= 30 && capture(renderer, 4U)) {
            stage_ = Stage::product_success;
            settled_frames_ = 0;
        }
        break;
    case Stage::product_success:
        if (model.load_state() == OperationState::succeeded && capture(renderer, 5U)) {
            stage_ = Stage::complete;
        }
        break;
    case Stage::complete:
        break;
    }
}

bool FrameCaptureSequence::capture(const D3D11Renderer& renderer, const std::size_t index) const {
    const auto names = capture_file_names();
    return index < names.size() && renderer.capture_png(directory_ / names[index]);
}

}  // namespace serein
