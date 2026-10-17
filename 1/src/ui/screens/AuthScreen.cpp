#include "ui/screens/AuthScreen.h"

#include "ui/Components.h"
#include "ui/Theme.h"
#include "ui/UiContracts.h"

#include <algorithm>

namespace serein::ui {
namespace {

constexpr ImVec4 page_color{0.025F, 0.023F, 0.027F, 1.0F};
constexpr ImVec4 field_color{0.055F, 0.053F, 0.057F, 1.0F};
constexpr ImVec4 field_border{0.19F, 0.18F, 0.20F, 1.0F};
constexpr ImVec4 purple{0.53F, 0.12F, 0.50F, 1.0F};
constexpr ImVec4 purple_top{0.54F, 0.13F, 0.51F, 1.0F};
constexpr ImVec4 purple_bottom{0.32F, 0.07F, 0.31F, 1.0F};
constexpr ImVec4 primary_text{0.91F, 0.89F, 0.92F, 1.0F};
constexpr ImVec4 secondary_text{0.70F, 0.67F, 0.71F, 1.0F};
constexpr ImVec4 icon_color{0.48F, 0.45F, 0.50F, 1.0F};

ImFont* font_at(const int index) {
    const ImVector<ImFont*>& fonts = ImGui::GetIO().Fonts->Fonts;
    return fonts.Size > index ? fonts[index] : ImGui::GetFont();
}

ImU32 rgba(const ImVec4 color) {
    return ImGui::ColorConvertFloat4ToU32(color);
}

void text_at(ImDrawList* draw,
             const ImVec2 position,
             ImFont* font,
             const float size,
             const ImVec4 color,
             const char* text) {
    draw->AddText(font, size, position, rgba(color), text);
}

void draw_person_icon(ImDrawList* draw, const ImVec2 center, const float scale) {
    const ImU32 color = rgba(icon_color);
    draw->AddCircleFilled(center + ImVec2{0.0F, -4.5F * scale}, 3.0F * scale, color, 16);
    draw->AddRectFilled(center + ImVec2{-5.7F * scale, 0.0F}, center + ImVec2{5.7F * scale, 7.0F * scale}, color, 3.0F * scale);
}

void draw_key_icon(ImDrawList* draw, const ImVec2 center, const float scale) {
    const ImU32 color = rgba(icon_color);
    draw->AddCircle(center + ImVec2{-2.8F * scale, 0.0F}, 4.2F * scale, color, 16, 1.7F * scale);
    draw->AddLine(center + ImVec2{1.2F * scale, 0.0F}, center + ImVec2{7.2F * scale, 0.0F}, color, 1.7F * scale);
    draw->AddLine(center + ImVec2{5.0F * scale, 0.0F}, center + ImVec2{5.0F * scale, 2.8F * scale}, color, 1.5F * scale);
    draw->AddLine(center + ImVec2{7.0F * scale, 0.0F}, center + ImVec2{7.0F * scale, 2.0F * scale}, color, 1.5F * scale);
}

void draw_eye_icon(ImDrawList* draw, const ImVec2 center, const bool active, const float scale) {
    const ImU32 color = rgba(active ? purple : secondary_text);
    draw->AddEllipse(center, ImVec2{7.8F * scale, 4.6F * scale}, color, 0.0F, 20, 1.35F * scale);
    draw->AddCircleFilled(center, 2.0F * scale, color, 16);
}

}  // namespace

AuthScreen::AuthScreen() {
    constexpr char demo_user[] = "serein";
    std::copy(std::begin(demo_user), std::end(demo_user), username_.begin());
}

void AuthScreen::render(AppModel& model,
                        const Texture* artwork,
                        const ImVec2 minimum,
                        const ImVec2 maximum,
                        const float time_seconds,
                        const float alpha) {
    static_cast<void>(time_seconds);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(alpha, 0.0F, 1.0F));
    ImDrawList* draw = ImGui::GetWindowDrawList();

    // The supplied reference is a 572 x 329 login window. Its inner geometry is
    // enlarged proportionally to fill this application's fixed 826 x 480 client.
    const float reference_scale = std::min((maximum.x - minimum.x) / 572.0F,
                                           (maximum.y - minimum.y) / 329.0F);
    const auto point = [&](const float x, const float y) {
        return minimum + ImVec2{x * reference_scale, y * reference_scale};
    };
    draw->AddRectFilledMultiColor(minimum,
                                  maximum,
                                  rgba(ImVec4{0.022F, 0.020F, 0.024F, 1.0F}),
                                  rgba(ImVec4{0.040F, 0.023F, 0.041F, 1.0F}),
                                  rgba(ImVec4{0.020F, 0.017F, 0.022F, 1.0F}),
                                  rgba(ImVec4{0.012F, 0.011F, 0.014F, 1.0F}));
    draw->AddRectFilled(minimum, ImVec2{point(277.0F, 0.0F).x, maximum.y}, rgba(page_color), 0.0F);
    draw->AddRectFilledMultiColor(point(0.0F, 238.0F), ImVec2{point(277.0F, 0.0F).x, maximum.y},
                                  IM_COL32(18, 10, 20, 0), IM_COL32(30, 10, 29, 18),
                                  IM_COL32(0, 0, 0, 82), IM_COL32(13, 2, 13, 94));

    const ImVec2 art_min = point(277.0F, 13.0F);
    const ImVec2 art_max = point(554.0F, 312.0F);
    if (artwork != nullptr && artwork->view != nullptr && artwork->width > 0 && artwork->height > 0) {
        const ImVec2 uv_min{0.035F, 0.0F};
        const ImVec2 uv_max{0.965F, 0.80F};
        draw->AddImageRounded(reinterpret_cast<ImTextureID>(artwork->view.Get()), art_min, art_max,
                              uv_min, uv_max, IM_COL32_WHITE, 7.0F * reference_scale);
    } else {
        draw->AddRectFilled(art_min, art_max, rgba(field_color), 7.0F * reference_scale);
        draw->AddRect(art_min, art_max, rgba(field_border), 7.0F * reference_scale);
        text_at(draw, art_min + ImVec2{16.0F, 16.0F}, font_at(2), 34.0F * reference_scale, purple, "Be ahead of all.");
    }

    const ImVec2 content = point(17.0F, 19.5F);
    text_at(draw, content, font_at(1), auth_welcome_font_size * reference_scale, primary_text, "Welcome to,");
    gradient_text(draw,
                  font_at(1),
                  auth_brand_font_size * reference_scale,
                  point(17.0F, 53.5F),
                  "Serein",
                  IM_COL32(184, 49, 172, 255),
                  IM_COL32(72, 25, 70, 178));
    text_at(draw, point(17.0F, 111.0F), font_at(0), 15.0F * reference_scale, primary_text, "Authorize below.");

    const ImVec2 username_label = point(17.0F, 131.0F);
    const ImVec2 username_min = point(17.0F, 148.0F);
    const ImVec2 field_size{336.5F, 49.0F};
    text_at(draw, username_label, font_at(0), 13.0F * reference_scale, secondary_text, "Username");
    draw->AddRectFilled(username_min, username_min + field_size, rgba(field_color), 6.0F * reference_scale);
    draw->AddRect(username_min, username_min + field_size, rgba(field_border), 6.0F * reference_scale, 0, 1.0F * reference_scale);
    draw_person_icon(draw, username_min + ImVec2{20.0F, field_size.y * 0.5F}, reference_scale);

    const ImVec2 password_label = point(17.0F, 188.0F);
    const ImVec2 password_min = point(17.0F, 204.0F);
    text_at(draw, password_label, font_at(0), 13.0F * reference_scale, secondary_text, "Password");
    draw->AddRectFilled(password_min, password_min + field_size, rgba(field_color), 6.0F * reference_scale);
    draw->AddRect(password_min, password_min + field_size, rgba(field_border), 6.0F * reference_scale, 0, 1.0F * reference_scale);
    draw_key_icon(draw, password_min + ImVec2{20.0F, field_size.y * 0.5F}, reference_scale);

    const ImVec2 eye_size{31.0F, 31.0F};
    const ImVec2 eye_min = password_min + ImVec2{field_size.x - 39.0F, (field_size.y - eye_size.y) * 0.5F};
    ImGui::SetCursorScreenPos(eye_min);
    ImGui::InvisibleButton("##reveal-password", eye_size);
    const bool eye_held = ImGui::IsItemActive() && ImGui::IsItemHovered();
    const bool reveal = password_reveal_requested(eye_held, ImGui::IsMouseDown(ImGuiMouseButton_Left));
    model.set_password_reveal_held(reveal);
    draw_eye_icon(draw, eye_min + eye_size * 0.5F, reveal, reference_scale);

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4{0.0F, 0.0F, 0.0F, 0.0F});
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4{0.0F, 0.0F, 0.0F, 0.0F});
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4{0.0F, 0.0F, 0.0F, 0.0F});
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4{0.0F, 0.0F, 0.0F, 0.0F});
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{0.0F, 12.5F});
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0F);
    ImGui::PushFont(font_at(3));
    const ImVec2 input_offset{42.0F, 1.0F};
    ImGui::SetCursorScreenPos(username_min + input_offset);
    ImGui::SetNextItemWidth(field_size.x - 50.0F);
    const bool username_entered = ImGui::InputTextWithHint("##username", "", username_.data(), username_.size(), ImGuiInputTextFlags_EnterReturnsTrue);

    ImGui::SetCursorScreenPos(password_min + input_offset);
    ImGui::SetNextItemWidth(field_size.x - 91.0F);
    ImGuiInputTextFlags password_flags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (!reveal) {
        password_flags |= ImGuiInputTextFlags_Password;
    }
    const bool password_entered = ImGui::InputTextWithHint("##password", "", password_.data(), password_.size(), password_flags);
    ImGui::PopFont();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(4);

    const ImVec2 remember_min = point(17.0F, 248.0F);
    const ImVec2 check_size{19.0F, 19.0F};
    constexpr char remember_label[] = "Remember me";
    const ImVec2 remember_label_size = font_at(0)->CalcTextSizeA(13.0F * reference_scale, 1000.0F, 0.0F, remember_label);
    const float remember_width = remember_control_width(check_size.x, 8.0F, remember_label_size.x);
    ImGui::SetCursorScreenPos(remember_min);
    if (ImGui::InvisibleButton("##remember-me", ImVec2{remember_width, check_size.y})) {
        remember_me_ = !remember_me_;
    }
    const bool remember_hovered = ImGui::IsItemHovered();
    draw->AddRectFilled(remember_min, remember_min + check_size, rgba(remember_me_ ? purple : field_color), 3.0F);
    draw->AddRect(remember_min,
                  remember_min + check_size,
                  rgba(remember_me_ || remember_hovered ? purple : field_border),
                  3.0F,
                  0,
                  1.0F);
    if (remember_me_) {
        draw->AddLine(remember_min + ImVec2{4.5F, 9.5F}, remember_min + ImVec2{8.0F, 13.0F}, rgba(primary_text), 1.7F);
        draw->AddLine(remember_min + ImVec2{8.0F, 13.0F}, remember_min + ImVec2{15.0F, 5.5F}, rgba(primary_text), 1.7F);
    }
    text_at(draw, remember_min + ImVec2{27.0F, 0.0F}, font_at(0), 13.0F * reference_scale,
            remember_hovered ? primary_text : secondary_text, remember_label);

    const ImVec2 button_min = point(17.0F, 274.0F);
    const ImVec2 button_size{336.5F, 47.0F};
    ImGui::SetCursorScreenPos(button_min);
    const bool continue_pressed = ImGui::InvisibleButton("##continue", button_size);
    const bool hovered = ImGui::IsItemHovered();
    const bool held = ImGui::IsItemActive();
    const ImVec4 top = hovered ? ImVec4{0.61F, 0.17F, 0.57F, 1.0F} : purple_top;
    const ImVec4 bottom = held ? ImVec4{0.27F, 0.055F, 0.26F, 1.0F} : purple_bottom;
    draw->AddRectFilled(button_min + ImVec2{0.0F, 4.0F}, button_min + button_size + ImVec2{0.0F, 5.0F},
                        IM_COL32(50, 9, 48, 84), 6.0F * reference_scale);
    draw->AddRectFilled(button_min - ImVec2{1.5F, 1.5F}, button_min + button_size + ImVec2{1.5F, 1.5F},
                        IM_COL32(173, 49, 160, hovered ? 30 : 18), 7.0F * reference_scale);
    draw->AddRectFilled(button_min, button_min + button_size, rgba(ImVec4{0.20F, 0.15F, 0.20F, 1.0F}),
                        7.0F * reference_scale);
    draw->AddRectFilledMultiColor(button_min + ImVec2{2.0F, 2.0F}, button_min + button_size - ImVec2{2.0F, 2.0F},
                                  rgba(top), rgba(ImVec4{0.44F, 0.09F, 0.42F, 1.0F}),
                                  rgba(ImVec4{0.22F, 0.16F, 0.22F, 1.0F}), rgba(bottom));
    draw->AddLine(button_min + ImVec2{6.0F, 1.5F},
                  ImVec2{button_min.x + button_size.x - 6.0F, button_min.y + 1.5F},
                  IM_COL32(255, 177, 245, hovered ? 54 : 35), 1.0F);
    draw->AddRect(button_min, button_min + button_size, rgba(purple), 7.0F * reference_scale);
    const char* continue_label = "Continue";
    ImFont* continue_font = font_at(1);
    const float continue_size = 14.5F * reference_scale;
    const ImVec2 label_size = continue_font->CalcTextSizeA(continue_size, 1000.0F, 0.0F, continue_label);
    draw->AddText(continue_font, continue_size, button_min + (button_size - label_size) * 0.5F,
                  rgba(primary_text), continue_label);

    const bool submit_with_keyboard = username_entered || password_entered ||
                                      keyboard_intent(ImGui::IsKeyPressed(ImGuiKey_Enter), false) == KeyboardIntent::submit;
    if (continue_pressed || submit_with_keyboard) {
        model.set_credentials(username_.data(), password_.data());
        static_cast<void>(model.submit_login());
    }

    if (!model.login_error().empty()) {
        text_at(draw, point(17.0F, 323.0F), font_at(0), 12.0F * reference_scale, ImVec4{0.80F, 0.33F, 0.37F, 1.0F}, model.login_error().data());
    }

    ImGui::PopStyleVar();
}

}  // namespace serein::ui
