#pragma once

#include "app/AppModel.h"

#include <imgui.h>

namespace serein::ui {

enum class SelectedAction {
    none,
    close,
    settings,
    load,
};

[[nodiscard]] SelectedAction draw_selected_product_panel(const Product& product,
                                                         std::string_view current_user,
                                                         ImVec2 minimum,
                                                         ImVec2 maximum,
                                                         float alpha,
                                                         float time_seconds);

}  // namespace serein::ui
