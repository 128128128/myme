#include "character.h"
#include "../Input/Input.h"
#include "../imgui/imgui.h"

Player::Player()
{
    state = STATE_IDLE;

    velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	angle =180;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);

}

void Player::animate(float elapsedtime)
{
    static ANIMATION_CLIP last_animation_clip;
    last_animation_clip = animation_clip;

	switch (state)
	{
	case STATE_IDLE:
		switch (animation_clip)
		{
		case RUN:
			animation_clip = LANDING;
			break;
		case JUMP_FALL:
			animation_clip = LANDING;
			break;
		}
		break;
	case STATE_RUN:
		if (animation_clip != ATTACK && animation_clip != RUN)
		{
			animation_clip = RUN;
		}
		else if (animation_clip == JUMP_FALL)
		{
			animation_clip = RUN;
		}
		break;
	case STATE_RUN_STOP:
		animation_clip = IDLE;
		break;
	case STATE_ATTACK:
		if (animation_clip != ATTACK)
		{
			animation_clip = ATTACK;
		}
		break;
	case STATE_JUMP_RISE:
		animation_clip = JUMP_RISE;
		break;
	case STATE_JUMP_FALL:
		if (animation_clip !=JUMP_FALL)
		{
			animation_clip = JUMP_FALL;
		}
		break;
	case STATE_LANDING:
		if (animation_clip != LANDING)
		{
			animation_clip = LANDING;
		}
		break;
	case STATE_DEATH:
		if (animation_clip != DEATH)
		{
			animation_clip = DEATH;
		}
		break;
	case STATE_STEPBACK:
		animation_clip = JUMP_RISE;
		break;
	}

    if (animation_clip != last_animation_clip)
    {
        animation_frame = 0;
        animation_tick = 0;
    }

    animation_frame = static_cast<size_t>(animation_tick * player->substance->animation_takes.at(animation_clip).sampling_rate);
    size_t last_of_frame = player->substance->animation_takes.at(animation_clip).number_of_keyframes - 2;

    if (animation_frame > last_of_frame) //end of frames
    {
        animation_frame = 0;
        animation_tick = 0;
		switch (animation_clip)
		{
		case IDLE:
			animation_clip = IDLE;
			state = STATE_IDLE;
			break;
		case RUN_B:
			animation_clip = RUN;
			state = STATE_RUN;
			break;
		case RUN:
			animation_clip = RUN;
			break;
		case RUN_E:
			animation_clip = IDLE;
			state = STATE_IDLE;
			break;
		case ATTACK:
			animation_clip = IDLE;
			state = STATE_IDLE;
			break;
		case JUMP_RISE:
			//animation_clip =JUMP_RISE;
			animation_frame = last_of_frame-1;
			animation_tick = static_cast<float>(INT_MAX);
			//state = STATE_JUMP_RISE;
			break;
		case JUMP_FALL:
			//animation_clip = JUMP_FALL;
			animation_frame = last_of_frame-1;
			animation_tick = static_cast<float>(INT_MAX);
			//state = STATE_JUMP_FALL;
			break;
		case LANDING:
			animation_clip = IDLE;
			state = STATE_IDLE;
			break;
		case DEATH:
			animation_clip = DEATH;
			animation_frame = last_of_frame-1;
			animation_tick = static_cast<float>(INT_MAX);
			state = STATE_DEATH;
			break;
		}
    }
    animation_tick += elapsedtime * 1.0f;

}

//void Player::update(Descartes::collision::mesh* collision_mesh, float elapsed_time)
//{
//	GamePad& gamePad = Input::Instance().GetGamePad();
//
//	XMMATRIX S, R, T;
//	
//		XMVECTOR X, Y, Z;
//		Y = XMVectorSet(0, 1, 0, 0);
//		Z = XMVector3Normalize(XMLoadFloat4(&direction));
//		X = XMVector3Cross(Y, Z);
//		Y = XMVector3Normalize(XMVector3Cross(Z, X));
//
//
//		//回転行列から全方向ベクトルを取り出す
//		DirectX::XMVECTOR Right = { world_transform._11, world_transform._12, world_transform._13 };
//		DirectX::XMVECTOR Up = { world_transform._21, world_transform._22, world_transform._23 };
//		DirectX::XMVECTOR Forward = { world_transform._31, world_transform._32, world_transform._33 };
//
//		DirectX::XMVECTOR Pos = DirectX::XMLoadFloat4(&position);
//
//		if (gamePad.GetAxisLX() > 0.2f || (gamePad.GetButton() & GamePad::KEY_A))
//		{
//			Pos += Right * abilities.speed * elapsed_time;
//		}
//		if (gamePad.GetAxisLX() < -0.2f || (gamePad.GetButton() & GamePad::KEY_D))
//		{
//			Pos += -Right * abilities.speed * elapsed_time;
//		}
//		if (gamePad.GetAxisLY() > 0.2f || (gamePad.GetButton() & GamePad::KEY_S))
//		{
//			//前へ
//			Pos += Forward * abilities.speed * elapsed_time;
//		}
//		if (gamePad.GetAxisLY() < -0.2f || (gamePad.GetButton() & GamePad::KEY_W))
//		{
//			//後ろへ
//			Pos += -Forward * abilities.speed * elapsed_time;
//		}
//
//	if (collision_mesh)
//			{
//				const float raycast_step_back = 1.0f;
//				const float raycast_step_up = 1.5f;
//				const float collision_radius = 0.5f;
//				const float raycast_lift_up = 1.75f;
//				//const float raycast_lift_up = 3.0f;
//		
//				const char* material_name = 0;
//				XMFLOAT3 intersection;
//				XMFLOAT4X4 world_transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
//				//ray picking to front
//				XMFLOAT3 ray_direction = XMFLOAT3(direction.x, direction.y, direction.z);
//				XMFLOAT3 ray_position = XMFLOAT3
//				(
//					position.x - raycast_step_back * ray_direction.x,
//					position.y + raycast_step_up,
//					position.z - raycast_step_back * ray_direction.z
//				);
//		
//				material_name = collision_mesh->ray_triangles_intersection(ray_position, ray_direction, world_transform, &intersection);
//				if (material_name)
//				{
//					float distance = (position.x - intersection.x) * (position.x - intersection.x) +
//						//(position.y - intersection.y) * (position.y - intersection.y) +
//						(position.z - intersection.z) * (position.z - intersection.z);
//		
//					if (distance < collision_radius)
//					{
//						position.x -= (collision_radius - distance) * ray_direction.x;
//						//position.y = intersection.y;
//						position.z -= (collision_radius - distance) * ray_direction.z;
//		
//						//velocity = XMFLOAT4(0, 0, 0, 0);
//						velocity.x = 0;
//						velocity.z = 0;
//		
//						state = STATE_IDLE;
//					}
//				}
//		
//				//ray picking to downward
//				material_name = collision_mesh->ray_triangles_intersection_downward(XMFLOAT3(position.x, position.y + raycast_lift_up, position.z), world_transform, &intersection);
//		
//				float landing_altitude = 1.0f;
//				if (velocity.y < 0 && position.y - intersection.y < landing_altitude)
//				{
//					if (state == STATE_JUMP_RISE || state == STATE_STEPBACK)
//					{
//						state = STATE_JUMP_FALL;
//					}
//				}
//		
//				if (position.y < intersection.y)
//				{
//					position.x = intersection.x;
//					position.y = intersection.y;
//					position.z = intersection.z;
//					position.w = 1;
//		
//					velocity.y = 0;
//		
//					if (state == STATE_JUMP_FALL)
//					{
//						state = STATE_LANDING;
//					}
//				}
//			}
//			
//	DirectX::XMStoreFloat4(&position, Pos);
//
//		    R = DirectX::XMMatrixIdentity();
//		    R.r[0] = X;
//		    R.r[1] = Y;
//		    R.r[2] = Z;
//		    S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
//		    T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
//			/*XMFLOAT4X4 c{ 1,0,0,0,
//		0,1,0,0,
//		0,0,1,0,
//		0,0,0,1 };*/
//		
//			//DirectX::XMMATRIX C = DirectX::XMLoadFloat4x4(&c);
//		    //XMStoreFloat4x4(&world_transform, C * S * R * T);
//		    XMStoreFloat4x4(&world_transform, S * R * T);
//		
//		    animate(elapsed_time);
//		
//		    last_state = state;
//}

void Player::update(Descartes::collision::mesh* collision_mesh, float elapsed_time)
{
	velocity.x = 0.0f;
	velocity.y = 0.0f;
	velocity.z = 0.0f;

	GamePad& gamePad = Input::Instance().GetGamePad();

	XMMATRIX S, R, T;

	XMVECTOR X, Y, Z;
	Y = XMVectorSet(0, 1, 0, 0);
	Z = XMVector3Normalize(XMLoadFloat4(&direction));
	X = XMVector3Cross(Y, Z);
	Y = XMVector3Normalize(XMVector3Cross(Z, X));

	float speed = 0.0f;
	float turn = 0;

	if (gamePad.GetAxisLX() > 0.2f || (gamePad.GetButton() & GamePad::KEY_L))
	{
		//to right
		command = COMMANDS::C_TURN_RIGHT;
	}
	else if (gamePad.GetAxisLX() > 0.2f || (gamePad.GetButton() & GamePad::KEY_J))
	{
		//to left
		command = COMMANDS::C_TURN_LEFT;
	}
	else if (gamePad.GetAxisLY() > 0.2f || (gamePad.GetButton() & GamePad::KEY_I))
	{
		//to front
		command = COMMANDS::C_RUN;
		
		//velocity.z += speed * direction.z;
	}
	else if (gamePad.GetButtonDown() & GamePad::BTN_B)
	{
		command = COMMANDS::C_JUMP;
	}
	else
	{
		//nothing
		command = COMMANDS::C_IDLE;
	}

	switch(command)
	{
	case COMMANDS::C_IDLE:
		switch(state)
		{
		case STATE_RUN:
			state = STATE_RUN_STOP;
			speed = 0;
			break;
		case STATE_ATTACK:
		case STATE_JUMP_RISE:
		case STATE_JUMP_FALL:
		case STATE_DEATH:
		case STATE_STEPBACK:
			break;
		default:
			state = STATE_IDLE;
		}
		break;

	case COMMANDS::C_RUN:
			switch (state)
			{
			case STATE_ATTACK:
			case STATE_JUMP_RISE:
			case STATE_JUMP_FALL:
				break;
			case STATE_LANDING:
			default:
				state = STATE_RUN;
			}
			if (state != STATE_ATTACK)
			{
				speed = abilities.speed;
			}
			break;

	case COMMANDS::C_ATTACK:
			state = STATE_ATTACK;
			speed = 0;
			break;
	case COMMANDS::C_JUMP:
			//if (state != STATE_JUMP_RISE)
		{
			state = STATE_JUMP_RISE;
			velocity.y = abilities.initial_jump_speed;
		}
		break;
		case COMMANDS::C_TURN_LEFT:
			turn = -abilities.turning;
			speed = abilities.speed;
			break;
		case COMMANDS::C_TURN_RIGHT:
			turn = abilities.turning;
			speed = abilities.speed;
			break;
		case COMMANDS::C_DEATH:
			state = STATE_DEATH;
			speed = 0;
			break;
		case COMMANDS::C_STEPBACK:
			if (state != STATE_STEPBACK)
			{
				state = STATE_STEPBACK;
				velocity.y = abilities.initial_jump_speed * 0.75f;
			}
			speed = -abilities.speed * 0.5f;
			break;
	}

	if (state == STATE_ATTACK)
	{
		speed = 0;
	}


	

	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
	Z = DirectX::XMVector4Transform(Z, R);
	XMStoreFloat4(&direction, Z);

	velocity.x = speed * direction.x;
	velocity.z = speed * direction.z;
	if (use_gravity)
		velocity.y -= gravity * elapsed_time;

	//position.x += velocity.x * elapsed_time;
	//position.y += velocity.y * elapsed_time;
	//position.z += velocity.z * elapsed_time;
	
    //velocity.x = speed * direction.x;
    //velocity.z = speed * direction.z;
	//velocity.y = 0;
	//position.y += velocity.y * elapsed_time;


	

	if (collision_mesh&&raycast)
	{
		const float raycast_step_back = 1.0f;
		const float raycast_step_up = 1.5f;
		const float collision_radius = 0.5f;
		const float raycast_lift_up = 1.75f;
		//const float raycast_lift_up = 3.0f;

		const char* material_name = 0;
		XMFLOAT3 intersection;
		XMFLOAT4X4 world_transform = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
		//ray picking to front
		XMFLOAT3 ray_direction = XMFLOAT3(direction.x, direction.y, direction.z);
		XMFLOAT3 ray_position = XMFLOAT3
		(
			position.x - raycast_step_back * ray_direction.x,
			position.y + raycast_step_up,
			position.z - raycast_step_back * ray_direction.z
		);

		material_name = collision_mesh->ray_triangles_intersection(ray_position, ray_direction, world_transform, &intersection);
		if (material_name)
		{
			float distance = (position.x - intersection.x) * (position.x - intersection.x) +
				//(position.y - intersection.y) * (position.y - intersection.y) +
				(position.z - intersection.z) * (position.z - intersection.z);

			if (distance < collision_radius)
			{
				position.x -= (collision_radius - distance) * ray_direction.x;
				//position.y = intersection.y;
				position.z -= (collision_radius - distance) * ray_direction.z;

				//velocity = XMFLOAT4(0, 0, 0, 0);
				velocity.x = 0;
				velocity.z = 0;

				state = STATE_IDLE;
			}
		}

		//ray picking to downward
		material_name = collision_mesh->ray_triangles_intersection_downward(XMFLOAT3(position.x, position.y + raycast_lift_up, position.z), world_transform, &intersection);

		float landing_altitude = 1.0f;
		if (velocity.y < 0 && position.y - intersection.y < landing_altitude)
		{
			if (state == STATE_JUMP_RISE || state == STATE_STEPBACK)
			{
				state = STATE_JUMP_FALL;
			}
		}

		if (position.y < intersection.y)
		{
			position.x = intersection.x;
			position.y = intersection.y;
			position.z = intersection.z;
			position.w = 1;

			velocity.y = 0;

			if (state == STATE_JUMP_FALL)
			{
				state = STATE_LANDING;
			}
		}
	}
	

    R = DirectX::XMMatrixIdentity();
    R.r[0] = X;
    R.r[1] = Y;
    R.r[2] = Z;
    S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
    T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	/*XMFLOAT4X4 c{ 1,0,0,0,
0,1,0,0,
0,0,1,0,
0,0,0,1 };*/

	//DirectX::XMMATRIX C = DirectX::XMLoadFloat4x4(&c);
    //XMStoreFloat4x4(&world_transform, C * S * R * T);
    XMStoreFloat4x4(&world_transform, S * R * T);

    animate(elapsed_time);

    last_state = state;

}

//int Player::raycast(Vector3& pos, Vector3& v, Vector3& out, Vector3& normal)
//{
//	int ret = -1; // ヒットした面番号
//	// 一番近いヒット面までの距離
//	float min_dist = 10000;
//
//	Vector3 v0;
//	Vector3 v1;
//	Vector3 v2;
//	Vector3 ray;
//	Vector3 l0, l1, l2;
//	Vector3 N;
//	Vector3 work;
//	Vector3 p;
//	Vector3 p0;
//	Vector3 N0;
//	Vector3 p1;
//	Vector3 N1;
//	Vector3 p2;
//	Vector3 N2;
//	// 全ポリゴンと判定
//	for (int n = 0; n < NumFaces; n++)
//	{
//		//n番目の三角形の0番目の点
//		int index = Indices[n * 3 + 0];
//		v0.x = Vertices[index].x;
//		v0.y = Vertices[index].y;
//		v0.z = Vertices[index].z;
//		//n番目の三角形の1番目の点
//		int index1 = Indices[n * 3 + 1];
//		v1.x = Vertices[index1].x;
//		v1.y = Vertices[index1].y;
//		v1.z = Vertices[index1].z;
//		//n番目の三角形の2番目の点
//		int index2 = Indices[n * 3 + 2];
//		v2.x = Vertices[index2].x;
//		v2.y = Vertices[index2].y;
//		v2.z = Vertices[index2].z;
//
//		// レイ方向
//		ray = v;
//		// 三角形の3辺
//		//Vector3 l0 = v1 - v0; // ０→１
//		//Vector3 l1 = v2 - v1; // １→２
//		//Vector3 l2 = v0 - v2; // ２→０
//		l0.x = v1.x - v0.x;
//		l0.y = v1.y - v0.y;
//		l0.z = v1.z - v0.z;
//		l1.x = v2.x - v1.x;
//		l1.y = v2.y - v1.y;
//		l1.z = v2.z - v1.z;
//		l2.x = v0.x - v2.x;
//		l2.y = v0.y - v2.y;
//		l2.z = v0.z - v2.z;
//
//		// 面法線(2辺の外積)
//
//		//Vector3::cross(N, l0, l1);
//		N.x = l0.y * l1.z - l0.z * l1.y;
//		N.y = l0.z * l1.x - l0.x * l1.z;
//		N.z = l0.x * l1.y - l0.y * l1.x;
//
//		N.Normalize(); // 正規化（長さ１）
//
//		// 法線をレイ方向に射影(内積)
//		float a;// = Vector3::dot(-N, ray);
//		a = -N.x * ray.x + -N.y * ray.y + -N.z * ray.z;
//
//		if (a <= 0) continue;
//
//		// 発射位置から3角形の一点を法線に射影
//		float b;// = Vector3::dot(-N, v0 - pos);
//
//		work.x = v0.x - pos.x;
//		work.y = v0.y - pos.y;
//		work.z = v0.z - pos.z;
//		b = -N.x * work.x + -N.y * work.y + -N.z * work.z;
//
//		// レイの長さ
//		float len = b / a;
//		if (len < 0) continue;
//		if (len > min_dist) continue;
//
//		// 交点
//		// = pos + ray * len;
//		p.x = pos.x + ray.x * len;
//		p.y = pos.y + ray.y * len;
//		p.z = pos.z + ray.z * len;
//
//		// 内点判定
//		// 3Dの外積で法線、内積で面法線との差
//		// = v0 - p;
//		p0.x = v0.x - p.x;
//		p0.y = v0.y - p.y;
//		p0.z = v0.z - p.z;
//
//		//Vector3::cross(N0, p0, l0);
//		N0.x = p0.y * l0.z - p0.z * l0.y;
//		N0.y = p0.z * l0.x - p0.x * l0.z;
//		N0.z = p0.x * l0.y - p0.y * l0.x;
//
//		float d0;// = Vector3::dot(N, N0);
//		d0 = N.x * N0.x + N.y * N0.y + N.z * N0.z;
//
//		// 法線が反対ならNG
//		if (d0 < 0) continue;
//
//		// = v1 - p;
//		p1.x = v1.x - p.x;
//		p1.y = v1.y - p.y;
//		p1.z = v1.z - p.z;
//
//
//		//Vector3::cross(N1, p1, l1);
//		N1.x = p1.y * l1.z - p1.z * l1.y;
//		N1.y = p1.z * l1.x - p1.x * l1.z;
//		N1.z = p1.x * l1.y - p1.y * l1.x;
//
//		float d1;// = Vector3::dot(N, N1);
//		d1 = N.x * N1.x + N.y * N1.y + N.z * N1.z;
//		// 法線が反対ならNG
//		if (d1 < 0) continue;
//
//		// = v2 - p;
//		p2.x = v2.x - p.x;
//		p2.y = v2.y - p.y;
//		p2.z = v2.z - p.z;
//
//
//		//Vector3::cross(N2, p2, l2);
//		N2.x = p2.y * l2.z - p2.z * l2.y;
//		N2.y = p2.z * l2.x - p2.x * l2.z;
//		N2.z = p2.x * l2.y - p2.y * l2.x;
//
//		float d2;// = Vector3::dot(N, N2);
//		d2 = N.x * N2.x + N.y * N2.y + N.z * N2.z;
//
//		// 法線が反対ならNG
//		if (d2 < 0) continue;
//
//		//	交点決定
//		out = p;
//		normal = N; // 法線も返す
//		min_dist = len; // 最短距離更新
//		ret = n; //n番目のポリゴンにヒット
//	}
//	return ret;
//}

//int Player::raycast_trs(Vector3& pos, Vector3& v, Vector3& out, Vector3& normal)
//{
//	Matrix m = transform;
//	Matrix im = transform;
//	im.inverse(); // 逆行列化(逆変換)
//	// レイを元素材に合わせる
//	Vector3 pos_i;
//	Vector3 v_i;
//
//	// posを元モデル合わせる = Transformの逆変換 (*逆行列) 
//	pos_i.x = pos.x * im._11 + pos.y * im._21 + pos.z * im._31 + 1 * im._41;
//	pos_i.y = pos.x * im._12 + pos.y * im._22 + pos.z * im._32 + 1 * im._42;
//	pos_i.z = pos.x * im._13 + pos.y * im._23 + pos.z * im._33 + 1 * im._43;
//	// vを元モデルに合わせる
//	v_i.x = v.x * im._11 + v.y * im._21 + v.z * im._31;// +0 * im._41;
//	v_i.y = v.x * im._12 + v.y * im._22 + v.z * im._32;// +0 * im._42;
//	v_i.z = v.x * im._13 + v.y * im._23 + v.z * im._33;// +0 * im._43;
//
//	Vector3 out_i;
//	Vector3 normal_i;
//	int n = raycast(pos_i, v_i, out_i, normal_i);
//	// 結果を現状に合わせる
//	//out = out_i * m
//	out.x = out_i.x * m._11 + out_i.y * m._21 + out_i.z * m._31 + 1 * m._41;
//	out.y = out_i.x * m._12 + out_i.y * m._22 + out_i.z * m._32 + 1 * m._42;
//	out.z = out_i.x * m._13 + out_i.y * m._23 + out_i.z * m._33 + 1 * m._43;
//
//	//normal = normal_i * m
//	normal.x = normal_i.x * m._11 + normal_i.y * m._21 + normal_i.z * m._31;
//	normal.y = normal_i.x * m._12 + normal_i.y * m._22 + normal_i.z * m._32;
//	normal.z = normal_i.x * m._13 + normal_i.y * m._23 + normal_i.z * m._33;
//
//	return n;
//}


void Player::DebugDrawGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
	{
		//トランスフォーム
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::SliderFloat3("Position", &position.x,-100.0f,100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &scale.x);
			//Velocity
			ImGui::InputFloat3("Velocity", &velocity.x);
			//gavity
			ImGui::InputFloat("Gravity", &gravity);
			//angle
			ImGui::SliderFloat("Angle", &angle,0,360);
			//gravity
			if(ImGui::Button("UseGravity"))
			{
			    use_gravity=!use_gravity;
			}
		}
		if (ImGui::Button("RayCast"))
		{
			raycast = !raycast;
		}
	}
	ImGui::End();
#endif
}

