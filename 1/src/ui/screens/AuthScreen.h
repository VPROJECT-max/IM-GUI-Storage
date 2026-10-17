#pragma once

#include "app/AppModel.h"
#include "render/TextureStore.h"

#include <imgui.h>

#include <array>

namespace serein::ui {

class AuthScreen {
public:
    AuthScreen();
    void render(AppModel& model, const Texture* artwork, ImVec2 minimum, ImVec2 maximum, float time_seconds, float alpha);

private:
    std::array<char, 64> username_{};
    std::array<char, 64> password_{};
    bool initialized_from_model_{false};
};

}  // namespace serein::ui
