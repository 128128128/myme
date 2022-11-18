#include "../others/shader.h"
#include "../../Functions/misc.h"
#include "pbr_static_mesh.h"

#include <fstream>
#include <vector>

#include <filesystem>
#include "../load/texture.h"

pbr_static_mesh::pbr_static_mesh(ID3D11Device* device, const char* filename)
{
	substance = std::make_unique<Descartes::substance<vertex>>(filename, false/*triangulate*/, -1/*not fetch animation*/);
	substance->create_render_objects(device, true/*clear_vertex_conteiner*/);

	char media_directory[256];
	strcpy_s(media_directory, filename);
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	/*load_texture_from_file(device, ".\\resources\\enviroments\\structures\\DropShipTexture\\Dropship_01_MetallicSmooth.tif", metallic_shader_resource_view[0].GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\enviroments\\structures\\DropShipTexture\\Dropship_02_MetallicSmooth.tif", metallic_shader_resource_view[1].GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\enviroments\\structures\\DropShipTexture\\Dropship_03_MetallicSmooth.tif", metallic_shader_resource_view[2].GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\enviroments\\structures\\DropShipTexture\\Dropship_04_MetallicSmooth.tif", metallic_shader_resource_view[3].GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\enviroments\\structures\\DropShipTexture\\Dropship_05_MetallicSmooth.tif", metallic_shader_resource_view[4].GetAddressOf(), true, true);

	material_name[0] = "Dropship_01_Mat";
	material_name[1] = "Dropship_02_Mat";
	material_name[2] = "Dropship_03_Mat";
	material_name[3] = "Dropship_04_Mat";
	material_name[4] = "Dropship_05_Mat";*/

	load_texture_from_file(device, ".\\resources\\Jummo\\Textures\\mixbot_low_mixamo_edit1_MetallicSmoothness.png", metallic_shader_resource_view.GetAddressOf(), true, true);

	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);
}

void pbr_static_mesh::render(ID3D11DeviceContext* immediate_context, const XMFLOAT4X4& world, ID3D11PixelShader* replaced_pixel_shader, ID3D11VertexShader* use_vertex_shader)
{
	if (replaced_pixel_shader)
	{
		immediate_context->PSSetShader(replaced_pixel_shader, nullptr, 0);
	}
	if (use_vertex_shader)
	{
		immediate_context->VSSetShader(use_vertex_shader, nullptr, 0);
	}

	for (Descartes::substance<vertex>::mesh& mesh : substance->meshes)
	{
		uint32_t stride = sizeof(vertex);
		uint32_t offset = 0;

		immediate_context->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &stride, &offset);
		immediate_context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMStoreFloat4x4
		(
			&constant_buffer->data.world,
			XMMatrixTranspose
			(
				XMLoadFloat4x4(&mesh.global_transform) *
				XMLoadFloat4x4(&substance->system_unit_transform) *
				XMLoadFloat4x4(&substance->axis_system_transform) *
				XMLoadFloat4x4(&world)
			)
		);
		constant_buffer->active(immediate_context, 0, true, true);
		for (Descartes::subset& subset : mesh.subsets)
		{
			const Descartes::material* material = substance->find_material(subset.name.c_str());
			immediate_context->PSSetShaderResources(0, 1, material->properties.at(0).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(1, 1, material->properties.at(1).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(2, 1, material->properties.at(2).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(3, 1, material->properties.at(3).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(4, 1, material->properties.at(4).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(5, 1, material->properties.at(5).shader_resource_view.GetAddressOf());


			//metallic
			/*for (int i = 0; i < 5; i++) {
			    std::string m = material_name[i];
				if (m == subset.name) {
					immediate_context->PSSetShaderResources(6, 1, metallic_shader_resource_view[i].GetAddressOf());
					break;
				}
			}*/
			immediate_context->PSSetShaderResources(6, 1, metallic_shader_resource_view.GetAddressOf());
			immediate_context->DrawIndexed(static_cast<UINT>(subset.index_count), static_cast<UINT>(subset.index_start), 0);
		}
		constant_buffer->inactive(immediate_context);		
	}
}