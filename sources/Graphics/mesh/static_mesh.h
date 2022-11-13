#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <memory>

#include <string>

#include <vector>

#include "../others/constant_buffer.h"
#include "../load/substance.h"

class static_mesh
{
public:
	static_mesh(ID3D11Device* device, const char* filename, bool shadow_reciever =false);
	virtual ~static_mesh() = default;
	static_mesh(static_mesh&) = delete;
	static_mesh& operator=(static_mesh&) = delete;

	void render(ID3D11DeviceContext* immediate_context, const DirectX::XMFLOAT4X4& world);

	struct shader_constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 color = DirectX::XMFLOAT4(1, 1, 1, 1);
		float shadow_reciever = 0.0f;//1.0...recieve true  0.0...false
		DirectX::XMFLOAT3 pad;
	};
	std::unique_ptr<Descartes::constant_buffer<shader_constants>> constant_buffer;

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
				color.x,color.y,color.z,color.w
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
