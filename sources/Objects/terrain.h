#pragma once
#include "../Graphics/mesh/terrain_mesh.h"
#include "../Graphics/others/shader.h"
#include "../Math/collision.h"
#include "../Graphics/others/render_state.h"

class grass_cluster
{
public:
	grass_cluster(ID3D11Device* device, const char* filename);
	virtual ~grass_cluster() = default;
	grass_cluster(grass_cluster&) = delete;
	grass_cluster& operator =(grass_cluster&) = delete;

	void create_instance_buffer(ID3D11Device* device);


public:
	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT2 texcoord;
		vertex(const Descartes::vertex& rhs)
		{
			position = rhs.position;
			normal = rhs.normal;
			texcoord = rhs.texcoord;
		}
		vertex() = default;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive
			(
				position.x, position.y, position.z,
				normal.x, normal.y, normal.z,
				texcoord.x, texcoord.y
			);
		}
	};
	std::unique_ptr<Descartes::substance<vertex>> substance;

	struct grass
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 dimension;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(position.x, position.y, position.z);
		}
	};
	std::vector<grass> grasses;

private:
	const size_t MAX_INSTANCES = 256;
	struct instance
	{
		DirectX::XMFLOAT4X4 transform;
	};
	std::vector<instance> instances;
	Microsoft::WRL::ComPtr<ID3D11Buffer> instance_buffer;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;

	std::unique_ptr<Descartes::rasterizer_state> rasterizer_state;
	std::unique_ptr<Descartes::blend_state> blend_state;

public:
	void render
	(
		ID3D11DeviceContext* immediate_context,
		const DirectX::XMFLOAT4X4& view,
		float draw_distance,
		float elapsed_time
	);
	friend class terrain;
};

class Terrain
{
public:
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Terrain(ID3D11Device* device, bool shadow_reciever = false);
	~Terrain() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context,Descartes::view_frustum view_frustum);


	//Debug
	void DebugDrawGUI(bool flag=false);

	//getter setter
	XMFLOAT4 get_position() { return position; }
	void set_position(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 get_direction() { return direction; }
	void set_direction(XMFLOAT4 dir) { direction = dir; }


private:
	std::unique_ptr<terrain_mesh> terrain;
	XMFLOAT4X4 world_transform;

	//shaders
	std::unique_ptr<vertex_shader<terrain_mesh::vertex>>vs;
	std::unique_ptr<pixel_shader> ps;

	//shader resources

};

class grass_Terrain
{
public:
	grass_Terrain(ID3D11Device* device);
	virtual ~grass_Terrain() = default;
	grass_Terrain(grass_Terrain&) = delete;
	grass_Terrain& operator =(grass_Terrain&) = delete;

	struct vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		DirectX::XMFLOAT4 tangent;
		DirectX::XMFLOAT2 texcoord;
		vertex(const Descartes::vertex& rhs)
		{
			position = rhs.position;
			normal = rhs.normal;
			tangent = rhs.tangent;
			texcoord = rhs.texcoord;
		}
		vertex() = default;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive
			(
				position.x, position.y, position.z,
				normal.x, normal.y, normal.z,
				tangent.x, tangent.y, tangent.z, tangent.w,
				texcoord.x, texcoord.y
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
			};
			num_elements = _countof(input_element_desc);
			return input_element_desc;
		}
	};
	std::unique_ptr<Descartes::substance<vertex>> substance;

	std::unique_ptr<grass_cluster> meadow;
	//std::unique_ptr<grass_cluster> pampas;
	std::unique_ptr<grass_cluster> flower;

private:
	void plant(const char* mesh_name, std::vector<grass_cluster::grass> & grasses, int pitch_cm, float scale_w, float scale_h, float noise/*ratio for height*/);

public:
	void render(ID3D11DeviceContext * immediate_context);

private:
	struct shader_constants
	{
		DirectX::XMFLOAT4X4 world;
		DirectX::XMFLOAT4 options; //xy:uv_scale, z:alpha, w:skymap reflection factor
	};
	std::unique_ptr<Descartes::constant_buffer<shader_constants>> constant_buffer;

	enum { NONE, ALPHA, ADD, ALPHA_TO_COVERAGE };
	std::unique_ptr<Descartes::blend_state> blend_states[4];
	std::unique_ptr<Descartes::rasterizer_state> rasterizer_state;

	DirectX::XMFLOAT3 translation = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 scaling = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
	DirectX::XMFLOAT3 rotation = DirectX::XMFLOAT3(0, 0, 0);

	//shaders
	std::unique_ptr<vertex_shader<terrain_mesh::vertex>>vs;
	std::unique_ptr<pixel_shader> ps;
};