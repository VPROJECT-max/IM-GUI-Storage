#include "ui/LoaderUI.h"

#include "ui/Animation.h"
#include "ui/Components.h"
#include "ui/Theme.h"
#include "ui/UiContracts.h"

#include <algorithm>
#include <cmath>

namespace serein::ui {

LoaderUI::LoaderUI(AppModel& model,
                   TextureStore& textures,
                   ID3D11Device* device,
                   std::filesystem::path asset_root)
    : model_(model), textures_(textures), device_(device), asset_root_(std::move(asset_root)) {
    load_assets();
}

LoaderFrameResult LoaderUI::render(HWND window, const float delta_seconds) {
    static_cast<void>(window);
    time_seconds_ += std::max(delta_seconds, 0.0F);
    const bool authentication = model_.screen() == Screen::authentication;
    const bool discovery = model_.screen() == Screen::loading_products;
    auth_alpha_ = animation::advance_toward(auth_alpha_, authentication ? 1.0F : 0.0F, 0.72F, delta_seconds);
    products_alpha_ = animation::advance_toward(products_alpha_, authentication ? 0.0F : 1.0F, 0.84F, delta_seconds);
    blur_alpha_ = animation::advance_toward(blur_alpha_, discovery ? 1.0F : 0.0F, 0.46F, delta_seconds);
    completion_exit_.update(model_.load_state(), delta_seconds);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2{0.0F, 0.0F});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0.0F, 0.0F});
    ImGui::Begin("SereinRoot", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
                     ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilledMultiColor(ImVec2{0.0F, 0.0F}, io.DisplaySize,
                                  IM_COL32(7, 7, 8, 255), IM_COL32(14, 13, 15, 255),
                                  IM_COL32(5, 5, 6, 255), IM_COL32(9, 8, 10, 255));

    const ImVec2 content_min{0.0F, 0.0F};
    const ImVec2 content_max = io.DisplaySize;
    if (auth_alpha_ > 0.01F) {
        auth_screen_.render(model_, promotional_art_, content_min, content_max, time_seconds_, auth_alpha_);
    }
    if (products_alpha_ > 0.01F) {
        const bool product_loading = model_.load_state() == OperationState::working;
        product_screen_.render(model_, product_art_, content_min, content_max, delta_seconds, time_seconds_, products_alpha_, discovery || product_loading);
    }
    ImGui::End();
    ImGui::PopStyleVar();

    if (blur_alpha_ > 0.002F) {
        ImDrawList* overlay = ImGui::GetForegroundDrawList();
        overlay->AddRectFilled(ImVec2{0.0F, 0.0F}, io.DisplaySize,
                               IM_COL32(4, 4, 5, static_cast<int>(84.0F * blur_alpha_)));
        const ImVec2 center{io.DisplaySize.x * 0.5F, io.DisplaySize.y * 0.5F - 7.0F};
        const ImU32 purple = IM_COL32(165, 46, 151, static_cast<int>(255.0F * blur_alpha_));
        overlay->AddCircle(center, 19.0F, IM_COL32(182, 108, 174, static_cast<int>(66.0F * blur_alpha_)), 48, 2.0F);
        constexpr float tau = 6.28318530718F;
        const float start = std::fmod(time_seconds_ * 2.6F, tau);
        overlay->PathClear();
        overlay->PathArcTo(center, 19.0F, start, start + 2.05F, 32);
        overlay->PathStroke(purple, 0, 3.0F);
        constexpr char copy[] = "loading your products.";
        constexpr float copy_size = discovery_loading_font_size;
        ImFont* loading_font = ImGui::GetIO().Fonts->Fonts.Size > 1 ? ImGui::GetIO().Fonts->Fonts[1] : ImGui::GetFont();
        const ImVec2 text_size = loading_font->CalcTextSizeA(copy_size, 1000.0F, 0.0F, copy);
        overlay->AddText(loading_font, copy_size,
                         ImVec2{(io.DisplaySize.x - text_size.x) * 0.5F, center.y + 37.0F},
                         IM_COL32(231, 227, 231, static_cast<int>(240.0F * blur_alpha_)), copy);
    }
    return LoaderFrameResult{blur_alpha_, completion_exit_.opacity(), completion_exit_.close_requested()};
}

void LoaderUI::load_assets() {
    promotional_art_ = textures_.load(device_, asset_root_ / model_.config().assets.promotional);
    for (std::size_t index = 0; index < product_art_.size() && index < model_.config().products.size(); ++index) {
        product_art_[index] = textures_.load(device_, asset_root_ / model_.config().products[index].image_path);
    }
}

}  // namespace serein::ui
