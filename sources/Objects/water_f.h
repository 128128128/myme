#pragma once

#include "objects.h"
#include "../Graphics/others/shader.h"

class Water_franel : public Objects
{
public:
	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Water_franel(ID3D11Device* device);
	~Water_franel() = default;

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
	std::unique_ptr<static_mesh> water_plane;
	XMFLOAT4X4 world_transform;

	//shaders
	std::unique_ptr<vertex_shader<static_mesh::vertex>>vs;
	std::unique_ptr<pixel_shader> ps;


};
