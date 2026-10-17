#include "render/D3D11Renderer.h"

#include <dxgi.h>
#include <d3dcompiler.h>
#include <wincodec.h>

#include <array>
#include <cstring>
#include <iterator>

namespace serein {

D3D11Renderer::~D3D11Renderer() {
    shutdown();
}

bool D3D11Renderer::initialize(HWND window) {
    DXGI_SWAP_CHAIN_DESC swap_desc{};
    swap_desc.BufferCount = 2;
    swap_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swap_desc.OutputWindow = window;
    swap_desc.SampleDesc.Count = 1;
    swap_desc.Windowed = TRUE;
    swap_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL requested_levels[]{D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    D3D_FEATURE_LEVEL acquired_level{};
    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        requested_levels,
        static_cast<UINT>(std::size(requested_levels)),
        D3D11_SDK_VERSION,
        &swap_desc,
        swap_chain_.GetAddressOf(),
        device_.GetAddressOf(),
        &acquired_level,
        context_.GetAddressOf());

#if defined(_DEBUG)
    if (FAILED(result)) {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        result = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            requested_levels,
            static_cast<UINT>(std::size(requested_levels)),
            D3D11_SDK_VERSION,
            &swap_desc,
            swap_chain_.GetAddressOf(),
            device_.GetAddressOf(),
            &acquired_level,
            context_.GetAddressOf());
    }
#endif

    return SUCCEEDED(result) && create_render_target() && create_blur_resources();
}

void D3D11Renderer::shutdown() noexcept {
    blur_rasterizer_state_.Reset();
    blur_blend_state_.Reset();
    blur_sampler_.Reset();
    blur_constants_.Reset();
    blur_pixel_shader_.Reset();
    blur_vertex_shader_.Reset();
    blur_intermediate_view_.Reset();
    blur_intermediate_target_.Reset();
    blur_intermediate_texture_.Reset();
    blur_source_view_.Reset();
    blur_source_texture_.Reset();
    render_target_.Reset();
    swap_chain_.Reset();
    context_.Reset();
    device_.Reset();
}

void D3D11Renderer::begin_frame(const float clear_color[4]) const noexcept {
    ID3D11RenderTargetView* target = render_target_.Get();
    context_->OMSetRenderTargets(1, &target, nullptr);
    context_->ClearRenderTargetView(target, clear_color);
}

void D3D11Renderer::blur_back_buffer(const float strength) const noexcept {
    if (strength <= 0.001F || !blur_source_texture_ || !blur_intermediate_target_) {
        return;
    }

    ID3D11RenderTargetView* no_target = nullptr;
    context_->OMSetRenderTargets(0, &no_target, nullptr);
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())))) {
        ID3D11RenderTargetView* target = render_target_.Get();
        context_->OMSetRenderTargets(1, &target, nullptr);
        return;
    }
    context_->CopyResource(blur_source_texture_.Get(), back_buffer.Get());

    context_->IASetInputLayout(nullptr);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->RSSetState(blur_rasterizer_state_.Get());
    context_->RSSetViewports(1, &blur_viewport_);
    const float blend_factor[4]{};
    context_->OMSetBlendState(blur_blend_state_.Get(), blend_factor, 0xFFFFFFFFU);
    context_->OMSetDepthStencilState(nullptr, 0);
    context_->VSSetShader(blur_vertex_shader_.Get(), nullptr, 0);
    context_->PSSetShader(blur_pixel_shader_.Get(), nullptr, 0);
    ID3D11SamplerState* sampler = blur_sampler_.Get();
    context_->PSSetSamplers(0, 1, &sampler);
    ID3D11Buffer* constants = blur_constants_.Get();
    context_->PSSetConstantBuffers(0, 1, &constants);

    struct BlurConstants {
        float texel_x;
        float texel_y;
        float radius;
        float vertical;
    } values{1.0F / blur_viewport_.Width, 1.0F / blur_viewport_.Height, 3.8F * strength, 0.0F};

    ID3D11ShaderResourceView* source = blur_source_view_.Get();
    ID3D11RenderTargetView* intermediate = blur_intermediate_target_.Get();
    context_->OMSetRenderTargets(1, &intermediate, nullptr);
    context_->UpdateSubresource(blur_constants_.Get(), 0, nullptr, &values, 0, 0);
    context_->PSSetShaderResources(0, 1, &source);
    context_->Draw(3, 0);

    ID3D11ShaderResourceView* no_view = nullptr;
    context_->PSSetShaderResources(0, 1, &no_view);
    values.vertical = 1.0F;
    context_->UpdateSubresource(blur_constants_.Get(), 0, nullptr, &values, 0, 0);
    ID3D11RenderTargetView* target = render_target_.Get();
    context_->OMSetRenderTargets(1, &target, nullptr);
    ID3D11ShaderResourceView* intermediate_view = blur_intermediate_view_.Get();
    context_->PSSetShaderResources(0, 1, &intermediate_view);
    context_->Draw(3, 0);
    context_->PSSetShaderResources(0, 1, &no_view);
}

void D3D11Renderer::present() const noexcept {
    swap_chain_->Present(1, 0);
}

bool D3D11Renderer::capture_png(const std::filesystem::path& path) const {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())))) {
        return false;
    }
    D3D11_TEXTURE2D_DESC description{};
    back_buffer->GetDesc(&description);
    D3D11_TEXTURE2D_DESC staging_description = description;
    staging_description.Usage = D3D11_USAGE_STAGING;
    staging_description.BindFlags = 0;
    staging_description.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    staging_description.MiscFlags = 0;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> staging;
    if (FAILED(device_->CreateTexture2D(&staging_description, nullptr, staging.GetAddressOf()))) {
        return false;
    }
    context_->CopyResource(staging.Get(), back_buffer.Get());
    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (FAILED(context_->Map(staging.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
        return false;
    }

    Microsoft::WRL::ComPtr<IWICImagingFactory> factory;
    Microsoft::WRL::ComPtr<IWICStream> stream;
    Microsoft::WRL::ComPtr<IWICBitmapEncoder> encoder;
    Microsoft::WRL::ComPtr<IWICBitmapFrameEncode> frame;
    Microsoft::WRL::ComPtr<IWICBitmap> bitmap;
    Microsoft::WRL::ComPtr<IPropertyBag2> properties;
    HRESULT result = CoCreateInstance(CLSID_WICImagingFactory2, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(factory.GetAddressOf()));
    if (SUCCEEDED(result)) {
        result = factory->CreateBitmapFromMemory(description.Width,
                                                 description.Height,
                                                 GUID_WICPixelFormat32bppRGBA,
                                                 mapped.RowPitch,
                                                 mapped.RowPitch * description.Height,
                                                 static_cast<BYTE*>(mapped.pData),
                                                 bitmap.GetAddressOf());
    }
    if (SUCCEEDED(result)) {
        result = factory->CreateStream(stream.GetAddressOf());
    }
    if (SUCCEEDED(result)) {
        result = stream->InitializeFromFilename(path.c_str(), GENERIC_WRITE);
    }
    if (SUCCEEDED(result)) {
        result = factory->CreateEncoder(GUID_ContainerFormatPng, nullptr, encoder.GetAddressOf());
    }
    if (SUCCEEDED(result)) {
        result = encoder->Initialize(stream.Get(), WICBitmapEncoderNoCache);
    }
    if (SUCCEEDED(result)) {
        result = encoder->CreateNewFrame(frame.GetAddressOf(), properties.GetAddressOf());
    }
    if (SUCCEEDED(result)) {
        result = frame->Initialize(properties.Get());
    }
    if (SUCCEEDED(result)) {
        result = frame->SetSize(description.Width, description.Height);
    }
    if (SUCCEEDED(result)) {
        result = frame->WriteSource(bitmap.Get(), nullptr);
    }
    if (SUCCEEDED(result)) {
        result = frame->Commit();
    }
    if (SUCCEEDED(result)) {
        result = encoder->Commit();
    }
    context_->Unmap(staging.Get(), 0);
    return SUCCEEDED(result);
}

ID3D11Device* D3D11Renderer::device() const noexcept {
    return device_.Get();
}

ID3D11DeviceContext* D3D11Renderer::context() const noexcept {
    return context_.Get();
}

bool D3D11Renderer::create_render_target() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())))) {
        return false;
    }
    return SUCCEEDED(device_->CreateRenderTargetView(back_buffer.Get(), nullptr, render_target_.GetAddressOf()));
}

bool D3D11Renderer::create_blur_resources() {
    Microsoft::WRL::ComPtr<ID3D11Texture2D> back_buffer;
    if (FAILED(swap_chain_->GetBuffer(0, IID_PPV_ARGS(back_buffer.GetAddressOf())))) {
        return false;
    }
    D3D11_TEXTURE2D_DESC texture_desc{};
    back_buffer->GetDesc(&texture_desc);
    texture_desc.Usage = D3D11_USAGE_DEFAULT;
    texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    texture_desc.CPUAccessFlags = 0;
    texture_desc.MiscFlags = 0;
    texture_desc.MipLevels = 1;
    texture_desc.ArraySize = 1;
    texture_desc.SampleDesc.Count = 1;
    texture_desc.SampleDesc.Quality = 0;
    if (FAILED(device_->CreateTexture2D(&texture_desc, nullptr, blur_source_texture_.GetAddressOf())) ||
        FAILED(device_->CreateShaderResourceView(blur_source_texture_.Get(), nullptr, blur_source_view_.GetAddressOf()))) {
        return false;
    }

    texture_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(device_->CreateTexture2D(&texture_desc, nullptr, blur_intermediate_texture_.GetAddressOf())) ||
        FAILED(device_->CreateRenderTargetView(blur_intermediate_texture_.Get(), nullptr, blur_intermediate_target_.GetAddressOf())) ||
        FAILED(device_->CreateShaderResourceView(blur_intermediate_texture_.Get(), nullptr, blur_intermediate_view_.GetAddressOf()))) {
        return false;
    }

    constexpr char shader_source[] = R"(
cbuffer BlurConstants : register(b0) {
    float2 TexelSize;
    float Radius;
    float Vertical;
};
Texture2D Scene : register(t0);
SamplerState LinearClamp : register(s0);
struct VertexOutput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };
VertexOutput VSMain(uint id : SV_VertexID) {
    VertexOutput output;
    float2 p = float2((id << 1) & 2, id & 2);
    output.uv = p;
    output.position = float4(p.x * 2.0f - 1.0f, 1.0f - p.y * 2.0f, 0.0f, 1.0f);
    return output;
}
float4 PSMain(VertexOutput input) : SV_TARGET {
    float2 axis = Vertical > 0.5f ? float2(0.0f, 1.0f) : float2(1.0f, 0.0f);
    float2 stepUv = TexelSize * axis * Radius;
    float4 color = Scene.SampleLevel(LinearClamp, input.uv, 0) * 0.18f;
    color += Scene.SampleLevel(LinearClamp, input.uv + stepUv, 0) * 0.15f;
    color += Scene.SampleLevel(LinearClamp, input.uv - stepUv, 0) * 0.15f;
    color += Scene.SampleLevel(LinearClamp, input.uv + stepUv * 2.0f, 0) * 0.12f;
    color += Scene.SampleLevel(LinearClamp, input.uv - stepUv * 2.0f, 0) * 0.12f;
    color += Scene.SampleLevel(LinearClamp, input.uv + stepUv * 3.0f, 0) * 0.09f;
    color += Scene.SampleLevel(LinearClamp, input.uv - stepUv * 3.0f, 0) * 0.09f;
    color += Scene.SampleLevel(LinearClamp, input.uv + stepUv * 4.0f, 0) * 0.05f;
    color += Scene.SampleLevel(LinearClamp, input.uv - stepUv * 4.0f, 0) * 0.05f;
    return color;
}
)";
    Microsoft::WRL::ComPtr<ID3DBlob> vertex_bytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> pixel_bytecode;
    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    if (FAILED(D3DCompile(shader_source, sizeof(shader_source) - 1, nullptr, nullptr, nullptr, "VSMain", "vs_4_0",
                          D3DCOMPILE_ENABLE_STRICTNESS, 0, vertex_bytecode.GetAddressOf(), errors.GetAddressOf())) ||
        FAILED(D3DCompile(shader_source, sizeof(shader_source) - 1, nullptr, nullptr, nullptr, "PSMain", "ps_4_0",
                          D3DCOMPILE_ENABLE_STRICTNESS, 0, pixel_bytecode.GetAddressOf(), errors.ReleaseAndGetAddressOf())) ||
        FAILED(device_->CreateVertexShader(vertex_bytecode->GetBufferPointer(), vertex_bytecode->GetBufferSize(), nullptr,
                                           blur_vertex_shader_.GetAddressOf())) ||
        FAILED(device_->CreatePixelShader(pixel_bytecode->GetBufferPointer(), pixel_bytecode->GetBufferSize(), nullptr,
                                          blur_pixel_shader_.GetAddressOf()))) {
        return false;
    }

    D3D11_BUFFER_DESC buffer_desc{};
    buffer_desc.ByteWidth = 16;
    buffer_desc.Usage = D3D11_USAGE_DEFAULT;
    buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(device_->CreateBuffer(&buffer_desc, nullptr, blur_constants_.GetAddressOf()))) {
        return false;
    }

    D3D11_SAMPLER_DESC sampler_desc{};
    sampler_desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
    if (FAILED(device_->CreateSamplerState(&sampler_desc, blur_sampler_.GetAddressOf()))) {
        return false;
    }

    D3D11_BLEND_DESC blend_desc{};
    blend_desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(device_->CreateBlendState(&blend_desc, blur_blend_state_.GetAddressOf()))) {
        return false;
    }
    D3D11_RASTERIZER_DESC rasterizer_desc{};
    rasterizer_desc.FillMode = D3D11_FILL_SOLID;
    rasterizer_desc.CullMode = D3D11_CULL_NONE;
    rasterizer_desc.DepthClipEnable = TRUE;
    rasterizer_desc.ScissorEnable = FALSE;
    if (FAILED(device_->CreateRasterizerState(&rasterizer_desc, blur_rasterizer_state_.GetAddressOf()))) {
        return false;
    }

    blur_viewport_.TopLeftX = 0.0F;
    blur_viewport_.TopLeftY = 0.0F;
    blur_viewport_.Width = static_cast<float>(texture_desc.Width);
    blur_viewport_.Height = static_cast<float>(texture_desc.Height);
    blur_viewport_.MinDepth = 0.0F;
    blur_viewport_.MaxDepth = 1.0F;
    return true;
}

}  // namespace serein
