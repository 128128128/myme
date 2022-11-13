
#include "../../Functions/misc.h"
#include "dynamic_mesh.h"

#include <fstream>
#include <sstream>
#include <functional>

using namespace DirectX;

#include "../others/shader.h"

#include <filesystem>
#include "../load/texture.h"

dynamic_mesh::dynamic_mesh(ID3D11Device* device, const char* filename, bool triangle, int animation_sampling_rate)
{
	substance = std::make_unique<Descartes::substance<vertex>>(filename, triangle/*no triangulate*/, animation_sampling_rate);
	substance->create_render_objects(device, true);
	char media_directory[256];
	strcpy_s(media_directory, filename);
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	object_constants_buffer = std::make_unique<Descartes::constant_buffer<object_constants>>(device);

}

dynamic_mesh::dynamic_mesh(ID3D11Device* device, const char* filename, const char* animation_filenames[], size_t number_of_animations, bool triangle, int animation_sampling_rate)
{
	substance = std::make_unique<Descartes::substance<vertex>>(filename, triangle/*no triangulate*/, animation_filenames, number_of_animations, animation_sampling_rate);
	substance->create_render_objects(device, true);

	char media_directory[256];
	strcpy_s(media_directory, filename);
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	object_constants_buffer = std::make_unique<Descartes::constant_buffer<object_constants>>(device);
}

void dynamic_mesh::render(ID3D11DeviceContext* immediate_context, const XMFLOAT4X4& world, size_t animation_clip, size_t animation_frame)
{
	size_t number_of_animations = substance->animation_takes.size();
	for (Descartes::substance<vertex>::mesh& mesh : substance->meshes)
	{
		uint32_t stride = sizeof(vertex);
		uint32_t offset = 0;

		immediate_context->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &stride, &offset);
		immediate_context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMStoreFloat4x4
		(
			&object_constants_buffer->data.world,
			(
				XMLoadFloat4x4(&mesh.global_transform) *
				XMLoadFloat4x4(&substance->system_unit_transform) *
				XMLoadFloat4x4(&substance->axis_system_transform) *
				XMLoadFloat4x4(&world)
			)
		);

		if (number_of_animations > 0)
		{
			const Descartes::animation& animation = mesh.animations.at(animation_clip);
			const Descartes::skeleton& skeleton_transform = animation.skeleton_transforms.at(animation_frame);
			XMFLOAT4X4 actor_transform;
			actor_transform = animation.actor_transforms.size() > 0 ? animation.actor_transforms.at(animation_frame).transform : mesh.global_transform;
			size_t number_of_bones = mesh.offset_transforms.size();
			_ASSERT_EXPR(number_of_bones < MAX_BONES, L"'number_of_bones' < MAX_BONES.");
			for (size_t i = 0; i < number_of_bones; i++)
			{
				XMStoreFloat4x4
				(
					&object_constants_buffer->data.bone_transforms[i],
					(
						XMLoadFloat4x4(&mesh.offset_transforms.at(i).transform) *
						XMLoadFloat4x4(&skeleton_transform.bones.at(i).transform) *
						//DirectX::XMMatrixInverse(0, XMLoadFloat4x4(&actor_transform))
						XMLoadFloat4x4(&actor_transform)
					)
				);
			}
		}
		else
		{
			for (int i = 0; i < MAX_BONES; i++)
			{
				object_constants_buffer->data.bone_transforms[i] = XMFLOAT4X4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
			}
		}
		object_constants_buffer->active(immediate_context, 0, true, false);
		for (Descartes::subset& subset : mesh.subsets)
		{
			const Descartes::material* material = substance->find_material(subset.name.c_str());
			immediate_context->PSSetShaderResources(0, 1, material->properties.at(0).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(1, 1, material->properties.at(1).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(2, 1, material->properties.at(2).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(3, 1, material->properties.at(3).shader_resource_view.GetAddressOf());
			

			immediate_context->DrawIndexed(subset.index_count, subset.index_start, 0);
		}
		object_constants_buffer->inactive(immediate_context);
	}
}


