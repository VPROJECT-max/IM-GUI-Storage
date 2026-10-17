#include "ui/Components.h"

#include "ui/Animation.h"
#include "ui/Theme.h"
#include "ui/UiContracts.h"

#include <algorithm>
#include <cmath>
#include <imgui_internal.h>

namespace serein::ui {
namespace {

ImFont* button_font() {
    const auto& fonts = ImGui::GetIO().Fonts->Fonts;
    return fonts.Size > 1 ? fonts[1] : ImGui::GetFont();
}

ImU32 button_color(const ButtonTone tone, const bool hovered, const bool held) {
    if (tone == ButtonTone::primary) {
        const ImVec4 color = held ? Theme::accent : (hovered ? Theme::accent_hover : Theme::accent);
        return color_u32(color);
    }
    if (tone == ButtonTone::danger) {
        return color_u32(Theme::danger, held ? 0.86F : (hovered ? 0.72F : 0.55F));
    }
    if (tone == ButtonTone::quiet) {
        return color_u32(Theme::surface_overlay, held ? 0.9F : (hovered ? 0.7F : 0.36F));
    }
    return color_u32(Theme::surface_overlay, held ? 1.0F : (hovered ? 0.88F : 0.68F));
}

void draw_image_cover_view(ImDrawList* draw_list,
                           ID3D11ShaderResourceView* view,
                           const int image_width,
                           const int image_height,
                           const ImVec2 minimum,
                           const ImVec2 maximum,
                           const float rounding,
                           const ImU32 tint) {
    if (view == nullptr || image_width <= 0 || image_height <= 0) {
        draw_image_placeholder(draw_list, minimum, maximum, "Artwork unavailable", rounding);
        return;
    }
    const float target_aspect = (maximum.x - minimum.x) / (maximum.y - minimum.y);
    const float image_aspect = static_cast<float>(image_width) / static_cast<float>(image_height);
    ImVec2 uv_min{0.0F, 0.0F};
    ImVec2 uv_max{1.0F, 1.0F};
    if (image_aspect > target_aspect) {
        const float visible = target_aspect / image_aspect;
        uv_min.x = (1.0F - visible) * 0.5F;
        uv_max.x = 1.0F - uv_min.x;
    } else {
        const float visible = image_aspect / target_aspect;
        uv_min.y = (1.0F - visible) * 0.5F;
        uv_max.y = 1.0F - uv_min.y;
    }
    draw_list->AddImageRounded(reinterpret_cast<ImTextureID>(view), minimum, maximum, uv_min, uv_max, tint, rounding);
}

}  // namespace

bool button(const char* id, const std::string_view label, const ImVec2 size, const ButtonTone tone) {
    ImGui::PushID(id);
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const bool pressed = ImGui::InvisibleButton("##button", size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    const bool focused = ImGui::IsItemFocused();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    if (tone != ButtonTone::quiet && tone != ButtonTone::danger) {
        draw->AddRectFilled(start + ImVec2{0.0F, 2.0F}, start + size + ImVec2{0.0F, 3.0F},
                            IM_COL32(0, 0, 0, button_shadow_alpha), button_corner_radius);
    }
    if (tone == ButtonTone::primary) {
        draw->AddRectFilled(start + ImVec2{0.0F, 3.0F}, start + size + ImVec2{0.0F, 4.0F},
                            IM_COL32(7, 5, 7, button_shadow_alpha), button_corner_radius);
        draw->AddRectFilled(start - ImVec2{1.0F, 1.0F}, start + size + ImVec2{1.0F, 1.0F},
                            color_u32(Theme::accent, hovered ? 0.07F : 0.025F), button_corner_radius + 1.0F);
    }
    draw->AddRectFilled(start, start + size, button_color(tone, hovered, held), button_corner_radius);
    if (tone == ButtonTone::primary) {
        const ImVec4 top_left = held ? ImVec4{0.40F, 0.075F, 0.38F, 1.0F} :
                                (hovered ? ImVec4{0.60F, 0.17F, 0.56F, 1.0F} : ImVec4{0.53F, 0.12F, 0.50F, 1.0F});
        const ImVec4 top_right = held ? ImVec4{0.32F, 0.070F, 0.31F, 1.0F} :
                                 (hovered ? ImVec4{0.52F, 0.14F, 0.49F, 1.0F} : ImVec4{0.44F, 0.095F, 0.42F, 1.0F});
        const ImVec4 bottom_left = held ? ImVec4{0.28F, 0.055F, 0.27F, 1.0F} : ImVec4{0.38F, 0.070F, 0.36F, 1.0F};
        const ImVec4 bottom_right = held ? ImVec4{0.15F, 0.13F, 0.15F, 1.0F} : ImVec4{0.22F, 0.16F, 0.22F, 1.0F};
        draw->AddRectFilledMultiColor(start + ImVec2{2.0F, 2.0F}, start + size - ImVec2{2.0F, 2.0F},
                                      color_u32(top_left), color_u32(top_right), color_u32(bottom_right), color_u32(bottom_left));
        draw->AddLine(start + ImVec2{5.0F, 1.5F}, ImVec2{start.x + size.x - 5.0F, start.y + 1.5F},
                      color_u32(ImVec4{1.0F, 0.54F, 0.94F, 1.0F}, held ? 0.10F : 0.20F), 1.0F);
    } else if (tone == ButtonTone::secondary) {
        const ImU32 top_left = color_u32(ImVec4{0.095F, 0.087F, 0.101F, 1.0F}, hovered ? 0.96F : 0.82F);
        const ImU32 top_right = color_u32(ImVec4{0.076F, 0.071F, 0.080F, 1.0F}, hovered ? 0.98F : 0.88F);
        const ImU32 bottom_left = color_u32(ImVec4{0.045F, 0.040F, 0.049F, 1.0F}, held ? 0.98F : 0.88F);
        const ImU32 bottom_right = color_u32(ImVec4{0.030F, 0.029F, 0.032F, 1.0F}, held ? 1.0F : 0.92F);
        draw->AddRectFilledMultiColor(start + ImVec2{2.0F, 2.0F}, start + size - ImVec2{2.0F, 2.0F},
                                      top_left, top_right, bottom_right, bottom_left);
    }
    if (tone != ButtonTone::primary) {
        draw->AddRect(start, start + size, color_u32(focused ? Theme::accent : Theme::border_subtle), button_corner_radius);
    }
    if (focused) {
        draw->AddRect(start - ImVec2{2.0F, 2.0F}, start + size + ImVec2{2.0F, 2.0F}, color_u32(Theme::accent, button_focus_alpha), button_corner_radius + 2.0F, ImDrawFlags_None, 1.0F);
    }
    ImFont* label_font = button_font();
    const ImVec2 text_size = label_font->CalcTextSizeA(button_text_size, 1000.0F, 0.0F, label.data(), label.data() + label.size());
    const ImU32 text_color = color_u32(tone == ButtonTone::primary ? ImVec4{0.94F, 0.89F, 0.94F, 1.0F} : Theme::content_primary);
    draw->AddText(label_font, button_text_size, start + (size - text_size) * 0.5F, text_color, label.data(), label.data() + label.size());
    ImGui::PopID();
    return pressed;
}

bool icon_button(const char* id, const std::string_view glyph, const ImVec2 size, const bool quiet) {
    return button(id, glyph, size, quiet ? ButtonTone::quiet : ButtonTone::secondary);
}

bool toggle(const char* id, bool* value) {
    ImGui::PushID(id);
    const ImVec2 size{38.0F, 22.0F};
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const bool pressed = ImGui::InvisibleButton("##toggle", size);
    if (pressed) {
        *value = !*value;
    }
    const bool hovered = ImGui::IsItemHovered();
    ImDrawList* draw = ImGui::GetWindowDrawList();
    const ImU32 track = *value ? color_u32(hovered ? Theme::accent_hover : Theme::accent) : color_u32(Theme::surface_overlay);
    draw->AddRectFilled(start, start + size, track, size.y * 0.5F);
    const float knob_x = *value ? start.x + size.x - 11.0F : start.x + 11.0F;
    draw->AddCircleFilled(ImVec2{knob_x, start.y + 11.0F}, 7.0F, color_u32(*value ? Theme::surface_base : Theme::content_secondary));
    ImGui::PopID();
    return pressed;
}

void spinner(const ImVec2 center, const float radius, const float thickness, const ImU32 color, const float seconds) {
    constexpr float tau = 6.28318530718F;
    const float start = std::fmod(seconds * 2.4F, tau);
    const float sweep = 1.4F + animation::pulse(seconds, 0.9F) * 2.2F;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->PathClear();
    draw->PathArcTo(center, radius, start, start + sweep, 32);
    draw->PathStroke(color, 0, thickness);
}

void progress_ring(const ImVec2 center,
                   const float radius,
                   const float thickness,
                   const float progress,
                   const ImU32 track,
                   const ImU32 fill) {
    constexpr float pi = 3.14159265359F;
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddCircle(center, radius, track, 48, thickness);
    draw->PathClear();
    draw->PathArcTo(center, radius, -pi * 0.5F, -pi * 0.5F + animation::clamp01(progress) * 2.0F * pi, 48);
    draw->PathStroke(fill, 0, thickness);
}

void status_pill(const std::string_view label, const ImU32 color) {
    const ImVec2 start = ImGui::GetCursorScreenPos();
    const ImVec2 text_size = ImGui::CalcTextSize(label.data(), label.data() + label.size());
    const ImVec2 size{text_size.x + 20.0F, 24.0F};
    ImGui::Dummy(size);
    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(start, start + size, color & IM_COL32_A_MASK ? (color & 0x40FFFFFFU) : color, 12.0F);
    draw->AddCircleFilled(start + ImVec2{10.0F, 12.0F}, 3.0F, color);
    draw->AddText(start + ImVec2{16.0F, 4.0F}, color_u32(Theme::content_primary), label.data(), label.data() + label.size());
}

void gradient_text(ImDrawList* draw_list,
                   ImFont* font,
                   const float font_size,
                   const ImVec2 position,
                   const std::string_view text,
                   const ImU32 left_color,
                   const ImU32 right_color) {
    if (text.empty()) {
        return;
    }
    const int vertex_start = draw_list->VtxBuffer.Size;
    draw_list->AddText(font, font_size, position, IM_COL32_WHITE, text.data(), text.data() + text.size());
    const int vertex_end = draw_list->VtxBuffer.Size;
    const ImVec2 text_size = font->CalcTextSizeA(font_size, 10000.0F, 0.0F, text.data(), text.data() + text.size());
    ImGui::ShadeVertsLinearColorGradientKeepAlpha(draw_list,
                                                   vertex_start,
                                                   vertex_end,
                                                   position,
                                                   ImVec2{position.x + std::max(text_size.x, 1.0F), position.y},
                                                   left_color,
                                                   right_color);
}

void draw_icon(ImDrawList* draw,
               const IconGlyph glyph,
               const ImVec2 minimum,
               const float size,
               const ImU32 color,
               const float thickness) {
    const auto p = [&](const float x, const float y) {
        return minimum + ImVec2{x * size / 24.0F, y * size / 24.0F};
    };
    const float stroke = std::max(thickness * size / 24.0F, 1.0F);
    switch (glyph) {
    case IconGlyph::close:
        draw->AddLine(p(6.0F, 6.0F), p(18.0F, 18.0F), color, stroke);
        draw->AddLine(p(18.0F, 6.0F), p(6.0F, 18.0F), color, stroke);
        break;
    case IconGlyph::product:
        draw->PathClear();
        draw->PathLineTo(p(5.0F, 5.0F));
        draw->PathLineTo(p(20.0F, 12.0F));
        draw->PathLineTo(p(5.0F, 19.0F));
        draw->PathFillConvex(color);
        break;
    case IconGlyph::calendar:
        draw->AddRect(p(3.0F, 4.0F), p(21.0F, 21.0F), color, 2.0F * size / 24.0F, 0, stroke);
        draw->AddLine(p(3.0F, 9.0F), p(21.0F, 9.0F), color, stroke);
        draw->AddLine(p(8.0F, 2.0F), p(8.0F, 6.0F), color, stroke);
        draw->AddLine(p(16.0F, 2.0F), p(16.0F, 6.0F), color, stroke);
        break;
    case IconGlyph::user:
        draw->AddCircle(p(12.0F, 7.0F), 4.0F * size / 24.0F, color, 18, stroke);
        draw->PathClear();
        draw->PathLineTo(p(5.0F, 21.0F));
        draw->PathLineTo(p(5.0F, 19.0F));
        draw->PathBezierCubicCurveTo(p(5.0F, 16.8F), p(6.8F, 15.0F), p(9.0F, 15.0F));
        draw->PathLineTo(p(15.0F, 15.0F));
        draw->PathBezierCubicCurveTo(p(17.2F, 15.0F), p(19.0F, 16.8F), p(19.0F, 19.0F));
        draw->PathLineTo(p(19.0F, 21.0F));
        draw->PathStroke(color, 0, stroke);
        break;
    case IconGlyph::status:
        draw->PathClear();
        draw->PathLineTo(p(12.0F, 2.0F));
        draw->PathBezierCubicCurveTo(p(14.0F, 3.8F), p(17.0F, 5.0F), p(20.0F, 5.0F));
        draw->PathLineTo(p(20.0F, 13.0F));
        draw->PathBezierCubicCurveTo(p(20.0F, 17.0F), p(17.0F, 20.0F), p(12.0F, 22.0F));
        draw->PathBezierCubicCurveTo(p(7.0F, 20.0F), p(4.0F, 17.0F), p(4.0F, 13.0F));
        draw->PathLineTo(p(4.0F, 5.0F));
        draw->PathBezierCubicCurveTo(p(7.0F, 5.0F), p(10.0F, 3.8F), p(12.0F, 2.0F));
        draw->PathStroke(color, ImDrawFlags_Closed, stroke);
        break;
    case IconGlyph::settings: {
        const ImVec2 center = p(12.0F, 12.0F);
        draw->AddCircle(center, 3.0F * size / 24.0F, color, 18, stroke);
        draw->AddCircle(center, 7.0F * size / 24.0F, color, 24, stroke);
        for (int index = 0; index < 8; ++index) {
            constexpr float tau = 6.28318530718F;
            const float angle = tau * static_cast<float>(index) / 8.0F;
            const ImVec2 direction{std::cos(angle), std::sin(angle)};
            draw->AddLine(center + direction * (7.0F * size / 24.0F), center + direction * (9.0F * size / 24.0F), color, stroke);
        }
        break;
    }
    case IconGlyph::load:
        draw->PathClear();
        draw->PathLineTo(p(9.0F, 12.0F));
        draw->PathBezierCubicCurveTo(p(11.0F, 7.0F), p(15.0F, 3.0F), p(22.0F, 2.0F));
        draw->PathBezierCubicCurveTo(p(22.0F, 8.0F), p(18.0F, 12.0F), p(13.0F, 15.0F));
        draw->PathLineTo(p(9.0F, 12.0F));
        draw->PathStroke(color, ImDrawFlags_Closed, stroke);
        draw->AddLine(p(12.0F, 15.0F), p(12.0F, 20.0F), color, stroke);
        draw->AddLine(p(9.0F, 12.0F), p(4.0F, 12.0F), color, stroke);
        break;
    }
}

void draw_image_cover(ImDrawList* draw_list,
                      const Texture* texture,
                      const ImVec2 minimum,
                      const ImVec2 maximum,
                      const float rounding,
                      const ImU32 tint) {
    if (texture == nullptr || texture->view == nullptr || texture->width <= 0 || texture->height <= 0) {
        draw_image_placeholder(draw_list, minimum, maximum, "Artwork unavailable", rounding);
        return;
    }
    draw_image_cover_view(draw_list, texture->view.Get(), texture->width, texture->height, minimum, maximum, rounding, tint);
}

void draw_blurred_image_cover(ImDrawList* draw_list,
                              const Texture* texture,
                              const ImVec2 minimum,
                              const ImVec2 maximum,
                              const float rounding,
                              const ImU32 tint) {
    if (texture == nullptr || texture->width <= 0 || texture->height <= 0) {
        draw_image_placeholder(draw_list, minimum, maximum, "Artwork unavailable", rounding);
        return;
    }
    ID3D11ShaderResourceView* view = texture->blurred_view != nullptr ? texture->blurred_view.Get() : texture->view.Get();
    draw_image_cover_view(draw_list, view, texture->width, texture->height, minimum, maximum, rounding, tint);
}

void draw_image_placeholder(ImDrawList* draw_list,
                            const ImVec2 minimum,
                            const ImVec2 maximum,
                            const std::string_view label,
                            const float rounding) {
    draw_list->AddRectFilled(minimum, maximum, color_u32(Theme::surface_overlay), rounding);
    draw_list->AddLine(minimum + ImVec2{12.0F, 12.0F}, maximum - ImVec2{12.0F, 12.0F}, color_u32(Theme::border_subtle), 1.0F);
    const ImVec2 text_size = ImGui::CalcTextSize(label.data(), label.data() + label.size());
    draw_list->AddText((minimum + maximum - text_size) * 0.5F, color_u32(Theme::content_muted), label.data(), label.data() + label.size());
}

}  // namespace serein::ui
