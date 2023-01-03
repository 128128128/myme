#include "objects.h"
#include "../Graphics/others/shader.h"

class Water_Fall : public Objects
{
public:
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Water_Fall(ID3D11Device* device);
	~Water_Fall() = default;

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
	void DebugDrawGUI(bool flag = false)override;

	//getter setter
	XMFLOAT4 get_position() { return position; }
	void set_position(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 get_direction() { return direction; }
	void set_direction(XMFLOAT4 dir) { direction = dir; }


private:
	std::unique_ptr<static_mesh> water_fall;
	XMFLOAT4X4 world_transform;

	//shaders
	std::unique_ptr<vertex_shader<static_mesh::vertex>>vs;
	std::unique_ptr<pixel_shader> ps;

	//shader resource
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_shader_map;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise_map_1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise_map_2;


	//constant buffer
    struct water_fall_constants
	{
		float speed = 0.66f;
		float offset = 1.0f;
		DirectX::XMFLOAT2 pad;
	};
	std::unique_ptr<Descartes::constant_buffer<water_fall_constants>> water_fall_constants_buffer;

};
