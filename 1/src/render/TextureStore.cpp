#include "render/TextureStore.h"

#include <wincodec.h>

#include <algorithm>
#include <cstddef>
#include <cwctype>
#include <vector>

namespace serein {
namespace {

std::wstring lower_extension(std::wstring_view extension) {
    std::wstring result{extension};
    std::ranges::transform(result, result.begin(), [](const wchar_t value) {
        return static_cast<wchar_t>(std::towlower(value));
    });
    return result;
}

}  // namespace

std::vector<std::byte> blur_rgba_pixels(const std::span<const std::byte> pixels,
                                        const unsigned int width,
                                        const unsigned int height,
                                        const unsigned int radius) {
    const std::size_t expected = static_cast<std::size_t>(width) * height * 4U;
    if (pixels.size() != expected || width == 0U || height == 0U || radius == 0U) {
        return std::vector<std::byte>{pixels.begin(), pixels.end()};
    }

    const auto channel = [](const std::span<const std::byte> source,
                            const unsigned int image_width,
                            const unsigned int x,
                            const unsigned int y,
                            const unsigned int component) {
        const std::size_t index = (static_cast<std::size_t>(y) * image_width + x) * 4U + component;
        return std::to_integer<unsigned int>(source[index]);
    };
    const unsigned int kernel = radius * 2U + 1U;
    std::vector<std::byte> horizontal(expected);
    std::vector<std::byte> blurred(expected);

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int component = 0; component < 4U; ++component) {
            unsigned int sum = 0U;
            for (int offset = -static_cast<int>(radius); offset <= static_cast<int>(radius); ++offset) {
                const auto sample_x = static_cast<unsigned int>(std::clamp(offset, 0, static_cast<int>(width) - 1));
                sum += channel(pixels, width, sample_x, y, component);
            }
            for (unsigned int x = 0; x < width; ++x) {
                horizontal[(static_cast<std::size_t>(y) * width + x) * 4U + component] =
                    static_cast<std::byte>(sum / kernel);
                const auto remove_x = static_cast<unsigned int>(std::clamp(static_cast<int>(x) - static_cast<int>(radius), 0, static_cast<int>(width) - 1));
                const auto add_x = static_cast<unsigned int>(std::clamp(static_cast<int>(x) + static_cast<int>(radius) + 1, 0, static_cast<int>(width) - 1));
                sum -= channel(pixels, width, remove_x, y, component);
                sum += channel(pixels, width, add_x, y, component);
            }
        }
    }

    const std::span<const std::byte> horizontal_span{horizontal};
    for (unsigned int x = 0; x < width; ++x) {
        for (unsigned int component = 0; component < 4U; ++component) {
            unsigned int sum = 0U;
            for (int offset = -static_cast<int>(radius); offset <= static_cast<int>(radius); ++offset) {
                const auto sample_y = static_cast<unsigned int>(std::clamp(offset, 0, static_cast<int>(height) - 1));
                sum += channel(horizontal_span, width, x, sample_y, component);
            }
            for (unsigned int y = 0; y < height; ++y) {
                blurred[(static_cast<std::size_t>(y) * width + x) * 4U + component] =
                    static_cast<std::byte>(sum / kernel);
                const auto remove_y = static_cast<unsigned int>(std::clamp(static_cast<int>(y) - static_cast<int>(radius), 0, static_cast<int>(height) - 1));
                const auto add_y = static_cast<unsigned int>(std::clamp(static_cast<int>(y) + static_cast<int>(radius) + 1, 0, static_cast<int>(height) - 1));
                sum -= channel(horizontal_span, width, x, remove_y, component);
                sum += channel(horizontal_span, width, x, add_y, component);
            }
        }
    }
    return blurred;
}

bool TextureStore::supports_extension(const std::wstring_view extension) {
    const auto lower = lower_extension(extension);
    return lower == L".jpg" || lower == L".jpeg" || lower == L".png" || lower == L".bmp" || lower == L".tif" || lower == L".tiff";
}

const Texture* TextureStore::load(ID3D11Device* device, const std::filesystem::path& path) {
    if (device == nullptr || !supports_extension(path.extension().wstring())) {
        return nullptr;
    }
    const auto key = path.lexically_normal().wstring();
    if (const auto existing = textures_.find(key); existing != textures_.end()) {
        return &existing->second;
    }

    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    HRESULT result = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
    if (FAILED(result)) {
        result = CoCreateInstance(CLSID_WICImagingFactory1, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
    }
    if (FAILED(result)) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IWICBitmapDecoder> decoder;
    result = factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decoder.GetAddressOf());
    if (FAILED(result)) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
    if (FAILED(decoder->GetFrame(0, frame.GetAddressOf()))) {
        return nullptr;
    }

    UINT width = 0;
    UINT height = 0;
    if (FAILED(frame->GetSize(&width, &height)) || width == 0 || height == 0) {
        return nullptr;
    }

    Microsoft::WRL::ComPtr<IWICFormatConverter> converter;
    if (FAILED(factory->CreateFormatConverter(converter.GetAddressOf())) ||
        FAILED(converter->Initialize(frame.Get(), GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom))) {
        return nullptr;
    }

    const UINT row_pitch = width * 4U;
    std::vector<std::byte> pixels(static_cast<std::size_t>(row_pitch) * height);
    if (FAILED(converter->CopyPixels(nullptr, row_pitch, static_cast<UINT>(pixels.size()), reinterpret_cast<BYTE*>(pixels.data())))) {
        return nullptr;
    }

    D3D11_TEXTURE2D_DESC texture_desc{};
    texture_desc.Width = width;
    texture_desc.Height = height;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    const D3D11_SUBRESOURCE_DATA texture_data{pixels.data(), row_pitch, 0};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device->CreateTexture2D(&texture_desc, &texture_data, texture.GetAddressOf()))) {
        return nullptr;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC view_desc{};
    view_desc.Format = texture_desc.Format;
    view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    view_desc.Texture2D.MipLevels = 1;

    Texture loaded;
    loaded.width = static_cast<int>(width);
    loaded.height = static_cast<int>(height);
    if (FAILED(device->CreateShaderResourceView(texture.Get(), &view_desc, loaded.view.GetAddressOf()))) {
        return nullptr;
    }

    constexpr unsigned int blur_radius = 10U;
    const auto blurred_pixels = blur_rgba_pixels(pixels, width, height, blur_radius);
    const D3D11_SUBRESOURCE_DATA blurred_data{blurred_pixels.data(), row_pitch, 0};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> blurred_texture;
    if (SUCCEEDED(device->CreateTexture2D(&texture_desc, &blurred_data, blurred_texture.GetAddressOf()))) {
        static_cast<void>(device->CreateShaderResourceView(blurred_texture.Get(), &view_desc, loaded.blurred_view.GetAddressOf()));
    }
    if (loaded.blurred_view == nullptr) {
        loaded.blurred_view = loaded.view;
    }

    const auto [position, inserted] = textures_.emplace(key, std::move(loaded));
    return inserted ? &position->second : nullptr;
}

const Texture* TextureStore::find(const std::filesystem::path& path) const {
    const auto existing = textures_.find(path.lexically_normal().wstring());
    return existing == textures_.end() ? nullptr : &existing->second;
}

void TextureStore::clear() noexcept {
    textures_.clear();
}

}  // namespace serein
