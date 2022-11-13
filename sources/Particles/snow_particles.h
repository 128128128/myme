#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <directxmath.h>
#include <vector>


struct snow_particles
{
	struct snow_particle
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 velocity;
	};
	std::vector<snow_particle> particles;

	struct particle_constants
	{
		DirectX::XMFLOAT4 current_eye_position;
		DirectX::XMFLOAT4 previous_eye_position;
		DirectX::XMFLOAT4X4 camera_view;
		DirectX::XMFLOAT4X4 camera_projection;

		// Radius of outermost orbit 
		float outermost_radius;
		// Height of snowfall area
		float snowfall_area_height;

		float particle_size;
		uint32_t particle_count;

	};
	particle_constants particle_data;

	snow_particles(ID3D11Device* device, DirectX::XMFLOAT3 initial_position);

	void integrate(ID3D11DeviceContext* immediate_context, DirectX::XMFLOAT3 eye_position, DirectX::XMFLOAT4X4 camera_view, DirectX::XMFLOAT4X4 camera_view_projection);
	void render(ID3D11DeviceContext* immediate_context);

private:

	size_t snow_particle_count{ 0 };
	Microsoft::WRL::ComPtr<ID3D11Buffer> particle_buffer;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> particle_buffer_uav;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particle_buffer_srv;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11GeometryShader> geometry_shader;
	Microsoft::WRL::ComPtr<ID3D11ComputeShader> compute_shader;
	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shader_resource_view;
	D3D11_TEXTURE2D_DESC texture2d_desc;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state;
};