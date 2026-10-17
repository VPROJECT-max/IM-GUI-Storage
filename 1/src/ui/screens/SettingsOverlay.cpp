#include "ui/screens/SettingsOverlay.h"

#include "ui/Components.h"
#include "ui/Theme.h"
#include "ui/UiContracts.h"

#include <algorithm>

namespace serein::ui {
namespace {

void setting_row(const char* id, const char* label, const char* detail, bool* value, const ImVec2 position, ImDrawList* draw) {
    draw->AddText(position, color_u32(Theme::content_primary), label);
    draw->AddText(ImVec2{position.x, position.y + 19.0F}, color_u32(Theme::content_muted), detail);
    ImGui::SetCursorScreenPos(ImVec2{position.x + 275.0F, position.y + 5.0F});
    static_cast<void>(toggle(id, value));
}

}  // namespace

bool draw_settings_overlay(SettingsState& state, const ImVec2 display_size, const float alpha) {
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(alpha, 0.0F, 1.0F));
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(ImVec2{0.0F, 0.0F}, display_size, IM_COL32(1, 7, 9, 196));
    const ImVec2 size{390.0F, 306.0F};
    const ImVec2 minimum{(display_size.x - size.x) * 0.5F, (display_size.y - size.y) * 0.5F};
    const ImVec2 maximum = minimum + size;
    draw->AddRectFilled(minimum + ImVec2{0.0F, 8.0F}, maximum + ImVec2{0.0F, 12.0F}, IM_COL32(0, 0, 0, 62), Theme::large_radius);
    draw->AddRectFilled(minimum, maximum, color_u32(Theme::surface_raised), Theme::large_radius);
    draw->AddRect(minimum, maximum, color_u32(Theme::border_strong), Theme::large_radius);
    draw->AddText(ImGui::GetIO().Fonts->Fonts.Size > 2 ? ImGui::GetIO().Fonts->Fonts[2] : ImGui::GetFont(), 23.0F, minimum + ImVec2{24.0F, 20.0F}, color_u32(Theme::content_primary), "Preferences");
    draw->AddText(minimum + ImVec2{24.0F, 52.0F}, color_u32(Theme::content_muted), "Local UI behavior for this template.");
    draw->AddLine(minimum + ImVec2{24.0F, 78.0F}, ImVec2{maximum.x - 24.0F, minimum.y + 78.0F}, color_u32(Theme::border_subtle));

    setting_row("deep-background", "Deep background", "Increase the cinematic image scrim.", &state.deep_background, minimum + ImVec2{24.0F, 96.0F}, draw);
    setting_row("reduced-motion", "Reduced motion", "Use shorter transitions and fewer pulses.", &state.reduced_motion, minimum + ImVec2{24.0F, 150.0F}, draw);
    setting_row("strong-selection", "Strong selection", "Emphasize the active product outline.", &state.strong_selection, minimum + ImVec2{24.0F, 204.0F}, draw);

    ImGui::SetCursorScreenPos(minimum + ImVec2{24.0F, 258.0F});
    const bool done = button("settings-done", "Done", ImVec2{size.x - 48.0F, 34.0F});
    const bool escape = keyboard_intent(false, ImGui::IsKeyPressed(ImGuiKey_Escape)) == KeyboardIntent::cancel;
    ImGui::PopStyleVar();
    return done || escape;
}

}  // namespace serein::ui
