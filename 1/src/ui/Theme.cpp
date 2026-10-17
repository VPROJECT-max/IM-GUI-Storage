#include "ui/Theme.h"

#include <algorithm>

namespace serein::ui {

const ImVec4 Theme::surface_base{0.025F, 0.023F, 0.027F, 1.0F};
const ImVec4 Theme::surface_raised{0.045F, 0.043F, 0.047F, 0.98F};
const ImVec4 Theme::surface_overlay{0.075F, 0.071F, 0.078F, 0.96F};
const ImVec4 Theme::border_subtle{0.24F, 0.22F, 0.25F, 0.42F};
const ImVec4 Theme::border_strong{0.38F, 0.34F, 0.39F, 0.58F};
const ImVec4 Theme::content_primary{0.93F, 0.91F, 0.94F, 1.0F};
const ImVec4 Theme::content_secondary{0.72F, 0.68F, 0.73F, 1.0F};
const ImVec4 Theme::content_muted{0.51F, 0.48F, 0.53F, 1.0F};
const ImVec4 Theme::accent{0.53F, 0.12F, 0.50F, 1.0F};
const ImVec4 Theme::accent_hover{0.62F, 0.17F, 0.58F, 1.0F};
const ImVec4 Theme::accent_soft{0.21F, 0.060F, 0.20F, 1.0F};
const ImVec4 Theme::success{0.53F, 0.12F, 0.50F, 1.0F};
const ImVec4 Theme::warning{0.94F, 0.72F, 0.35F, 1.0F};
const ImVec4 Theme::danger{0.94F, 0.39F, 0.42F, 1.0F};

void Theme::apply(const float scale) {
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowPadding = ImVec2{20.0F, 20.0F};
    style.FramePadding = ImVec2{12.0F, 9.0F};
    style.ItemSpacing = ImVec2{10.0F, 10.0F};
    style.ItemInnerSpacing = ImVec2{8.0F, 6.0F};
    style.WindowRounding = large_radius;
    style.ChildRounding = radius;
    style.FrameRounding = small_radius;
    style.PopupRounding = radius;
    style.ScrollbarRounding = 12.0F;
    style.GrabRounding = 8.0F;
    style.WindowBorderSize = 0.0F;
    style.ChildBorderSize = 1.0F;
    style.FrameBorderSize = 1.0F;
    style.ScrollbarSize = 10.0F;

    auto& colors = style.Colors;
    colors[ImGuiCol_Text] = content_primary;
    colors[ImGuiCol_TextDisabled] = content_muted;
    colors[ImGuiCol_WindowBg] = surface_base;
    colors[ImGuiCol_ChildBg] = ImVec4{0.0F, 0.0F, 0.0F, 0.0F};
    colors[ImGuiCol_PopupBg] = surface_overlay;
    colors[ImGuiCol_Border] = border_subtle;
    colors[ImGuiCol_FrameBg] = ImVec4{0.055F, 0.08F, 0.09F, 0.96F};
    colors[ImGuiCol_FrameBgHovered] = ImVec4{0.075F, 0.11F, 0.12F, 1.0F};
    colors[ImGuiCol_FrameBgActive] = ImVec4{0.09F, 0.14F, 0.15F, 1.0F};
    colors[ImGuiCol_Button] = accent_soft;
    colors[ImGuiCol_ButtonHovered] = ImVec4{accent.x, accent.y, accent.z, 0.72F};
    colors[ImGuiCol_ButtonActive] = accent;
    colors[ImGuiCol_CheckMark] = accent;
    colors[ImGuiCol_SliderGrab] = accent;
    colors[ImGuiCol_SliderGrabActive] = accent_hover;
    colors[ImGuiCol_Header] = ImVec4{accent.x, accent.y, accent.z, 0.16F};
    colors[ImGuiCol_HeaderHovered] = ImVec4{accent.x, accent.y, accent.z, 0.25F};
    colors[ImGuiCol_HeaderActive] = ImVec4{accent.x, accent.y, accent.z, 0.34F};
    colors[ImGuiCol_NavCursor] = accent_hover;
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4{0.0F, 0.02F, 0.025F, 0.78F};

    if (scale != 1.0F) {
        style.ScaleAllSizes(std::max(scale, 0.5F));
    }
}

ImU32 color_u32(const ImVec4& color, const float alpha_multiplier) {
    ImVec4 adjusted = color;
    adjusted.w = std::clamp(adjusted.w * alpha_multiplier, 0.0F, 1.0F);
    return ImGui::ColorConvertFloat4ToU32(adjusted);
}

}  // namespace serein::ui
