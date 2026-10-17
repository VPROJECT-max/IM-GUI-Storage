#pragma once

#include <algorithm>
#include <array>

namespace serein::ui {

inline constexpr float panel_slide_seconds = 0.82F;
inline constexpr float auth_welcome_font_size = 31.5F;
inline constexpr float auth_brand_font_size = 26.0F;
inline constexpr float auth_brand_y = 48.5F;
inline constexpr float button_text_size = 18.5F;
inline constexpr float discovery_loading_font_size = 21.0F;
inline constexpr float button_corner_radius = 8.0F;
inline constexpr int button_shadow_alpha = 46;
inline constexpr float button_focus_alpha = 0.38F;

enum class KeyboardIntent {
    none,
    submit,
    cancel,
};

struct PanelVerticalLayout {
    float description_y;
    float settings_y;
    float settings_height;
    float load_y;
    float load_height;
};

struct ProductCardRect {
    float x;
    float y;
    float width;
    float height;
};

[[nodiscard]] constexpr bool password_reveal_requested(const bool eye_hovered,
                                                       const bool primary_button_down) noexcept {
    return eye_hovered && primary_button_down;
}

[[nodiscard]] constexpr KeyboardIntent keyboard_intent(const bool enter_pressed,
                                                       const bool escape_pressed) noexcept {
    if (escape_pressed) {
        return KeyboardIntent::cancel;
    }
    return enter_pressed ? KeyboardIntent::submit : KeyboardIntent::none;
}

[[nodiscard]] constexpr float product_grid_width(const float available_width,
                                                 const float panel_width,
                                                 const float panel_progress) noexcept {
    constexpr float inspector_gutter = 12.0F;
    constexpr float minimum_grid_width = 240.0F;
    const float progress = std::clamp(panel_progress, 0.0F, 1.0F);
    return std::max(minimum_grid_width, available_width - (panel_width + inspector_gutter) * progress);
}

[[nodiscard]] constexpr std::array<ProductCardRect, 4> product_card_layout(const float client_width,
                                                                           const float client_height) noexcept {
    constexpr float horizontal_inset = 18.0F;
    constexpr float vertical_inset = 12.0F;
    constexpr float column_gap = 20.0F;
    constexpr float row_gap = 16.0F;
    const float card_width = std::max(0.0F, (client_width - horizontal_inset * 2.0F - column_gap) * 0.5F);
    const float card_height = std::max(0.0F, (client_height - vertical_inset * 2.0F - row_gap) * 0.5F);
    return {{{horizontal_inset, vertical_inset, card_width, card_height},
             {horizontal_inset + card_width + column_gap, vertical_inset, card_width, card_height},
             {horizontal_inset, vertical_inset + card_height + row_gap, card_width, card_height},
             {horizontal_inset + card_width + column_gap, vertical_inset + card_height + row_gap, card_width, card_height}}};
}

[[nodiscard]] constexpr PanelVerticalLayout selected_panel_vertical_layout(const float panel_height) noexcept {
    constexpr float bottom_inset = 8.0F;
    constexpr float load_height = 36.0F;
    constexpr float action_gap = 8.0F;
    constexpr float settings_height = 32.0F;
    constexpr float description_gap = 20.0F;
    const float load_y = panel_height - bottom_inset - load_height;
    const float settings_y = load_y - action_gap - settings_height;
    return {settings_y - description_gap, settings_y, settings_height, load_y, load_height};
}

[[nodiscard]] constexpr float background_dim_alpha(const bool deep_background) noexcept {
    return deep_background ? 226.0F : 192.0F;
}

[[nodiscard]] constexpr float selection_border_width(const bool selected,
                                                     const bool strong_selection) noexcept {
    if (!selected) {
        return 1.0F;
    }
    return strong_selection ? 2.4F : 1.6F;
}

[[nodiscard]] constexpr float remember_control_width(const float checkbox_width,
                                                     const float label_gap,
                                                     const float label_width) noexcept {
    return std::max(checkbox_width, 0.0F) + std::max(label_gap, 0.0F) + std::max(label_width, 0.0F);
}

}  // namespace serein::ui
