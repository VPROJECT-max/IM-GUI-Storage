#pragma once

#include "app/AppModel.h"
#include "render/TextureStore.h"

#include <imgui.h>

#include <array>
#include <optional>

namespace serein::ui {

struct SettingsState {
    bool deep_background{true};
    bool reduced_motion{false};
    bool strong_selection{true};
};

class ProductScreen {
public:
    void render(AppModel& model,
                const std::array<const Texture*, 4>& product_textures,
                ImVec2 minimum,
                ImVec2 maximum,
                float delta_seconds,
                float time_seconds,
                float alpha,
                bool soft_focus);

    [[nodiscard]] const SettingsState& settings() const noexcept;

private:
    std::array<float, 4> card_hover_{};
    float panel_progress_{0.0F};
    std::optional<std::size_t> displayed_product_index_;
    std::optional<std::size_t> pending_product_index_;
    bool switching_product_{false};
    float load_overlay_alpha_{0.0F};
    SettingsState settings_{};
};

}  // namespace serein::ui
