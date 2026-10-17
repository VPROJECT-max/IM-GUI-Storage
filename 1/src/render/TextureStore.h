#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace serein {

struct Texture {
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blurred_view;
    int width{0};
    int height{0};
};

[[nodiscard]] std::vector<std::byte> blur_rgba_pixels(std::span<const std::byte> pixels,
                                                       unsigned int width,
                                                       unsigned int height,
                                                       unsigned int radius);

class TextureStore {
public:
    [[nodiscard]] static bool supports_extension(std::wstring_view extension);
    [[nodiscard]] const Texture* load(ID3D11Device* device, const std::filesystem::path& path);
    [[nodiscard]] const Texture* find(const std::filesystem::path& path) const;
    void clear() noexcept;

private:
    std::unordered_map<std::wstring, Texture> textures_;
};

}  // namespace serein
