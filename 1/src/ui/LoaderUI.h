#pragma once

#include "app/AppModel.h"
#include "render/TextureStore.h"
#include "ui/CompletionExitSequence.h"
#include "ui/screens/AuthScreen.h"
#include "ui/screens/ProductScreen.h"

#include <d3d11.h>
#include <imgui.h>
#include <windows.h>

#include <array>
#include <filesystem>

namespace serein::ui {

struct LoaderFrameResult {
    float blur_strength{0.0F};
    float window_opacity{1.0F};
    bool close_requested{false};
};

class LoaderUI {
public:
    LoaderUI(AppModel& model,
             TextureStore& textures,
             ID3D11Device* device,
             std::filesystem::path asset_root);

    [[nodiscard]] LoaderFrameResult render(HWND window, float delta_seconds);

private:
    void load_assets();

    AppModel& model_;
    TextureStore& textures_;
    ID3D11Device* device_{};
    std::filesystem::path asset_root_;
    const Texture* promotional_art_{};
    std::array<const Texture*, 4> product_art_{};
    AuthScreen auth_screen_;
    ProductScreen product_screen_;
    float time_seconds_{0.0F};
    float auth_alpha_{1.0F};
    float products_alpha_{0.0F};
    float blur_alpha_{0.0F};
    CompletionExitSequence completion_exit_;
};

}  // namespace serein::ui
