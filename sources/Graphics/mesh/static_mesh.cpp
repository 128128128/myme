#include "../others/shader.h"
#include "../../Functions/misc.h"
#include "static_mesh.h"

#include <fstream>
#include <vector>

#include <filesystem>
#include "../load/texture.h"

static_mesh::static_mesh(ID3D11Device* device, const char* filename,bool shadow_reciever)
{
	substance = std::make_unique<Descartes::substance<vertex>>(filename, true/*triangulate*/, -1/*not fetch animation*/);
	substance->create_render_objects(device, false/*clear_vertex_conteiner*/);

	char media_directory[256];
	strcpy_s(media_directory, filename);
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	if (shadow_reciever)
		is_shadow_reciever = true;

	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);
}
void static_mesh::render(ID3D11DeviceContext* immediate_context, const XMFLOAT4X4& world, ID3D11PixelShader* replaced_pixel_shader)
{
	if(replaced_pixel_shader)
	{
		immediate_context->PSSetShader(replaced_pixel_shader, 0, 0);
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

		if (is_shadow_reciever)
			constant_buffer->data.shadow_reciever = 1.0f;

		for (Descartes::subset& subset : mesh.subsets)
		{
			constant_buffer->active(immediate_context, 0, true, true);

			const Descartes::material* material = substance->find_material(subset.name.c_str());
			immediate_context->PSSetShaderResources(0, 1, material->properties.at(0).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(1, 1, material->properties.at(1).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(2, 1, material->properties.at(2).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(3, 1, material->properties.at(3).shader_resource_view.GetAddressOf());

			immediate_context->DrawIndexed(subset.index_count, subset.index_start, 0);
		}
		constant_buffer->inactive(immediate_context);
	}
}