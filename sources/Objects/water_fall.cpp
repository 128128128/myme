#include "water_fall.h"
#include "../../imgui/imgui.h"

Water_Fall::Water_Fall(ID3D11Device* device)
{
	water_fall = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\water_fall\\stage_water_fall.fbx");
	vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//water_fall_ps.cso");

	water_fall_constants_buffer = std::make_unique<Descartes::constant_buffer<water_fall_constants>>(device);

load_texture_from_file(device, ".\\resources\\enviroments\\water_fall\\FAE_Waterfall_Shadermap.dds", srv_shader_map.GetAddressOf(), true, true);
load_texture_from_file(device, ".\\resources\\sprite\\noise\\noise2.png", noise_map_1.GetAddressOf(), true, true);
load_texture_from_file(device, ".\\resources\\sprite\\noise\\noise3.png", noise_map_2.GetAddressOf(), true, true);

	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Water_Fall::update(float elapsed_time)
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

void Water_Fall::render(ID3D11DeviceContext* immediate_context)
{
	immediate_context->PSSetShaderResources(4, 1, srv_shader_map.GetAddressOf());
	immediate_context->PSSetShaderResources(5, 1, noise_map_1.GetAddressOf());
	immediate_context->PSSetShaderResources(6, 1, noise_map_2.GetAddressOf());

	ps->active(immediate_context);
	vs->active(immediate_context);
	water_fall_constants_buffer->active(immediate_context,3);
	water_fall->render(immediate_context, world_transform);
	ps->inactive(immediate_context);
	vs->inactive(immediate_context);
}

void Water_Fall::DebugDrawGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Terrain", nullptr, ImGuiWindowFlags_None))
	{
		//transform
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//position
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
		//constant buffer
		if (ImGui::CollapsingHeader("Texture", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//ˆÊ’u
			ImGui::SliderFloat("Speed", &water_fall_constants_buffer->data.speed, -1.0f, 1.0f);
			//Scale
			ImGui::SliderFloat("Offset", &water_fall_constants_buffer->data.offset,0.0f,1.0f);
			ImGui::Image(srv_shader_map.Get(), { 256,256 });
			ImGui::Image(noise_map_1.Get(), { 256,256 });
			ImGui::Image(noise_map_2.Get(), { 256,256 });
		}
	}
	ImGui::End();
#endif
}