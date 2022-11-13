#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <memory>

#include <string>

#include <vector>

#include "../others/constant_buffer.h"
#include "../load/substance.h"
#include "../../Math/collision.h"

class terrain_mesh
{
public:
	terrain_mesh(ID3D11Device* device, const char* filename,bool shadow_reciever = false);
	virtual ~terrain_mesh() = default;
	terrain_mesh(terrain_mesh&) = delete;
	terrain_mesh& operator=(terrain_mesh&) = delete;

	void render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world, Descartes::view_frustum view_frustum);

	struct shader_constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1);
		float shadow_reciever = 0.0;
		DirectX::XMFLOAT3 pad;
	};
	std::unique_ptr<Descartes::constant_buffer<shader_constants>> constant_buffer;

	DirectX::XMFLOAT4X4 default_global_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	DirectX::XMFLOAT4X4 geometric_transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };

	// bounding_box [0] is the min vertex and [1] is the max

	// This function calculates and outputs the coordinates of the bounding box in world space.
	void transform_bounding_box(const DirectX::XMFLOAT4X4& world_transform, DirectX::XMFLOAT4X4 mesh_global_transform, DirectX::XMFLOAT3 mesh_bounding_box[2], DirectX::XMFLOAT3 world_bounding_box[2]) const
	//void transform_bounding_box(const DirectX::XMFLOAT4X4& world_transform, DirectX::XMFLOAT4X4 mesh_global_transform, Descartes::view_frustum view_frustum) const
	{
		//DirectX::XMMATRIX concatenated_matrix{
		//	//DirectX::XMLoadFloat4x4(&mesh_global_transform)*
		//	DirectX::XMLoadFloat4x4(&geometric_transform) *
		//	DirectX::XMLoadFloat4x4(&default_global_transform) *
		//	DirectX::XMLoadFloat4x4(&world_transform) };
		//XMMATRIX T = XMMatrixInverse(0, concatenated_matrix);

		//for (int plane_index = 0; plane_index < 6; ++plane_index)
		//{
		//	XMStoreFloat4(&view_frustum.faces[plane_index], XMVector4Transform(XMVectorSet(view_frustum.faces[plane_index].x, view_frustum.faces[plane_index].y, view_frustum.faces[plane_index].z, view_frustum.faces[plane_index].w), T));

		//}

		DirectX::XMMATRIX concatenated_matrix{
			DirectX::XMLoadFloat4x4(&mesh_global_transform)*
			DirectX::XMLoadFloat4x4(&geometric_transform) *
			DirectX::XMLoadFloat4x4(&default_global_transform) *
			DirectX::XMLoadFloat4x4(&world_transform) };
		DirectX::XMStoreFloat3(&world_bounding_box[0], DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&mesh_bounding_box[0]), concatenated_matrix));
		DirectX::XMStoreFloat3(&world_bounding_box[1], DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&mesh_bounding_box[1]), concatenated_matrix));

		if (world_bounding_box[0].x > world_bounding_box[1].x) 
			std::swap<float>(world_bounding_box[0].x, world_bounding_box[1].x);
		if (world_bounding_box[0].y > world_bounding_box[1].y) 
			std::swap<float>(world_bounding_box[0].y, world_bounding_box[1].y);
		if (world_bounding_box[0].z > world_bounding_box[1].z) 
			std::swap<float>(world_bounding_box[0].z, world_bounding_box[1].z);
	}

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 tangent;
		DirectX::XMFLOAT3 binormal;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT4 color;
		vertex(const Descartes::vertex& rhs)
		{
			position = rhs.position;
			normal = rhs.normal;
			tangent = rhs.tangent;
			binormal = rhs.binormal;
			texcoord = rhs.texcoord;
			color = rhs.color;
		}
		vertex() = default;

		template<class T>
		void serialize(T& archive)
		{
			archive
			(
				position.x, position.y, position.z,
				normal.x, normal.y, normal.z,
				tangent.x, tangent.y, tangent.z, tangent.w,
				binormal.x, binormal.y, binormal.z,
				texcoord.x, texcoord.y,
				color.x, color.y, color.z, color.w
			);
		}
		static const D3D11_INPUT_ELEMENT_DESC* acquire_input_element_desc(size_t& num_elements)
		{
			static const D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			num_elements = _countof(input_element_desc);
			return input_element_desc;
		}
	};
	std::unique_ptr<Descartes::substance<vertex>> substance;

protected:
	bool is_shadow_reciever = false;
};
