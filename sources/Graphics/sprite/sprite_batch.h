#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

#include <vector>

class sprite_batch
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
	D3D11_TEXTURE2D_DESC texture2d_desc;

	const size_t MAX_INSTANCES = 256;
	struct instance
	{
		DirectX::XMFLOAT4X4 ndc_transform;
		DirectX::XMFLOAT4 texcoord_transform;
		DirectX::XMFLOAT4 color;
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> instance_buffer;
public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
	};

public:
	sprite_batch(ID3D11Device* device, const char* filename, size_t max_instance, bool force_srgb, bool enable_caching, D3D11_FILTER sampler_filter = D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode = D3D11_TEXTURE_ADDRESS_BORDER, DirectX::XMFLOAT4 sampler_boarder_colour = DirectX::XMFLOAT4(0, 0, 0, 0));
	~sprite_batch() = default;
	sprite_batch(sprite_batch&) = delete;
	sprite_batch& operator =(sprite_batch&) = delete;

	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/, float sx, float sy, float sw, float sh);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh);

	void begin(ID3D11DeviceContext* immediate_context, bool use_embedded_vertex_shader = true, bool use_embedded_pixel_shader = true,
		bool use_embedded_rasterizer_state = true, bool use_embedded_depth_stencil_state = true, bool use_embedded_sampler_state = true);
	void end(ID3D11DeviceContext* immediate_context);

private:
	static void rotate(float& x, float& y, float cx, float cy, float sin, float cos)
	{
		x -= cx;
		y -= cy;

		float tx{ x }, ty{ y };
		x = cos * tx + -sin * ty;
		y = sin * tx + cos * ty;

		x += cx;
		y += cy;
	}
	D3D11_VIEWPORT viewport;

	size_t count_instance = 0;
	instance* instances = 0;

	bool using_embedded_rasterizer_state = false;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> embedded_rasterizer_state;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> default_rasterizer_state;

	bool using_embedded_depth_stencil_state = false;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> embedded_depth_stencil_state;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> default_depth_stencil_state;

	bool using_embedded_sampler_state = false;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> embedded_sampler_state;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> default_sampler_state;
};




