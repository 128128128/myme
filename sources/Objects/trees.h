#include "objects.h"
#include "../Graphics/others/shader.h"


class Trees : public Objects
{
public:
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Trees(ID3D11Device* device);
	~Trees() = default;

	void update(float elapsed_time)override;

	void render(ID3D11DeviceContext* immediate_context)override;

	void reset(XMFLOAT4& location)override
	{
		position = location;
		float angle = 180;
		direction.x = sinf(angle * 0.01745f);
		direction.y = 0;
		direction.z = cosf(angle * 0.01745f);
	}
	//Debug
	void DebugDrawGUI()override;

	//getter setter
	XMFLOAT4 get_position() { return position; }
	void set_position(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 get_direction() { return direction; }
	void set_direction(XMFLOAT4 dir) { direction = dir; }


private:
	std::unique_ptr<static_mesh> trees;
	std::unique_ptr<static_mesh> trunks;
	XMFLOAT4X4 world_transform;

	//shaders
	std::unique_ptr<vertex_shader<static_mesh::vertex>>vs;
	std::unique_ptr<vertex_shader<static_mesh::vertex>>trunk_vs;
	std::unique_ptr<pixel_shader> ps;
	std::unique_ptr<pixel_shader> trunk_ps;

	//shader resources
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> vector_map;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> leaves_map;

	//constant buffer
	struct shader_constants
	{
		float max_wind_strength = 1.0f;
		float wind_strength = 0.46f;
		float wind_speed = 0.5f;
		float mask = 0.5f;
		DirectX::XMFLOAT3 wind_direction = { 0.0f,0.0f,-1.0f };
		float wind_amplitude=1.62f;
		float wind_amplitude_multiplier = 10.0f;
		float flat_lighting = 0.3f;
		float wind_debug = 0.0f;
		float pad;
	};
	std::unique_ptr<Descartes::constant_buffer<shader_constants>> constant_buffer;

};
