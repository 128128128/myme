#include "water_fall_particle.h"

#include <random>

#include "../Graphics/others/shader.h"
#include "../Graphics/load/texture.h"
#include "../Functions/misc.h"

using namespace DirectX;

water_fall_particles::water_fall_particles(ID3D11Device* device, DirectX::XMFLOAT3 initial_position)
{
	// Radius of outermost orbit 
	float outermost_radius{ 8 };
	// Interval between two particles
	float interval{ 0.5f };
	// Height of snowfall area
	float area_height{ 6 };
	// Falling speed of snowflake
	float fall_speed{ -0.5f };

	// Orbit count
	int orbit_count = static_cast<int>(outermost_radius / interval);

	for (int orbit_index = 1; orbit_index <= orbit_count; ++orbit_index)
	{
		float radius = orbit_index * interval;

		for (float theta = 0; theta < 2 * 3.14159265358979f; theta += interval / radius)
		{
			for (float height = 0; height < area_height; height += interval)
			{
				particle_count++;
			}
		}
	}
	particles.resize(particle_count);

	std::mt19937 mt{ std::random_device{}() };
	std::uniform_real_distribution<float> rand(-interval * 0.5f, +interval * 0.5f);
	std::uniform_real_distribution<float> rand_d(-1.0f, +1.0f);

	size_t index{ 0 };
	for (int orbit_index = 1; orbit_index <= orbit_count; ++orbit_index)
	{
		float radius = orbit_index * interval;

		for (float theta = 0; theta < 2 * 3.14159265358979f; theta += interval / radius)
		{
			const float x{ radius * cosf(theta) };
			const float z{ radius * sinf(theta) };
			for (float height = -area_height * 0.5f; height < area_height * 0.5f; height += interval)
			{
				particles.at(index).position = { x + initial_position.x + rand(mt),  initial_position.y , z + initial_position.z + rand(mt) };
				//DirectX::XMFLOAT3 vec={0 - particles.at(index).position.x, 0 - particles.at(index).position.y, 0 - particles.at(index).position.z};
				DirectX::XMVECTOR vec = { 0 - particles.at(index).position.x, 0 - particles.at(index).position.y, 0 - particles.at(index).position.z };
				DirectX::XMVector3Normalize(vec);
				DirectX::XMFLOAT3 v = {};
				DirectX::XMStoreFloat3(&v, vec);
				float rand_speed = fall_speed + rand(mt) * (fall_speed * 0.5f) * v.y;
				if (rand_speed < 0.0f)
					rand_speed *= -1;
				particles.at(index).velocity = { 0.0f,rand_speed , 0.0f };
				particles.at(index).direction = { rand_d(mt),1.0f ,  rand_d(mt) };
				index++;
				//particles.at(index++).velocity = { 0.0f, fall_speed, 0.0f };
			}
		}
	}

	HRESULT hr{ S_OK };

	D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(water_fall_particle) * particle_count);
	buffer_desc.StructureByteStride = sizeof(water_fall_particle);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D11_SUBRESOURCE_DATA subresource_data{};
	subresource_data.pSysMem = particles.data();
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, particle_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_UNORDERED_ACCESS_VIEW_DESC unordered_access_view_desc;
	unordered_access_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	unordered_access_view_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	unordered_access_view_desc.Buffer.FirstElement = 0;
	unordered_access_view_desc.Buffer.NumElements = static_cast<UINT>(particle_count);
	unordered_access_view_desc.Buffer.Flags = 0;
	hr = device->CreateUnorderedAccessView(particle_buffer.Get(), &unordered_access_view_desc, particle_buffer_uav.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc;
	shader_resource_view_desc.Format = DXGI_FORMAT_UNKNOWN;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	shader_resource_view_desc.Buffer.ElementOffset = 0;
	shader_resource_view_desc.Buffer.NumElements = static_cast<UINT>(particle_count);
	hr = device->CreateShaderResourceView(particle_buffer.Get(), &shader_resource_view_desc, particle_buffer_srv.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	buffer_desc.ByteWidth = sizeof(particle_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	create_vs_from_cso(device, "shader//water_fall_particles_vs.cso", vertex_shader.ReleaseAndGetAddressOf(), nullptr, nullptr, 0);
	create_ps_from_cso(device, "shader//water_fall_particles_ps.cso", pixel_shader.ReleaseAndGetAddressOf());
	create_gs_from_cso(device, "shader//water_fall_particles_gs.cso", geometry_shader.ReleaseAndGetAddressOf());
	create_cs_from_cso(device, "shader//water_fall_particles_cs.cso", compute_shader.ReleaseAndGetAddressOf());

	particle_data.current_eye_position = { initial_position.x,initial_position.y,initial_position.z,1.0f };
	particle_data.previous_eye_position = { initial_position.x,initial_position.y,initial_position.z,1.0f };
	particle_data.snowfall_area_height = area_height;
	particle_data.outermost_radius = outermost_radius;
	particle_data.particle_size = 0.2f;
	
	particle_data.particle_count = static_cast<uint32_t>(particle_count);

}

UINT align(UINT num, UINT alignment)
{
	return (num + (alignment - 1)) & ~(alignment - 1);
}

void water_fall_particles::integrate(ID3D11DeviceContext* immediate_context, DirectX::XMFLOAT3 eye_position, XMFLOAT4X4 camera_view, DirectX::XMFLOAT4X4 camera_projection, float elapsd_time, float time)
{
	HRESULT hr{ S_OK };
	particle_data.previous_eye_position = particle_data.current_eye_position;
	particle_data.current_eye_position = { eye_position.x,eye_position.y,eye_position.z,1.0f };
	particle_data.camera_view = camera_view;
	particle_data.camera_projection = camera_projection;
	particle_data.elapsed_time = elapsd_time;
	particle_data.time = time;

	immediate_context->UpdateSubresource(constant_buffer.Get(), 0, 0, &particle_data, 0, 0);
	immediate_context->CSSetConstantBuffers(10, 1, constant_buffer.GetAddressOf());

	immediate_context->CSSetShader(compute_shader.Get(), NULL, 0);
	immediate_context->CSSetUnorderedAccessViews(0, 1, particle_buffer_uav.GetAddressOf(), nullptr);

	UINT num_threads = align(static_cast<UINT>(particle_count), 256);
	immediate_context->Dispatch(num_threads / 256, 1, 1);

	ID3D11UnorderedAccessView* null_unordered_access_view{};
	immediate_context->CSSetUnorderedAccessViews(0, 1, &null_unordered_access_view, nullptr);

}

void water_fall_particles::render(ID3D11DeviceContext* immediate_context)
{
	immediate_context->PSSetShaderResources(0, 1, shader_resource_view.GetAddressOf());
	immediate_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());

	immediate_context->VSSetShader(vertex_shader.Get(), NULL, 0);
	immediate_context->PSSetShader(pixel_shader.Get(), NULL, 0);
	immediate_context->GSSetShader(geometry_shader.Get(), NULL, 0);
	immediate_context->GSSetShaderResources(10, 1, particle_buffer_srv.GetAddressOf());
	immediate_context->GSSetConstantBuffers(10, 1, constant_buffer.GetAddressOf());

	immediate_context->IASetInputLayout(NULL);
	immediate_context->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
	immediate_context->IASetIndexBuffer(NULL, DXGI_FORMAT_R32_UINT, 0);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	immediate_context->Draw(static_cast<UINT>(particle_count), 0);

	ID3D11ShaderResourceView* null_shader_resource_view{};
	immediate_context->GSSetShaderResources(9, 1, &null_shader_resource_view);
	immediate_context->VSSetShader(NULL, NULL, 0);
	immediate_context->PSSetShader(NULL, NULL, 0);
	immediate_context->GSSetShader(NULL, NULL, 0);
}