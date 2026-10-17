#pragma once

#include <imgui.h>

namespace serein::ui {

struct Theme {
    static constexpr float base_unit = 4.0F;
    static constexpr float small_radius = 8.0F;
    static constexpr float radius = 12.0F;
    static constexpr float large_radius = 18.0F;

    static const ImVec4 surface_base;
    static const ImVec4 surface_raised;
    static const ImVec4 surface_overlay;
    static const ImVec4 border_subtle;
    static const ImVec4 border_strong;
    static const ImVec4 content_primary;
    static const ImVec4 content_secondary;
    static const ImVec4 content_muted;
    static const ImVec4 accent;
    static const ImVec4 accent_hover;
    static const ImVec4 accent_soft;
    static const ImVec4 success;
    static const ImVec4 warning;
    static const ImVec4 danger;

    static void apply(float scale = 1.0F);
};

[[nodiscard]] ImU32 color_u32(const ImVec4& color, float alpha_multiplier = 1.0F);

}  // namespace serein::ui
