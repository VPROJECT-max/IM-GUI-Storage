#include "app/Configuration.h"
#include "platform/FrameCapture.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>

int main() {
    constexpr auto names = serein::capture_file_names();
    static_assert(names.size() == 6U);
    static_assert(names[0] == L"login.png");
    static_assert(names[1] == L"products-loading.png");
    static_assert(names[2] == L"products.png");
    static_assert(names[3] == L"selected.png");
    static_assert(names[4] == L"product-loading.png");
    static_assert(names[5] == L"product-success.png");

    const std::filesystem::path source_root{SEREIN_SOURCE_DIR};
    const auto config = serein::make_default_config();
    if (!std::filesystem::exists(source_root / config.assets.background)) {
        std::cerr << "Configured background asset is missing.\n";
        return EXIT_FAILURE;
    }
    if (config.assets.promotional != "Assets/Serein Hazmat Warrior Thumbnail.png" ||
        !std::filesystem::exists(source_root / config.assets.promotional)) {
        std::cerr << "Configured login artwork is missing or incorrect.\n";
        return EXIT_FAILURE;
    }
    for (const auto& product : config.products) {
        if (!std::filesystem::exists(source_root / product.image_path)) {
            std::cerr << "Configured product asset is missing: " << product.image_path << '\n';
            return EXIT_FAILURE;
        }
    }
    std::cout << "Serein asset and capture contracts passed.\n";
    return EXIT_SUCCESS;
}
