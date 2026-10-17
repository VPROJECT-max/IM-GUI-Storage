#pragma once

#include "ui/screens/ProductScreen.h"

#include <imgui.h>

namespace serein::ui {

[[nodiscard]] bool draw_settings_overlay(SettingsState& state, ImVec2 display_size, float alpha);

}  // namespace serein::ui
