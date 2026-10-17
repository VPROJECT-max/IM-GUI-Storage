#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <filesystem>

namespace serein {

class D3D11Renderer {
public:
    D3D11Renderer() = default;
    ~D3D11Renderer();

    D3D11Renderer(const D3D11Renderer&) = delete;
    D3D11Renderer& operator=(const D3D11Renderer&) = delete;
    D3D11Renderer(D3D11Renderer&&) = delete;
    D3D11Renderer& operator=(D3D11Renderer&&) = delete;

    [[nodiscard]] bool initialize(HWND window);
    void shutdown() noexcept;
    void begin_frame(const float clear_color[4]) const noexcept;
    void blur_back_buffer(float strength) const noexcept;
    void present() const noexcept;
    [[nodiscard]] bool capture_png(const std::filesystem::path& path) const;

    [[nodiscard]] ID3D11Device* device() const noexcept;
    [[nodiscard]] ID3D11DeviceContext* context() const noexcept;

private:
    [[nodiscard]] bool create_render_target();
    [[nodiscard]] bool create_blur_resources();

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGISwapChain> swap_chain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> render_target_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> blur_source_texture_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blur_source_view_;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> blur_intermediate_texture_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> blur_intermediate_target_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> blur_intermediate_view_;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> blur_vertex_shader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> blur_pixel_shader_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> blur_constants_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> blur_sampler_;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blur_blend_state_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> blur_rasterizer_state_;
    D3D11_VIEWPORT blur_viewport_{};
};

}  // namespace serein
