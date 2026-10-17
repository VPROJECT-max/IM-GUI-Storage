#include "ui/screens/SelectedProductPanel.h"

#include "ui/Components.h"
#include "ui/Theme.h"
#include "ui/UiContracts.h"

#include <algorithm>
#include <string>

namespace serein::ui {
namespace {

ImFont* font_at(const int index) {
    const auto& fonts = ImGui::GetIO().Fonts->Fonts;
    return fonts.Size > index ? fonts[index] : ImGui::GetFont();
}

void label_value(ImDrawList* draw,
                 const ImVec2 position,
                 const IconGlyph icon,
                 const char* label,
                 const std::string_view value,
                 const float value_right,
                 const float scale,
                 const float alpha) {
    draw_icon(draw, icon, position, 13.0F * scale, color_u32(Theme::accent, alpha), 1.7F);
    draw->AddText(font_at(1), 12.0F * scale, position + ImVec2{19.0F * scale, 0.0F},
                  color_u32(Theme::content_secondary, alpha), label);
    const ImVec2 size = font_at(0)->CalcTextSizeA(12.0F * scale, 1000.0F, 0.0F,
                                                  value.data(), value.data() + value.size());
    draw->AddText(font_at(0), 12.0F * scale, ImVec2{value_right - size.x, position.y}, color_u32(Theme::content_muted, alpha), value.data(), value.data() + value.size());
}

}  // namespace

SelectedAction draw_selected_product_panel(const Product& product,
                                           const std::string_view current_user,
                                           const ImVec2 minimum,
                                           const ImVec2 maximum,
                                           const float alpha,
                                           const float time_seconds) {
    static_cast<void>(time_seconds);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(alpha, 0.0F, 1.0F));
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const float scale = std::min((maximum.x - minimum.x) / 272.0F, (maximum.y - minimum.y) / 329.0F);
    const float left = 18.0F * scale;
    const float value_right = maximum.x - 16.0F * scale;
    draw->AddRectFilledMultiColor(minimum,
                                  maximum,
                                  color_u32(ImVec4{0.052F, 0.047F, 0.055F, 1.0F}, alpha),
                                  color_u32(ImVec4{0.026F, 0.024F, 0.029F, 1.0F}, alpha),
                                  color_u32(ImVec4{0.018F, 0.016F, 0.020F, 1.0F}, alpha),
                                  color_u32(ImVec4{0.041F, 0.018F, 0.038F, 1.0F}, alpha));
    draw->AddLine(minimum, ImVec2{minimum.x, maximum.y}, color_u32(Theme::border_subtle, alpha), 1.0F);

    draw->AddText(font_at(2), 23.0F * scale, minimum + ImVec2{left, 18.0F * scale}, color_u32(Theme::content_primary, alpha), "Selected");
    draw->AddText(font_at(1), 14.0F * scale, minimum + ImVec2{left, 49.0F * scale}, color_u32(Theme::accent, alpha), product.name.c_str());

    const float close_size = 25.0F * scale;
    ImGui::SetCursorScreenPos(ImVec2{maximum.x - close_size - 12.0F * scale, minimum.y + 8.0F * scale});
    const bool close_requested = ImGui::InvisibleButton("##close-selection", ImVec2{close_size, close_size});
    const bool close_hovered = ImGui::IsItemHovered();
    draw_icon(draw,
              IconGlyph::close,
              ImVec2{maximum.x - close_size - 7.5F * scale, minimum.y + 12.5F * scale},
              15.0F * scale,
              color_u32(close_hovered ? Theme::content_primary : Theme::content_muted, alpha),
              1.8F);

    const float row_one = 95.0F * scale;
    const float row_two = 123.0F * scale;
    const float row_three = 151.0F * scale;
    label_value(draw, minimum + ImVec2{left, row_one}, IconGlyph::status, "Status", product.status, value_right, scale, alpha);
    label_value(draw, minimum + ImVec2{left, row_two}, IconGlyph::calendar, "Last update", product.last_update, value_right, scale, alpha);
    label_value(draw, minimum + ImVec2{left, row_three}, IconGlyph::user, "User", current_user, value_right, scale, alpha);

    SelectedAction action = close_requested ? SelectedAction::close : SelectedAction::none;
    const float width = maximum.x - minimum.x - left * 2.0F;
    const float actions_top = 225.0F * scale;
    const float settings_y = 241.0F * scale;
    const float load_y = 274.0F * scale;
    draw->AddRectFilledMultiColor(ImVec2{minimum.x + 1.0F, minimum.y + actions_top},
                                  ImVec2{maximum.x, maximum.y},
                                  IM_COL32(42, 18, 40, 14), IM_COL32(20, 16, 21, 28),
                                  IM_COL32(12, 8, 13, 86), IM_COL32(29, 9, 27, 66));
    const float settings_width = (width - 10.0F * scale) * 0.5F;
    const float settings_x = minimum.x + left + settings_width + 10.0F * scale;
    ImGui::SetCursorScreenPos(ImVec2{settings_x, minimum.y + settings_y});
    if (!close_requested && button("settings", "Settings", ImVec2{settings_width, 26.0F * scale}, ButtonTone::secondary)) {
        action = SelectedAction::settings;
    }
    const ImVec2 settings_text = ImGui::CalcTextSize("Settings");
    const float settings_center = settings_x + settings_width * 0.5F;
    draw_icon(draw, IconGlyph::settings,
              ImVec2{settings_center - settings_text.x * 0.5F - 21.0F * scale, minimum.y + settings_y + 6.0F * scale},
              14.0F * scale, color_u32(Theme::accent, alpha), 1.6F);
    ImGui::SetCursorScreenPos(ImVec2{minimum.x + left, minimum.y + load_y});
    if (!close_requested && button("load", "Load", ImVec2{width, 27.0F * scale}, ButtonTone::primary)) {
        action = SelectedAction::load;
    }
    const ImVec2 load_text = ImGui::CalcTextSize("Load");
    const float load_center = minimum.x + left + width * 0.5F;
    draw_icon(draw, IconGlyph::load,
              ImVec2{load_center + load_text.x * 0.5F + 6.0F * scale, minimum.y + load_y + 6.0F * scale},
              14.0F * scale, color_u32(Theme::content_primary, alpha), 1.6F);
    ImGui::PopStyleVar();
    return action;
}

}  // namespace serein::ui
