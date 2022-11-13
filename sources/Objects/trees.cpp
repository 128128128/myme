#include "trees.h"
#include "../../imgui/imgui.h"

Trees::Trees(ID3D11Device* device)
{
	trees = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\trees\\tree_B.fbx");
	trunks = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\trees\\tree_T.fbx");
	vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//tree_branch_vs.cso");
	trunk_vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//tree_branch_ps.cso");
	trunk_ps = std::make_unique<pixel_shader>(device, "shader//rocks_ps.cso");

	load_texture_from_file(device, ".\\resources\\sprite\\noise\\FAE_Windvectors.png", vector_map.GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\enviroments\\trees\\Tree_leaves1.tga", leaves_map.GetAddressOf(), true, true);
	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);

	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Trees::update(float elapsed_time)
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

void Trees::render(ID3D11DeviceContext* immediate_context)
{
	constant_buffer->active(immediate_context, 5, true, true);

	ps->active(immediate_context);
	vs->active(immediate_context);
	immediate_context->PSSetShaderResources(4, 1, vector_map.GetAddressOf());
	immediate_context->VSSetShaderResources(4, 1, vector_map.GetAddressOf());
	immediate_context->PSSetShaderResources(5, 1, leaves_map.GetAddressOf());
	trees->render(immediate_context, world_transform);
	ps->inactive(immediate_context);
	vs->inactive(immediate_context);

	trunk_ps->active(immediate_context);
	trunk_vs->active(immediate_context);
	trunks->render(immediate_context, world_transform);
	trunk_ps->inactive(immediate_context);
	trunk_vs->inactive(immediate_context);

	constant_buffer->inactive(immediate_context);

}

void Trees::DebugDrawGUI()
{
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Trees", nullptr, ImGuiWindowFlags_None))
	{
		//Transform
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//Position
			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
			//Scale
			ImGui::InputFloat3("Scale", &this->scale.x);
		}
		//Wind Param
		if (ImGui::CollapsingHeader("Wind", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderFloat("max_wind_strength", & constant_buffer->data.max_wind_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("wind_strength", & constant_buffer->data.wind_strength, 0.0f, 1.0f);
			ImGui::SliderFloat("wind_speed", & constant_buffer->data.wind_speed, 0.0f, 1.0f);
			ImGui::SliderFloat3("wind_direction", & constant_buffer->data.wind_direction.x, -1.0f, 1.0f);
			ImGui::SliderFloat("wind_amplitude", & constant_buffer->data.wind_amplitude, 0.0f, 5.0f);
			ImGui::SliderFloat("wind_amplitude_multiplier", & constant_buffer->data.wind_amplitude_multiplier, 0.0f, 10.0f);
			ImGui::SliderFloat("flat_lighting", & constant_buffer->data.flat_lighting, 0.0f, 1.0f);
			ImGui::SliderFloat("wind_debug", & constant_buffer->data.wind_debug, 0.0f, 1.0f);
			ImGui::SliderFloat("mask", & constant_buffer->data.mask, 0.15f, 2.0f);
		}
	}
	ImGui::End();
}