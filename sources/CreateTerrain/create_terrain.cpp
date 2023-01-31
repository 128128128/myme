#include "create_terrain.h"

#include <fstream>
#include <vector>

#include "../Graphics/load/texture.h"
#include "../Graphics/others/shader.h"
#include "../Functions/misc.h"

#include "../../imgui/imgui.h"

terrain::terrain(ID3D11Device* device)
{
	HRESULT hr = S_OK;

	load_texture_from_file(device, ".\\resources\\create_terrain\\moss.png", shader_resource_views[0].ReleaseAndGetAddressOf(),true,true);
	load_texture_from_file(device, ".\\resources\\create_terrain\\red_earth.png", shader_resource_views[1].ReleaseAndGetAddressOf(),true,true);
	load_texture_from_file(device, ".\\resources\\create_terrain\\moss_1.png", shader_resource_views[2].ReleaseAndGetAddressOf(),true,true);
	load_texture_from_file(device, ".\\resources\\create_terrain\\ground.png", shader_resource_views[3].ReleaseAndGetAddressOf(),true,true);


	texture2d_desc =fetch_texture2d_description(shader_resource_views[0].Get());


	std::vector<vertex> vertices;

	std::vector<DirectX::XMFLOAT3> positions;
	std::vector<DirectX::XMFLOAT2> texcoords;

	std::wifstream fin("./resources/create_terrain/grid.obj");
	_ASSERT_EXPR(fin, L"'OBJ file not found.");
	wchar_t command[256];
	while (fin)
	{
		fin >> command;
		if (0 == wcscmp(command, L"v"))
		{
			float x, y, z;
			fin >> x >> y >> z;
			positions.push_back({ x, y, z });
			fin.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"vt"))
		{
			float u, v;
			fin >> u >> v;
			texcoords.push_back({ u, 1.0f - v });
			fin.ignore(1024, L'\n');
		}
		else if (0 == wcscmp(command, L"f"))
		{
			for (size_t i = 0; i < 3; i++)
			{
				vertex vertex;
				size_t v, vt, vn;

				fin >> v;
				vertex.position = positions.at(v - 1);
				if (L'/' == fin.peek())
				{
					fin.ignore(1);
					if (L'/' != fin.peek())
					{
						fin >> vt;
						vertex.texcoord = texcoords.at(vt - 1);
					}
					if (L'/' == fin.peek())
					{
						fin.ignore(1);
						fin >> vn;
					}
				}
				vertices.push_back(vertex);
			}
			fin.ignore(1024, L'\n');
		}
		else
		{
			fin.ignore(1024, L'\n');
		}
	}
	fin.close();

	vertex_count = static_cast<UINT>(vertices.size());

	D3D11_BUFFER_DESC buffer_desc{};
	D3D11_SUBRESOURCE_DATA subresource_data{};
	buffer_desc.ByteWidth = static_cast<UINT>(sizeof(vertex) * vertices.size());
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	subresource_data.pSysMem = vertices.data();
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertex_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	//D3D11_BUFFER_DESC buffer_desc{};
	buffer_desc.ByteWidth = sizeof(primitive_constants);
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	hr = device->CreateBuffer(&buffer_desc, nullptr, constant_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));


	D3D11_INPUT_ELEMENT_DESC input_element_desc[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	create_vs_from_cso(device, "shader//create_terrain_vs.cso", vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, ARRAYSIZE(input_element_desc));
	create_hs_from_cso(device, "shader//create_terrain_hs.cso", hull_shader.GetAddressOf());
	create_ds_from_cso(device, "shader//create_terrain_ds.cso", domain_shader.GetAddressOf());
	create_gs_from_cso(device, "shader//create_terrain_gs.cso", geometry_shader.GetAddressOf());
	create_ps_from_cso(device, "shader//create_terrain_ps.cso", pixel_shader.GetAddressOf());

}

void terrain::draw(ID3D11DeviceContext* immediate_context)
{
	using namespace DirectX;

	primitive_constants primitive_data = {};

	const DirectX::XMFLOAT4X4 coordinate_system_transforms[] = {
		{ -1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },	// 0:RHS Y-UP
		{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 },		// 1:LHS Y-UP
		{ -1, 0, 0, 0, 0, 0, -1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },	// 2:RHS Z-UP
		{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1 },		// 3:LHS Z-UP
	};
	XMMATRIX C = XMLoadFloat4x4(&coordinate_system_transforms[0]);
	const float aspect_ratio = static_cast<float>(texture2d_desc.Width) / texture2d_desc.Height;
	XMStoreFloat4x4(&primitive_data.world, C * XMMatrixScaling(horizontal_scale * aspect_ratio, vertical_scale, horizontal_scale));
	primitive_data.eye_in_local_space.w = tesselation_max_subdivision;
	primitive_data.mix_rate = rate;
	primitive_data.texture1_num = texture0;
	primitive_data.texture2_num = texture1;
	primitive_data.vertical_scale = vertical_scale;
	primitive_data.smooth = smooth;

	immediate_context->UpdateSubresource(constant_buffer.Get(), 0, 0, &primitive_data, 0, 0);
	//immediate_context->VSSetConstantBuffers(8, 1, constant_buffer.GetAddressOf());
	immediate_context->HSSetConstantBuffers(8, 1, constant_buffer.GetAddressOf());
	immediate_context->GSSetConstantBuffers(8, 1, constant_buffer.GetAddressOf());
	immediate_context->PSSetConstantBuffers(8, 1, constant_buffer.GetAddressOf());

	uint32_t stride{ sizeof(vertex) };
	uint32_t offset{ 0 };
	immediate_context->IASetVertexBuffers(0, 1, vertex_buffer.GetAddressOf(), &stride, &offset);
	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	immediate_context->IASetInputLayout(input_layout.Get());
	immediate_context->VSSetShader(vertex_shader.Get(), NULL, 0);
	immediate_context->HSSetShader(hull_shader.Get(), NULL, 0);
	immediate_context->DSSetShader(domain_shader.Get(), NULL, 0);
	immediate_context->GSSetShader(geometry_shader.Get(), NULL, 0);
	immediate_context->PSSetShader(pixel_shader.Get(), NULL, 0);

	immediate_context->DSSetShaderResources(0, 1, shader_resource_views[0].GetAddressOf());
	immediate_context->DSSetShaderResources(1, 1, shader_resource_views[1].GetAddressOf());
	if(texture0==0.0f)
    immediate_context->PSSetShaderResources(0, 1, shader_resource_views[0].GetAddressOf());
	else
		immediate_context->PSSetShaderResources(0, 1, shader_resource_views[2].GetAddressOf());
	if (texture1 == 0.0f)
		immediate_context->PSSetShaderResources(1, 1, shader_resource_views[1].GetAddressOf());
	else
		immediate_context->PSSetShaderResources(1, 1, shader_resource_views[3].GetAddressOf());

	immediate_context->Draw(vertex_count, 0);

	immediate_context->IASetInputLayout(NULL);
	immediate_context->VSSetShader(NULL, NULL, 0);
	immediate_context->HSSetShader(NULL, NULL, 0);
	immediate_context->DSSetShader(NULL, NULL, 0);
	immediate_context->GSSetShader(NULL, NULL, 0);
	immediate_context->PSSetShader(NULL, NULL, 0);
}

void terrain::DebugDrawGUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Texture", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("smooth", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("terrain smooth", &smooth, 0.0f, 1.0f);
		}
		if (ImGui::CollapsingHeader("Rate", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("mix rate", &rate, 1.0f, 5.0f);
		}
		if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (texture0 == 0.0f)
			ImGui::Image(shader_resource_views[0].Get(), { 100.0f,100.0f });
			else
				ImGui::Image(shader_resource_views[2].Get(), { 100.0f,100.0f });
			ImGui::InputFloat("texture0 num", &texture0, 1.0f, 1.0f);

			if (texture1 == 0.0f)
			ImGui::Image(shader_resource_views[1].Get(), { 100.0f,100.0f });
			else
			ImGui::Image(shader_resource_views[3].Get(), { 100.0f,100.0f });
			ImGui::InputFloat("texture1 num", &texture1, 1.0f, 1.0f);

		}
	}

	ImGui::End();
}