#pragma once

#include <d3d11.h>
#include <directxmath.h>

#include <wrl.h>
#include <string>

#include "../../Input/Input.h"

class playshader
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> forplay_buffer;

	D3D11_TEXTURE2D_DESC texture2d_desc;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_material_noise;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;

public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

	struct forplay
	{
		DirectX::XMFLOAT4 mouse;//xy..coord, zw..push buttom
		float coverage = 0.5f;//â_ÇÃñßèWìx
		float rain = 0.0f;//âJ
		float cloud_speed = 0.07f;//â_ÇÃë¨ìx
		float sky_state;
		DirectX::XMFLOAT4 top_sky_color;
		DirectX::XMFLOAT4 under_sky_color;
	};

	playshader(ID3D11Device* device, const char* filename, bool force_srgb, bool enable_caching, D3D11_FILTER sampler_filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode = D3D11_TEXTURE_ADDRESS_BORDER, DirectX::XMFLOAT4 sampler_boarder_color = DirectX::XMFLOAT4(0, 0, 0, 0));

	virtual ~playshader() = default;
	playshader(playshader&) = delete;
	playshader& operator =(playshader&) = delete;

	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/, float sx, float sy, float sw, float sh);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh);
	void textout(ID3D11DeviceContext* immediate_context, std::string s, float x, float y, float w, float h, float r, float g, float b, float a);
	void DebugDrawGUI(bool flag);

private:
	float cover = 0.5f;
	float rainy = 0.0f;
	float c_speed = 0.07f;
	float sky_state = 0.0f;
	DirectX::XMFLOAT4 top_sky_color = DirectX::XMFLOAT4(0.0f, 0.1f, 0.4f, 1.0f);
	DirectX::XMFLOAT4 un_sky_color = DirectX::XMFLOAT4(0.3f, 0.6f, 0.8f, 1.0f);


};

