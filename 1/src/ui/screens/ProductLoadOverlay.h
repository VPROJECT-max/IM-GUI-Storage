#pragma once

#include "app/AppModel.h"

#include <imgui.h>

namespace serein::ui {

void draw_product_load_overlay(const AppModel& model, ImVec2 display_size, float time_seconds, float alpha);

}  // namespace serein::ui
