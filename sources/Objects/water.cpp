#include "water.h"
#include "../imgui/imgui.h"

Water::Water(ID3D11Device* device)
{
	water_plane = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\water\\water_1119.fbx");

	vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//water_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//water_ps.cso");

	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	load_texture_from_file(device, ".\\resources\\skymaps\\envmap_miramar\\miramar_bk.tga", sky_map.GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\enviroments\\water\\sea_normal.png", sea_normal_map.GetAddressOf(), true, true);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Water::update(float elapsed_time)
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

void Water::render(ID3D11DeviceContext* immediate_context)
{
	ps->active(immediate_context);
	vs->active(immediate_context);
	immediate_context->PSSetShaderResources(8, 1, sky_map.GetAddressOf());
	immediate_context->PSSetShaderResources(9, 1, sea_normal_map.GetAddressOf());
	water_plane->render(immediate_context, world_transform);

	ps->inactive(immediate_context);
	vs->inactive(immediate_context);
}

void Water::DebugDrawGUI(bool flag)
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Water", nullptr, ImGuiWindowFlags_None))
	{
		//transform
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//position
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
	}
	ImGui::End();
}
