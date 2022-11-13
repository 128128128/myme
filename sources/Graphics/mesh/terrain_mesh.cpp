#include "../others/shader.h"
#include "../../Functions/misc.h"
#include "terrain_mesh.h"

#include <fstream>
#include <vector>

#include <filesystem>
#include "../load/texture.h"

terrain_mesh::terrain_mesh(ID3D11Device* device, const char* filename, bool shadow_reciever)
{
	substance = std::make_unique<Descartes::substance<vertex>>(filename, true/*triangulate*/, -1/*not fetch animation*/);
	substance->create_render_objects(device, true/*clear_vertex_conteiner*/);

	char media_directory[256];
	strcpy_s(media_directory, filename);
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	if (shadow_reciever)
		is_shadow_reciever = true;

	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);
}

void terrain_mesh::render(ID3D11DeviceContext* immediate_context, const XMFLOAT4X4& world, Descartes::view_frustum view_frustum)
{
    DirectX::XMFLOAT3 bounding_box[2];
    DirectX::XMFLOAT3 world_bounding_box[2];
	for (Descartes::substance<vertex>::mesh& mesh : substance->meshes)
	{

		//frustam
		bounding_box[0] = mesh.bounding_box.min;
		bounding_box[1] = mesh.bounding_box.max;
		const XMFLOAT4X4 terrain_world_transform ={ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		//const XMFLOAT4X4 terrain_world_transform{ -0.01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.0f, 0.0f, 0.0f, 0.0f, 0.01f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

		//transform_bounding_box(world_transform, mesh.global_transform,view_frustum);

		transform_bounding_box(world, mesh.global_transform,bounding_box,world_bounding_box);
		//if (Descartes::view_frustum::intersect_frustum_aabb(view_frustum, world_bounding_box))
		//if (!Descartes::view_frustum::judge_cross(view_frustum, world_bounding_box))
		//{
		//	continue;
		//}

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

		if(is_shadow_reciever)
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