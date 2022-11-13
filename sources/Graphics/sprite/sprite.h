#pragma once

#include <d3d11.h>
#include <directxmath.h>

#include <wrl.h>
#include <string>

class sprite
{
private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;

	D3D11_TEXTURE2D_DESC texture2d_desc;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
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

	sprite(ID3D11Device* device, const char* filename, bool force_srgb, bool enable_caching,D3D11_FILTER sampler_filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode = D3D11_TEXTURE_ADDRESS_BORDER, DirectX::XMFLOAT4 sampler_boarder_color = DirectX::XMFLOAT4(0, 0, 0, 0));
	sprite(ID3D11Device* device, const char* filename, bool force_srgb, bool enable_caching, const char* ps_shader,D3D11_FILTER sampler_filter = D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_MODE sampler_texture_address_mode = D3D11_TEXTURE_ADDRESS_BORDER, DirectX::XMFLOAT4 sampler_boarder_color = DirectX::XMFLOAT4(0, 0, 0, 0));

	virtual ~sprite() = default;
	sprite(sprite&) = delete;
	sprite& operator =(sprite&) = delete;

	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh, float r, float g, float b, float a, float angle/*degree*/, float sx, float sy, float sw, float sh);
	void render(ID3D11DeviceContext* immediate_context, float dx, float dy, float dw, float dh);
	void textout(ID3D11DeviceContext* immediate_context, std::string s, float x, float y, float w, float h, float r, float g, float b, float a);

	//differential   The value for scaling the judgment range from the original image.
    //ç∑ï™Å@ñ{óàÇÃâÊëúÇ©ÇÁîªíËÇÃîÕàÕÇägëÂèkè¨Ç∑ÇÈÇ∆Ç´ÇÃÇªÇÃíl
	bool hit_cursor(DirectX::XMFLOAT2 cursor_pos, DirectX::XMFLOAT2 differential = { 0,0 });

private:
	DirectX::XMFLOAT2 min_position{ 0,0 };
	DirectX::XMFLOAT2 max_position{ 0,0 };
};
