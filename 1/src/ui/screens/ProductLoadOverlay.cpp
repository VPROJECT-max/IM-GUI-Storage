#include "ui/screens/ProductLoadOverlay.h"

#include "ui/Components.h"
#include "ui/Theme.h"

#include <algorithm>

namespace serein::ui {
namespace {

ImFont* font_at(const int index) {
    const auto& fonts = ImGui::GetIO().Fonts->Fonts;
    return fonts.Size > index ? fonts[index] : ImGui::GetFont();
}

}  // namespace

void draw_product_load_overlay(const AppModel& model,
                               const ImVec2 display_size,
                               const float time_seconds,
                               const float alpha) {
    const float visible_alpha = std::clamp(alpha, 0.0F, 1.0F);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, visible_alpha);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(ImVec2{0.0F, 0.0F}, display_size, IM_COL32(1, 7, 9, static_cast<int>(206.0F * visible_alpha)));
    const ImVec2 size{344.0F, 190.0F};
    const ImVec2 minimum{(display_size.x - size.x) * 0.5F, (display_size.y - size.y) * 0.5F};
    const ImVec2 maximum = minimum + size;
    draw->AddRectFilled(minimum + ImVec2{0.0F, 9.0F}, maximum + ImVec2{0.0F, 13.0F},
                        IM_COL32(0, 0, 0, static_cast<int>(66.0F * visible_alpha)), Theme::large_radius);
    draw->AddRectFilled(minimum, maximum, color_u32(Theme::surface_raised, 0.98F * visible_alpha), Theme::large_radius);
    draw->AddRect(minimum, maximum, color_u32(Theme::border_strong, visible_alpha), Theme::large_radius);

    const ImVec2 center{minimum.x + size.x * 0.5F, minimum.y + 62.0F};
    const OperationState state = model.load_state();
    if (state == OperationState::working) {
        progress_ring(center, 30.0F, 4.0F, model.load_progress(), color_u32(Theme::border_subtle, visible_alpha), color_u32(Theme::accent, visible_alpha));
        spinner(center, 35.0F, 2.0F, color_u32(Theme::accent_hover, 0.6F * visible_alpha), time_seconds);
    } else {
        const ImVec4 semantic = state == OperationState::succeeded ? Theme::success : Theme::danger;
        draw->AddCircle(center, 31.0F, color_u32(semantic, 0.45F * visible_alpha), 40, 3.0F);
        draw->AddCircleFilled(center, 19.0F, color_u32(semantic, 0.14F * visible_alpha), 32);
        if (state == OperationState::succeeded) {
            draw->AddLine(center + ImVec2{-8.0F, 0.0F}, center + ImVec2{-2.0F, 7.0F}, color_u32(semantic), 2.5F);
            draw->AddLine(center + ImVec2{-2.0F, 7.0F}, center + ImVec2{10.0F, -7.0F}, color_u32(semantic), 2.5F);
        } else {
            draw->AddLine(center + ImVec2{-7.0F, -7.0F}, center + ImVec2{7.0F, 7.0F}, color_u32(semantic), 2.5F);
            draw->AddLine(center + ImVec2{7.0F, -7.0F}, center + ImVec2{-7.0F, 7.0F}, color_u32(semantic), 2.5F);
        }
    }

    const char* title = state == OperationState::working ? "Loading Product" :
                        (state == OperationState::succeeded ? "Product Ready" : "Loading Failed");
    constexpr float title_font_size = 16.0F;
    const ImVec2 title_size = font_at(1)->CalcTextSizeA(title_font_size, 1000.0F, 0.0F, title);
    draw->AddText(font_at(1), title_font_size, ImVec2{center.x - title_size.x * 0.5F, minimum.y + 110.0F}, color_u32(Theme::content_primary, visible_alpha), title);
    const auto message = model.load_message();
    constexpr float message_font_size = 13.5F;
    const ImVec2 message_size = font_at(0)->CalcTextSizeA(message_font_size, 1000.0F, 0.0F, message.data(), message.data() + message.size());
    draw->AddText(font_at(0), message_font_size, ImVec2{center.x - message_size.x * 0.5F, minimum.y + 138.0F}, color_u32(Theme::content_muted, visible_alpha), message.data(), message.data() + message.size());
    ImGui::PopStyleVar();
}

}  // namespace serein::ui
