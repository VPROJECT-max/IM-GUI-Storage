#pragma once

#include "app/Product.h"

#include <cstdint>
#include <string>
#include <vector>

namespace serein {

struct AssetPaths {
    std::string background;
    std::string promotional;
};

struct ThemeConfig {
    std::uint32_t accent_rgba{0x62D6C6FFU};
};

struct AppConfig {
    std::string application_name;
    std::string version;
    AssetPaths assets;
    ThemeConfig theme;
    std::vector<Product> products;
    float product_discovery_seconds{3.0F};
};

[[nodiscard]] AppConfig make_default_config();

}  // namespace serein
