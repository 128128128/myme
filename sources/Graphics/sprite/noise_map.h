#pragma once

#include <d3d11.h>
#include <directxmath.h>

#include <wrl.h>
#include <string>

#include "../../Input/Input.h"

class noise_map
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> noise_buffer;

	D3D11_TEXTURE2D_DESC texture2d_desc;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_material_noise;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;

	float num_noise = 0;
	float sum_octaves = 4.0f;
	float num_amplitude = 0.5f;
	float n_param = 32.0f;
	DirectX::XMFLOAT2 mat0_param= { 1.6f, 1.2f };
	DirectX::XMFLOAT2 mat1_param = { -1.2f, 1.6f };
public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

	struct noise
	{
		float kind_of_noise;
		float octaves;
		float amplitude;
		float param;
		DirectX::XMFLOAT2 mat0;
		DirectX::XMFLOAT2 mat1;
	};

	noise_map(ID3D11Device* device, const char* filename, bool force_srgb, bool enable_caching, D3D11_FILTER sampler_filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode = D3D11_TEXTURE_ADDRESS_BORDER, DirectX::XMFLOAT4 sampler_boarder_color = DirectX::XMFLOAT4(0, 0, 0, 0));

	virtual ~noise_map() = default;
	noise_map(noise_map&) = delete;
	noise_map& operator =(noise_map&) = delete;

	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/, float sx, float sy, float sw, float sh);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh);
	void textout(ID3D11DeviceContext* immediate_context, std::string s, float x, float y, float w, float h, float r, float g, float b, float a);
	void DebugDrawGUI();


};


