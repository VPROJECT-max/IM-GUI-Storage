#include "platform/Win32Application.h"
#include "render/D3D11Renderer.h"
#include "render/TextureStore.h"

#include <cstdlib>
#include <cstddef>
#include <iostream>
#include <span>
#include <type_traits>
#include <vector>

int main() {
    static_assert(serein::Win32Application::client_width == 806);
    static_assert(serein::Win32Application::client_height == 468);
    static_assert(!std::is_copy_constructible_v<serein::D3D11Renderer>);
    static_assert(!std::is_copy_assignable_v<serein::D3D11Renderer>);

    if (!serein::TextureStore::supports_extension(L".jpg") ||
        !serein::TextureStore::supports_extension(L".jpeg") ||
        !serein::TextureStore::supports_extension(L".png") ||
        serein::TextureStore::supports_extension(L".webp")) {
        std::cerr << "Texture extension contract failed.\n";
        return EXIT_FAILURE;
    }

    const std::vector<std::byte> pixels{
        std::byte{255}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{0}, std::byte{0}, std::byte{255},
        std::byte{0}, std::byte{0}, std::byte{255}, std::byte{255},
    };
    const auto blurred = serein::blur_rgba_pixels(pixels, 3U, 1U, 1U);
    if (blurred.size() != pixels.size() ||
        std::to_integer<unsigned int>(blurred[4]) == 0U ||
        std::to_integer<unsigned int>(blurred[6]) == 0U ||
        std::to_integer<unsigned int>(blurred[7]) != 255U) {
        std::cerr << "Blurred companion texture contract failed.\n";
        return EXIT_FAILURE;
    }
    std::cout << "Serein platform contracts passed.\n";
    return EXIT_SUCCESS;
}
