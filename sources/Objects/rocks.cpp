#include "rocks.h"
#include "../imgui/imgui.h"

Rocks::Rocks(ID3D11Device* device, bool shadow_reciever)
{
	
    /*rocks[0] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\RockChunk01.fbx");
    rocks[1] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\CliffBig02.fbx");
    rocks[2] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\CliffBig03.fbx");
    rocks[3] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\CliffEdge01.fbx");
    rocks[4] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\CliffEdge02.fbx");
    rocks[5] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\CliffEdge04.fbx");
    rocks[6] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\RockLedge.fbx");
    rocks[7] = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\new_Rocks\\RockSwamp01.fbx");*/
    rock = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\rocks\\stage_rocks.fbx",shadow_reciever);

	vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//rocks_ps.cso");
	//rocks_constants_buffer = std::make_unique<Descartes::constant_buffer<rocks_constants>>(device);


	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Rocks::update(float elapsed_time)
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

void Rocks::render(ID3D11DeviceContext* immediate_context)
{
	ps->active(immediate_context);
	vs->active(immediate_context);
	//rocks_constants_buffer->active(immediate_context, 3);
	
	rock->render(immediate_context, world_transform);

	ps->inactive(immediate_context);
	vs->inactive(immediate_context);
	//rocks_constants_buffer->inactive(immediate_context);
}

void Rocks::DebugDrawGUI(bool flag)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Rocks", nullptr, ImGuiWindowFlags_None))
	{
		//transform
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//position
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
		/*if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("Blend01", &rocks_constants_buffer->data.Falloff01, 0.0f, 1.0f);
			ImGui::SliderFloat("Blend02", &rocks_constants_buffer->data.Falloff02, 0.0f, 1.0f);
			ImGui::SliderFloat("TextureScale01", &rocks_constants_buffer->data.TextureScale01, 0.0f, 1.0f);
			ImGui::SliderFloat("TextureScale02", &rocks_constants_buffer->data.TextureScale02, 0.0f, 1.0f);
			ImGui::SliderFloat("StoneTextureScale03", &rocks_constants_buffer->data.TextureScale03, 0.0f, 1.0f);

		}*/
	}
	ImGui::End();
}