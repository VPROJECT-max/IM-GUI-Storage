#pragma once

#include "render/TextureStore.h"

#include <imgui.h>

#include <string_view>

namespace serein::ui {

enum class ButtonTone {
    primary,
    secondary,
    quiet,
    danger,
};

enum class IconGlyph {
    status,
    calendar,
    user,
    settings,
    load,
    product,
    close,
};

[[nodiscard]] bool button(const char* id, std::string_view label, ImVec2 size, ButtonTone tone = ButtonTone::primary);
[[nodiscard]] bool icon_button(const char* id, std::string_view glyph, ImVec2 size, bool quiet = true);
[[nodiscard]] bool toggle(const char* id, bool* value);
void spinner(ImVec2 center, float radius, float thickness, ImU32 color, float seconds);
void progress_ring(ImVec2 center, float radius, float thickness, float progress, ImU32 track, ImU32 fill);
void status_pill(std::string_view label, ImU32 color);
void gradient_text(ImDrawList* draw_list,
                   ImFont* font,
                   float font_size,
                   ImVec2 position,
                   std::string_view text,
                   ImU32 left_color,
                   ImU32 right_color);
void draw_icon(ImDrawList* draw_list,
               IconGlyph glyph,
               ImVec2 minimum,
               float size,
               ImU32 color,
               float thickness = 1.5F);
void draw_image_cover(ImDrawList* draw_list,
                      const Texture* texture,
                      ImVec2 minimum,
                      ImVec2 maximum,
                      float rounding,
                      ImU32 tint = IM_COL32_WHITE);
void draw_blurred_image_cover(ImDrawList* draw_list,
                              const Texture* texture,
                              ImVec2 minimum,
                              ImVec2 maximum,
                              float rounding,
                              ImU32 tint = IM_COL32_WHITE);
void draw_image_placeholder(ImDrawList* draw_list, ImVec2 minimum, ImVec2 maximum, std::string_view label, float rounding);

}  // namespace serein::ui
