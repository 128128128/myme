#include "objects.h"
#include "../Input/Input.h"
#include "../imgui/imgui.h"


Cube::Cube()
{

	position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Cube::update(float elapsed_time)
{
	XMVECTOR X, Y, Z;
	Y = XMVectorSet(0, 1, 0, 0);
	Z = XMVector3Normalize(XMLoadFloat4(&direction));
	X = XMVector3Cross(Y, Z);
	Y = XMVector3Normalize(XMVector3Cross(Z, X));


	XMMATRIX S, R, T;
	float speed = 0;
	float turn = 0;
	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
	Z = DirectX::XMVector4Transform(Z, R);
	XMStoreFloat4(&direction, Z);

	//velocity.x = speed * direction.x;
	//velocity.z = speed * direction.z;
	//velocity.y = 0;

	position.x += velocity.x * elapsed_time;
	position.y += velocity.y * elapsed_time;
	position.z += velocity.z * elapsed_time;

	R = DirectX::XMMatrixIdentity();
	R.r[0] = X;
	R.r[1] = Y;
	R.r[2] = Z;
	S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	XMStoreFloat4x4(&world_transform, S * R * T);

}

void Cube::DebugDrawGUI(bool flag)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("stage", nullptr, ImGuiWindowFlags_None))
	{
		//トランスフォーム
		if (ImGui::CollapsingHeader("StageTransform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			ImGui::InputFloat3("Position", &this->position.x);
		    //Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
	}
	ImGui::End();
}

pbr_Stage::pbr_Stage(ID3D11Device* device, const char* filename)
{
	mesh= std::make_unique<pbr_static_mesh>(device, filename);
	position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 180;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);

	pbr_constant_buffer = std::make_unique<Descartes::constant_buffer<pbr_constants>>(device);

}

void pbr_Stage::update(float elapsed_time)
{
	GamePad& gamePad = Input::Instance().GetGamePad();


	XMVECTOR X, Y, Z;
	Y = XMVectorSet(0, 1, 0, 0);
	Z = XMVector3Normalize(XMLoadFloat4(&direction));
	X = XMVector3Cross(Y, Z);
	Y = XMVector3Normalize(XMVector3Cross(Z, X));


	XMMATRIX S, R, T;
	float speed = 1;
	float turn = 0;
	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
	Z = DirectX::XMVector4Transform(Z, R);
	XMStoreFloat4(&direction, Z);

	if (gamePad.GetButton() & GamePad::KEY_I)
		velocity.x = speed * direction.x;
	else if (gamePad.GetButton() & GamePad::KEY_K)
		velocity.x = -speed * direction.x;


	if (gamePad.GetButton() & GamePad::KEY_L)
	velocity.z = speed * direction.z;
	else if (gamePad.GetButton() & GamePad::KEY_J)
		velocity.z = -speed * direction.z;



	velocity.y = 0;

	position.x += velocity.x * elapsed_time;
	position.y += velocity.y * elapsed_time;
	position.z += velocity.z * elapsed_time;

	R = DirectX::XMMatrixIdentity();
	R.r[0] = X;
	R.r[1] = Y;
	R.r[2] = Z;
	S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	XMStoreFloat4x4(&world_transform, S * R * T);

}

void pbr_Stage::DebugDrawGUI(bool flag)
{
	
	
	if (flag)
	{
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("pbr_obj", nullptr, ImGuiWindowFlags_None))
		{
			//Transform
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);

				ImGui::InputFloat3("Scale", &this->scale.x);
			}
			//pbr
			if (ImGui::CollapsingHeader("MetallicRoughness", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//RM
				ImGui::SliderFloat("Metallic", &pbr_constant_buffer->data.metallic, 0.0f, 1.0f);
				ImGui::SliderFloat("Roughness", &pbr_constant_buffer->data.roughness, 0.0f, 1.0f);
				//pure white
				ImGui::SliderFloat("pure_white", &pbr_constant_buffer->data.pure_white, 0.0f, +10.0f);

			}
		}
		ImGui::End();
	}
}


pbr_Base::pbr_Base(ID3D11Device* device, const char* filename)
{
	mesh = std::make_unique<pbr_static_mesh>(device, filename);
	position = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 180;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);

	pbr_constant_buffer = std::make_unique<Descartes::constant_buffer<pbr_constants>>(device);

}

void pbr_Base::update(float elapsed_time)
{
	XMVECTOR X, Y, Z;
	Y = XMVectorSet(0, 1, 0, 0);
	Z = XMVector3Normalize(XMLoadFloat4(&direction));
	X = XMVector3Cross(Y, Z);
	Y = XMVector3Normalize(XMVector3Cross(Z, X));


	XMMATRIX S, R, T;
	float speed = 0;
	float turn = 0;
	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
	Z = DirectX::XMVector4Transform(Z, R);
	XMStoreFloat4(&direction, Z);

	//velocity.x = speed * direction.x;
	//velocity.z = speed * direction.z;
	//velocity.y = 0;

	position.x += velocity.x * elapsed_time;
	position.y += velocity.y * elapsed_time;
	position.z += velocity.z * elapsed_time;

	R = DirectX::XMMatrixIdentity();
	R.r[0] = X;
	R.r[1] = Y;
	R.r[2] = Z;
	S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	XMStoreFloat4x4(&world_transform, S * R * T);

}

void pbr_Base::DebugDrawGUI(bool flag)
{


	if (flag)
	{
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("base", nullptr, ImGuiWindowFlags_None))
		{
			//Transform
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);

				ImGui::InputFloat3("Scale", &this->scale.x);
			}
			//pbr
			if (ImGui::CollapsingHeader("MetallicRoughness", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//RM
				ImGui::SliderFloat("Metallic", &pbr_constant_buffer->data.metallic, 0.0f, 1.0f);
				ImGui::SliderFloat("Roughness", &pbr_constant_buffer->data.roughness, 0.0f, 1.0f);
				//pure white
				ImGui::SliderFloat("pure_white", &pbr_constant_buffer->data.pure_white, 0.0f, +10.0f);

			}
		}
		ImGui::End();
	}
}

Ground::Ground()
{

	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Ground::update(float elapsed_time)
{
	XMVECTOR X, Y, Z;
	Y = XMVectorSet(0, 1, 0, 0);
	Z = XMVector3Normalize(XMLoadFloat4(&direction));
	X = XMVector3Cross(Y, Z);
	Y = XMVector3Normalize(XMVector3Cross(Z, X));


	XMMATRIX S, R, T;
	float turn = 0;
	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
	Z = DirectX::XMVector4Transform(Z, R);
	XMStoreFloat4(&direction, Z);

	position.x += velocity.x * elapsed_time;
	position.y += velocity.y * elapsed_time;
	position.z += velocity.z * elapsed_time;

	R = DirectX::XMMatrixIdentity();
	R.r[0] = X;
	R.r[1] = Y;
	R.r[2] = Z;
	S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	XMStoreFloat4x4(&world_transform, S * R * T);

}

void Ground::DebugDrawGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("ground", nullptr, ImGuiWindowFlags_None))
	{
		//トランスフォーム
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
	}
	ImGui::End();
#endif
}

VegetationSmall::VegetationSmall(ID3D11Device* device)
{
	vegetation = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\structures\\VegetationSmall01.fbx");
	vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//vegetation_ps.cso");
	load_texture_from_file(device, ".\\resources\\enviroments\\structures\\VegetationSmall01.fbm\\VegetationSmall01_Emission.tif", emissive_shader_resource_view.GetAddressOf(), true, true);
	constants_buffer = std::make_unique<Descartes::constant_buffer<constants>>(device);

	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void VegetationSmall::update(float elapsed_time)
{
	XMVECTOR X, Y, Z;
	Y = XMVectorSet(0, 1, 0, 0);
	Z = XMVector3Normalize(XMLoadFloat4(&direction));
	X = XMVector3Cross(Y, Z);
	Y = XMVector3Normalize(XMVector3Cross(Z, X));


	XMMATRIX S, R, T;
	float turn = 0;
	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
	Z = DirectX::XMVector4Transform(Z, R);
	XMStoreFloat4(&direction, Z);

	position.x += velocity.x * elapsed_time;
	position.y += velocity.y * elapsed_time;
	position.z += velocity.z * elapsed_time;

	R = DirectX::XMMatrixIdentity();
	R.r[0] = X;
	R.r[1] = Y;
	R.r[2] = Z;
	S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	XMStoreFloat4x4(&world_transform, S * R * T);

}

void VegetationSmall::render(ID3D11DeviceContext* immediate_context)
{
	ps->active(immediate_context);
	vs->active(immediate_context);
	constants_buffer->active(immediate_context, 4);
	immediate_context->PSSetShaderResources(5, 1, emissive_shader_resource_view.GetAddressOf());

	vegetation->render(immediate_context, world_transform);

	constants_buffer->inactive(immediate_context);
	ps->inactive(immediate_context);
	vs->inactive(immediate_context);
}

void VegetationSmall::DebugDrawGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Vegetation", nullptr, ImGuiWindowFlags_None))
	{
		//トランスフォーム
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
		if(ImGui::CollapsingHeader("Emissive", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::SliderFloat("Power", &constants_buffer->data.power, 0.0f, 100.0f);
			ImGui::SliderFloat3("Color", &constants_buffer->data.color.x, 0.0f, 255.0f);
		}
	}
	ImGui::End();
#endif
}
