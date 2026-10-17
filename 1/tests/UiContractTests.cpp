#include "ui/UiContracts.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

int failures = 0;

void expect(const bool value, const char* message) {
    if (!value) {
        std::cerr << "FAIL: " << message << '\n';
        ++failures;
    }
}

}  // namespace

int main() {
    using namespace serein::ui;
    expect(auth_welcome_font_size == 31.5F, "login heading uses the measured reference scale");
    expect(auth_brand_font_size == 26.0F, "Serein brand line uses the requested larger display size");
    expect(auth_brand_y == 48.5F, "Serein brand line uses the requested higher baseline");
    expect(button_text_size == 18.5F, "button labels use a stronger readable size");
    expect(discovery_loading_font_size == 21.0F, "discovery loading copy matches the reference proportion");
    expect(button_corner_radius == 8.0F, "buttons use the slightly rounder final radius");
    expect(button_shadow_alpha == 46, "button shadow stays dark and restrained");
    expect(button_focus_alpha == 0.38F, "button focus remains visible without a neon halo");
    expect(!password_reveal_requested(false, false), "idle eye stays hidden");
    expect(!password_reveal_requested(true, false), "hover alone stays hidden");
    expect(!password_reveal_requested(false, true), "mouse down away from eye stays hidden");
    expect(password_reveal_requested(true, true), "active hold over eye reveals password");

    expect(keyboard_intent(true, false) == KeyboardIntent::submit, "Enter submits focused form");
    expect(keyboard_intent(false, true) == KeyboardIntent::cancel, "Escape cancels overlay");
    expect(keyboard_intent(false, false) == KeyboardIntent::none, "idle keyboard has no intent");

    const float full = product_grid_width(738.0F, 274.0F, 0.0F);
    const float compressed = product_grid_width(738.0F, 274.0F, 1.0F);
    expect(std::abs(full - 738.0F) < 0.001F, "grid uses full width without selection");
    expect(std::abs(compressed - 452.0F) < 0.001F, "grid leaves a 12px inspector gutter");
    expect(product_grid_width(300.0F, 274.0F, 1.0F) >= 240.0F, "narrow grid preserves minimum card access");

    const auto panel = selected_panel_vertical_layout(296.0F);
    expect(panel.description_y + 16.0F <= panel.settings_y, "description does not intersect Settings");
    expect(panel.settings_y + panel.settings_height + 8.0F <= panel.load_y, "secondary and primary actions keep an 8px gap");
    expect(panel.load_y + panel.load_height <= 288.0F, "primary action stays inside the panel inset");

    expect(background_dim_alpha(true) > background_dim_alpha(false), "deep-background setting increases the cinematic scrim");
    expect(selection_border_width(true, true) > selection_border_width(true, false), "strong-selection setting visibly emphasizes the selected card");
    expect(selection_border_width(false, true) == 1.0F, "unselected cards keep a restrained border");
    expect(std::abs(remember_control_width(19.0F, 8.0F, 82.0F) - 109.0F) < 0.001F,
           "Remember Me hit target includes the checkbox, gap, and complete label");

    const auto cards = product_card_layout(806.0F, 468.0F);
    expect(cards[0].x == 18.0F && cards[0].y == 12.0F, "product grid uses the scaled reference inset");
    expect(cards[0].width == cards[1].width && cards[0].height == cards[2].height, "product cards use equal 2x2 dimensions");
    expect(cards[1].x + cards[1].width == 788.0F, "right cards stay inside the compact client width");
    expect(cards[2].y + cards[2].height == 456.0F, "bottom cards stay inside the compact client height");

    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
