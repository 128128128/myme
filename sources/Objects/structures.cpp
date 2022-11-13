#include "structures.h"
#include "../imgui/imgui.h"

Structures::Structures(ID3D11Device* device)
{

	//mesh = std::make_unique<terrain_mesh>(device, ".\\resources\\enviroments\\structures\\new\\Structures.fbx");
	mesh = std::make_unique<terrain_mesh>(device, ".\\resources\\enviroments\\seaport\\seaport_0929.fbx");
	//mesh = std::make_unique<terrain_mesh>(device, ".\\resources\\enviroments\\stage_bi.fbx");
	//mesh = std::make_unique<terrain_mesh>(device, ".\\resources\\factory\\factory_0929.fbx");

	vs = std::make_unique<vertex_shader<terrain_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//structures_ps.cso");
	structures_constants_buffer = std::make_unique<Descartes::constant_buffer<structures_constants>>(device);
	
	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Structures::update(float elapsed_time)
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

void Structures::render(ID3D11DeviceContext* immediate_context, Descartes::view_frustum view_frustum)
{
	ps->active(immediate_context);
	vs->active(immediate_context);
	structures_constants_buffer->active(immediate_context, 3);
	
	mesh->render(immediate_context, world_transform,view_frustum);

	ps->inactive(immediate_context);
	vs->inactive(immediate_context);
	structures_constants_buffer->inactive(immediate_context);
}

void Structures::DebugDrawGUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Structures", nullptr, ImGuiWindowFlags_None))
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
}