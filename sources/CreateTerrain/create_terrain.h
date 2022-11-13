#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>

class terrain
{
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
	};
	struct primitive_constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 eye_in_local_space; // w:tesselation_max_division
		float mix_rate = 1.0f;
		float texture1_num = 0.0f;
		float texture2_num = 0.0f;
		float pad;
	};
	float rate = 1.0f;
	float texture0 = 0.0f;
	float texture1 = 0.0f;
	UINT vertex_count;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;

	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11HullShader> hull_shader;
	Microsoft::WRL::ComPtr<ID3D11DomainShader> domain_shader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometry_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;

	D3D11_TEXTURE2D_DESC texture2d_desc;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_views[4];

	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_noise;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_noise;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_noise;
public:

	float horizontal_scale = 100.0f;
	float vertical_scale = 32.0f;
	float tesselation_max_subdivision = 16;

	terrain(ID3D11Device* device);
	virtual ~terrain() = default;

	void draw(ID3D11DeviceContext* immediate_context);
	void DebugDrawGUI();

};
