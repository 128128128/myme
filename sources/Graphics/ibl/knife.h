#pragma once
#include "../mesh/static_mesh.h"
#include "../others/shader.h"
#include "../../Math/collision.h"

class Knife
{
public:
	std::unique_ptr<static_mesh> knife;

	XMFLOAT4 position = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 90.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

public:
	Knife();
	~Knife() = default;

	void update(float elapsed_time);

	void render(ID3D11DeviceContext* immediate_context);


	//Debug
	void DebugDrawGUI();

	//getter setter
	XMFLOAT4 get_position() { return position; }
	void set_position(XMFLOAT4 pos) { position = pos; }
	XMFLOAT4 get_direction() { return direction; }
	void set_direction(XMFLOAT4 dir) { direction = dir; }


private:
	XMFLOAT4X4 world_transform;


	//shader resources

};