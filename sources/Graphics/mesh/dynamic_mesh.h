#pragma once

#include <d3d11.h>
#include <wrl.h>

#include <DirectXMath.h>

#include <vector>
#include <string>

#include <fbxsdk.h>

#include <unordered_map>

#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>

#include "../load/substance.h"
#include "../others/constant_buffer.h"


class dynamic_mesh
{
public:
	dynamic_mesh(ID3D11Device* device, const char* filename, bool triangle = false, int animation_sampling_rate = 0);
	dynamic_mesh(ID3D11Device* device, const char* filename, const char* animation_filenames[] ,size_t number_of_animations, bool triangle = false, int animation_sampling_rate = 0);

	virtual ~dynamic_mesh() = default;
	dynamic_mesh(dynamic_mesh&) = delete;
	dynamic_mesh& operator =(dynamic_mesh&) = delete;

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 tangent;
		DirectX::XMFLOAT2 texcoord;
		float bone_weights[MAX_BONE_INFLUENCES];
		uint32_t bone_indices[MAX_BONE_INFLUENCES];
		vertex(const Descartes::vertex& rhs)
		{
			position = rhs.position;
			normal = rhs.normal;
			tangent = rhs.tangent;
			texcoord = rhs.texcoord;
			memcpy(bone_weights, rhs.bone_weights, sizeof(bone_weights));
			memcpy(bone_indices, rhs.bone_indices, sizeof(bone_indices));
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
				texcoord.x, texcoord.y,
				bone_weights,
				bone_indices
			);
		}
		static const D3D11_INPUT_ELEMENT_DESC* acquire_input_element_desc(size_t& num_elements)
		{
			static const D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
				{ "BONES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			};
			num_elements = _countof(input_element_desc);
			return input_element_desc;
		}
	};
	std::unique_ptr<Descartes::substance<vertex>> substance;

	struct object_constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4X4 bone_transforms[MAX_BONES] = { { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 } };
	};
	std::unique_ptr<Descartes::constant_buffer<object_constants>> object_constants_buffer;

	void render(ID3D11DeviceContext* immediate_context, const XMFLOAT4X4& world, size_t animation_clip, size_t animation_frame);
	void add_animations(const char* fbx_filename, int sampling_rate /*0:use default value*/)
	{
		substance->add_animations(fbx_filename, sampling_rate);
	}

};