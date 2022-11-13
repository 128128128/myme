#pragma once
#include <d3d11.h>

#include "../Graphics/mesh/dynamic_mesh.h"
#include "../Graphics/mesh/pbr_dynamic_mesh.h"
#include "../Graphics/mesh/static_mesh.h"
#include "../Math/collision.h"

using namespace DirectX;
class Player
{
public:
	std::unique_ptr<dynamic_mesh> player;
	XMFLOAT4 position = XMFLOAT4(-110.0f, 21.5f, -39.0f, 0.0f);
	XMFLOAT4 direction = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT4 velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	XMFLOAT3 scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	struct
	{
		float speed = 8;
		float turning = 200;
		float initial_jump_speed = 30.5f;
		float magic_attack_range = 1.5f;
		int life = 3;
	} abilities;

	enum STATES { STATE_IDLE, STATE_RUN, STATE_RUN_STOP, STATE_ATTACK, STATE_JUMP_RISE, STATE_JUMP_FALL, STATE_LANDING, STATE_DEATH, STATE_STEPBACK, STATE_RESPAWN };
	STATES state = STATE_JUMP_FALL;

private:
	//animate
	enum ANIMATION_CLIP { IDLE, RUN, RUN_B, RUN_E, JUMP_FALL, LANDING,JUMP_RISE, ATTACK, DEATH };
	ANIMATION_CLIP animation_clip = IDLE;
	float animation_tick = 0.0f;
	size_t animation_frame = 0;
	void animate(float elapsed_time);

	//move
    enum COMMANDS{ C_IDLE, C_RUN, C_ATTACK, C_TURN_LEFT, C_TURN_RIGHT, C_JUMP, C_DEATH, C_STEPBACK };
	COMMANDS command = C_IDLE;
public:
	Player();
	virtual ~Player() = default;

	void update(Descartes::collision::mesh* collision_mesh,float elapsed_time);

    // レイキャスト
	//int raycast(Vector3& pos, Vector3& v, Vector3& out, Vector3& normal);
	//レイキャスト対象の移動(Translation)・回転(Rotation)・拡大(Scaling)対応
	//int raycast_trs(Vector3& pos, Vector3& v, Vector3& out, Vector3& normal);

	void render(ID3D11DeviceContext* immediate_context)
	{
		player->render(immediate_context, world_transform, animation_clip, animation_frame);
	}

	void reset(XMFLOAT4& location)
	{
		position = location;
		state = STATE_JUMP_FALL;
		float angle = 180;
		direction.x = sinf(angle * 0.01745f);
		direction.y = 0;
		direction.z = cosf(angle * 0.01745f);
	}

	void DebugDrawGUI();
private:
	XMFLOAT4X4 world_transform;

	STATES last_state = STATE_IDLE;

	float gravity = 98.0f;


	float angle = 90;

	bool raycast = false;
	bool use_gravity = false;
};
