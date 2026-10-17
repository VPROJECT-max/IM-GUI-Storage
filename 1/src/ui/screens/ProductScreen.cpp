#include "ui/screens/ProductScreen.h"

#include "ui/Animation.h"
#include "ui/Components.h"
#include "ui/Theme.h"
#include "ui/UiContracts.h"
#include "ui/screens/ProductLoadOverlay.h"
#include "ui/screens/SelectedProductPanel.h"
#include "ui/screens/SettingsOverlay.h"

#include <algorithm>

namespace serein::ui {
namespace {

constexpr ImVec4 accent{0.63F, 0.13F, 0.59F, 1.0F};
constexpr ImVec4 text_primary{0.91F, 0.90F, 0.91F, 1.0F};
constexpr ImVec4 text_secondary{0.54F, 0.52F, 0.55F, 1.0F};

ImFont* regular_font() {
    const auto& fonts = ImGui::GetIO().Fonts->Fonts;
    return fonts.Size > 0 ? fonts[0] : ImGui::GetFont();
}

ImFont* semibold_font() {
    const auto& fonts = ImGui::GetIO().Fonts->Fonts;
    return fonts.Size > 1 ? fonts[1] : ImGui::GetFont();
}

void draw_play_button(ImDrawList* draw, const ImVec2 minimum, const float hover, const float alpha) {
    const ImVec2 size{42.0F, 42.0F};
    const ImVec4 upper = hover > 0.15F ? ImVec4{0.62F, 0.17F, 0.58F, 1.0F} : ImVec4{0.53F, 0.12F, 0.50F, 1.0F};
    const ImVec4 lower{0.31F, 0.065F, 0.30F, 1.0F};
    draw->AddRectFilled(minimum + ImVec2{0.0F, 3.0F}, minimum + size + ImVec2{0.0F, 5.0F},
                        IM_COL32(41, 5, 40, static_cast<int>(78.0F * alpha)), 3.0F);
    draw->AddRectFilled(minimum, minimum + size, color_u32(ImVec4{0.19F, 0.15F, 0.19F, 1.0F}, alpha), 6.0F);
    draw->AddRectFilledMultiColor(minimum + ImVec2{2.0F, 2.0F}, minimum + size - ImVec2{2.0F, 2.0F},
                                  color_u32(upper, alpha), color_u32(ImVec4{0.45F, 0.095F, 0.42F, 1.0F}, alpha),
                                  color_u32(ImVec4{0.20F, 0.16F, 0.20F, 1.0F}, alpha), color_u32(lower, alpha));
    draw_icon(draw, IconGlyph::product, minimum + ImVec2{11.0F, 11.0F}, 20.0F,
              color_u32(ImVec4{0.08F, 0.025F, 0.075F, 1.0F}, alpha), 1.5F);
}

void draw_card(ImDrawList* draw,
               const Product& product,
               const Texture* texture,
               const ProductCardRect& bounds,
               const float hover,
               const bool selected,
               const float alpha) {
    const ImVec2 minimum{bounds.x, bounds.y - hover * 1.0F};
    const ImVec2 maximum{bounds.x + bounds.width, bounds.y + bounds.height - hover * 1.0F};
    constexpr float radius = 5.0F;
    draw->AddRectFilled(minimum + ImVec2{0.0F, 4.0F}, maximum + ImVec2{0.0F, 6.0F},
                        IM_COL32(0, 0, 0, static_cast<int>(46.0F * alpha)), radius);
    draw->AddRectFilled(minimum, maximum, IM_COL32(9, 9, 10, static_cast<int>(255.0F * alpha)), radius);

    const float info_height = 56.0F;
    const float image_bottom = maximum.y - info_height;
    draw_image_cover(draw, texture, minimum, maximum, radius,
                     color_u32(ImVec4{1.0F, 1.0F, 1.0F, 1.0F}, alpha));
    if (hover > 0.01F) {
        draw->AddRectFilled(minimum, ImVec2{maximum.x, image_bottom},
                            IM_COL32(255, 255, 255, static_cast<int>(10.0F * hover * alpha)), radius);
    }
    draw->PushClipRect(ImVec2{minimum.x, image_bottom}, maximum, true);
    draw_blurred_image_cover(draw, texture, minimum, maximum, radius,
                             IM_COL32(255, 255, 255, static_cast<int>(235.0F * alpha)));
    draw->PopClipRect();
    draw->AddRectFilledMultiColor(ImVec2{minimum.x, image_bottom - 13.0F}, maximum,
                                  IM_COL32(10, 5, 11, 18), IM_COL32(4, 4, 5, 26),
                                  IM_COL32(37, 7, 35, static_cast<int>(202.0F * alpha)),
                                  IM_COL32(5, 5, 6, static_cast<int>(211.0F * alpha)));
    draw->AddLine(ImVec2{minimum.x + 1.0F, image_bottom}, ImVec2{maximum.x - 1.0F, image_bottom},
                  IM_COL32(192, 65, 179, static_cast<int>(25.0F * alpha)), 1.0F);

    const float info_x = minimum.x + 13.0F;
    const float label_y = image_bottom + 8.0F;
    draw_icon(draw, IconGlyph::calendar, ImVec2{info_x - 1.0F, label_y}, 13.0F, color_u32(accent, alpha), 1.6F);
    draw->AddText(semibold_font(), 14.5F, ImVec2{info_x + 17.0F, label_y}, color_u32(text_primary, alpha), "Last update");
    draw->AddText(regular_font(), 12.5F, ImVec2{info_x, label_y + 22.0F}, color_u32(text_secondary, alpha), product.last_update.empty() ? "05-05-2027" : product.last_update.c_str());

    const ImVec2 button_min{maximum.x - 50.0F, image_bottom + 10.0F};
    draw->AddRectFilled(button_min + ImVec2{0.0F, 3.0F}, button_min + ImVec2{42.0F, 45.0F},
                        IM_COL32(48, 8, 45, static_cast<int>(70.0F * alpha)), 3.0F);
    if (hover > 0.15F) {
        draw->AddRectFilled(button_min - ImVec2{1.0F, 1.0F}, button_min + ImVec2{43.0F, 43.0F},
                            IM_COL32(173, 49, 160, static_cast<int>(22.0F * hover * alpha)), 4.0F);
    }
    draw_play_button(draw, button_min, hover, alpha);
    const ImU32 border = selected ? color_u32(accent, alpha) :
                         color_u32(hover > 0.2F ? ImVec4{0.30F, 0.28F, 0.31F, 1.0F} : ImVec4{0.20F, 0.19F, 0.21F, 1.0F}, alpha);
    draw->AddRect(minimum, maximum, border, radius, 0, selected ? 2.0F : 1.0F);
}

}  // namespace

void ProductScreen::render(AppModel& model,
                           const std::array<const Texture*, 4>& product_textures,
                           const ImVec2 minimum,
                           const ImVec2 maximum,
                           const float delta_seconds,
                           const float time_seconds,
                           const float alpha,
                           const bool soft_focus) {
    const float content_alpha = std::clamp(alpha, 0.0F, 1.0F);
    const auto layout = product_card_layout(maximum.x - minimum.x, maximum.y - minimum.y);
    const auto selected_index = model.selected_product_index();
    if (selected_index.has_value() && !displayed_product_index_.has_value()) {
        displayed_product_index_ = selected_index;
    } else if (selected_index.has_value() && displayed_product_index_.has_value() &&
               *selected_index != *displayed_product_index_ && !switching_product_) {
        pending_product_index_ = selected_index;
        switching_product_ = true;
    }
    if (!selected_index.has_value()) {
        pending_product_index_.reset();
        switching_product_ = false;
    }

    float panel_target = selected_index.has_value() ? 1.0F : 0.0F;
    float panel_duration = panel_slide_seconds;
    if (switching_product_) {
        panel_target = pending_product_index_.has_value() ? 0.0F : 1.0F;
        panel_duration = panel_slide_seconds * 0.5F;
    }
    panel_progress_ = animation::advance_toward(panel_progress_, panel_target, panel_duration, delta_seconds);
    if (switching_product_ && pending_product_index_.has_value() && panel_progress_ <= 0.0F) {
        displayed_product_index_ = pending_product_index_;
        pending_product_index_.reset();
    } else if (switching_product_ && !pending_product_index_.has_value() && panel_progress_ >= 1.0F) {
        switching_product_ = false;
    }
    if (!selected_index.has_value() && panel_progress_ <= 0.0F) {
        displayed_product_index_.reset();
    }
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float client_width = maximum.x - minimum.x;
    const float client_height = maximum.y - minimum.y;
    const float panel_progress = animation::ease_in_out_cubic(panel_progress_);
    const float list_x = 16.0F;
    const float list_width = client_width * 0.5F - 26.0F;
    const float list_gap = 16.0F;
    const float list_height = (client_height - 32.0F - list_gap) * 0.5F;
    const std::size_t active_column = displayed_product_index_.has_value() ? *displayed_product_index_ % 2U : 0U;

    for (std::size_t index = 0; index < model.config().products.size() && index < product_textures.size(); ++index) {
        ProductCardRect target{layout[index].x, layout[index].y, layout[index].width, layout[index].height};
        float card_alpha = content_alpha;
        if (displayed_product_index_.has_value() && panel_progress_ > 0.0F) {
            if (index % 2U == active_column) {
                const float row = static_cast<float>(index / 2U);
                target = {list_x,
                          16.0F + row * (list_height + list_gap),
                          list_width,
                          list_height};
            } else {
                card_alpha *= 1.0F - panel_progress;
            }
            const float from_x = layout[index].x;
            const float from_y = layout[index].y;
            target.x = from_x + (target.x - from_x) * panel_progress;
            target.y = from_y + (target.y - from_y) * panel_progress;
            target.width = layout[index].width + (target.width - layout[index].width) * panel_progress;
            target.height = layout[index].height + (target.height - layout[index].height) * panel_progress;
        }
        const ProductCardRect card{minimum.x + target.x,
                                   minimum.y + target.y,
                                   target.width,
                                   target.height};
        const ImVec2 card_min{card.x, card.y};
        const ImVec2 card_max{card.x + card.width, card.y + card.height};
        const bool card_visible = card_alpha > 0.01F;
        const bool hovered = card_visible && !soft_focus && ImGui::IsMouseHoveringRect(card_min, card_max);
        card_hover_[index] = animation::damp(card_hover_[index], hovered ? 1.0F : 0.0F, 12.0F, delta_seconds);
        if (card_visible) {
            draw_card(draw,
                      model.config().products[index],
                      product_textures[index],
                      card,
                      card_hover_[index],
                      displayed_product_index_.has_value() && *displayed_product_index_ == index,
                      card_alpha);
        }

        ImGui::PushID(static_cast<int>(index));
        ImGui::SetCursorScreenPos(card_min);
        if (card_visible && ImGui::InvisibleButton("##select-product", ImVec2{card.width, card.height}) && !soft_focus) {
            static_cast<void>(model.select_product(index));
        }
        ImGui::PopID();
    }

    if (displayed_product_index_.has_value() && panel_progress > 0.01F) {
        const float target_panel_x = client_width * 0.5F;
        const float panel_x = client_width + (target_panel_x - client_width) * panel_progress;
        const ImVec2 moving_panel_min{minimum.x + panel_x, minimum.y};
        const ImVec2 moving_panel_max{moving_panel_min.x + client_width * 0.5F, maximum.y};
        draw->AddRectFilledMultiColor(moving_panel_min, moving_panel_max,
                                      IM_COL32(17, 15, 18, static_cast<int>(247.0F * panel_progress)),
                                      IM_COL32(10, 9, 11, static_cast<int>(250.0F * panel_progress)),
                                      IM_COL32(8, 7, 9, static_cast<int>(248.0F * panel_progress)),
                                      IM_COL32(14, 7, 13, static_cast<int>(244.0F * panel_progress)));
        if (panel_progress > 0.015F && *displayed_product_index_ < model.config().products.size()) {
            const SelectedAction action = draw_selected_product_panel(
                model.config().products[*displayed_product_index_],
                model.current_user(),
                moving_panel_min,
                moving_panel_max,
                content_alpha * panel_progress,
                time_seconds);
            const bool interactive = panel_progress_ >= 1.0F && !switching_product_ &&
                                     selected_index.has_value() && *selected_index == *displayed_product_index_;
            if (interactive && action == SelectedAction::close) {
                model.close_selection();
            } else if (interactive && action == SelectedAction::settings) {
                model.set_settings_open(true);
            } else if (interactive && action == SelectedAction::load) {
                static_cast<void>(model.begin_product_load());
            }
        }
    }

    if (model.settings_open()) {
        if (draw_settings_overlay(settings_, ImVec2{client_width, client_height}, content_alpha)) {
            model.set_settings_open(false);
        }
    }
    const bool has_load_overlay = model.load_state() != OperationState::idle;
    const float overlay_target = has_load_overlay ? 1.0F : 0.0F;
    load_overlay_alpha_ = animation::advance_toward(load_overlay_alpha_, overlay_target, 0.34F, delta_seconds);
    if (has_load_overlay && load_overlay_alpha_ > 0.001F) {
        draw_product_load_overlay(model,
                                  ImVec2{client_width, client_height},
                                  time_seconds,
                                  content_alpha * animation::ease_in_out_cubic(load_overlay_alpha_));
    }
}

const SettingsState& ProductScreen::settings() const noexcept {
    return settings_;
}

}  // namespace serein::ui
