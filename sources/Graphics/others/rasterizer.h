#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXTex.h>

namespace Descartes
{
	class rasterizer
	{
	public:
		rasterizer(ID3D11Device* device, D3D11_FILTER sampler_filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode = D3D11_TEXTURE_ADDRESS_BORDER, DirectX::XMFLOAT4 sampler_boarder_color = DirectX::XMFLOAT4(0, 0, 0, 0));
		virtual ~rasterizer() = default;
		rasterizer(rasterizer&) = delete;
		rasterizer& operator=(rasterizer&) = delete;

	private:
		Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> embedded_vertex_shader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> embedded_input_layout;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> embedded_pixel_shaders[2];

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> embedded_rasterizer_state;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> embedded_depth_stencil_state;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> embedded_sampler_state;

	public:
		struct vertex
		{
			DirectX::XMFLOAT3 position;
			DirectX::XMFLOAT2 texcoord;
			DirectX::XMFLOAT4 color;
		};

		void blit
		(
			ID3D11DeviceContext* immediate_context,
			ID3D11ShaderResourceView* shader_resource_view,
			float dx, float dy, float dw, float dh,
			float sx, float sy, float sw, float sh,
			float angle/*degree*/,
			float r, float g, float b, float a,
			bool use_embedded_vertex_shader = true,
			bool use_embedded_pixel_shader = true,
			bool use_embedded_rasterizer_state = true,
			bool use_embedded_depth_stencil_state = true,
			bool use_embedded_sampler_state = true
		) const;
		void blit(
			ID3D11DeviceContext* immediate_context,
			ID3D11ShaderResourceView* shader_resource_view,
			float dx, float dy, float dw, float dh,
			float angle = 0/*degree*/,
			float r = 1, float g = 1, float b = 1, float a = 1,
			bool use_embedded_vertex_shader = true,
			bool use_embedded_pixel_shader = true,
			bool use_embedded_rasterizer_state = true,
			bool use_embedded_depth_stencil_state = true,
			bool use_embedded_sampler_state = true
		) const;
	};

	//'fullscreen_quad' dose not have pixel shader and sampler state. you have to make and set pixel shader and sampler state by yourself.
	class fullscreen_quad
	{
	public:
		fullscreen_quad(ID3D11Device* device);
		virtual ~fullscreen_quad() = default;
		fullscreen_quad(fullscreen_quad&) = delete;
		fullscreen_quad& operator=(fullscreen_quad&) = delete;

	private:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> embedded_vertex_shader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> embedded_pixel_shaders[2];

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> embedded_rasterizer_state;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> embedded_depth_stencil_state;

	public:
		void blit(ID3D11DeviceContext* immediate_context, bool use_embedded_rasterizer_state = true, bool use_embedded_depth_stencil_state = true, bool use_embedded_pixel_shader = false, bool enable_msaa = false);
	};
}
