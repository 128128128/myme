#include "terrain.h"
#include "../../imgui/imgui.h"

//Terrain::Terrain(ID3D11Device* device)
//{
//	terrain = std::make_unique<terrain_mesh>(device, ".\\resources\\enviroments\\terrains\\terrain_0829.fbx");
//	vs = std::make_unique<vertex_shader<terrain_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
//	ps = std::make_unique<pixel_shader>(device, "shader//terrain_ps.cso");
//	terrain_constants_buffer = std::make_unique<Descartes::constant_buffer<terrain_constants>>(device);
//
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\MuddyGround_Albedo.tif", diffuse2_shader_resource_view.GetAddressOf(), true, true);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\Moss_Normal1.tif", normal1_shader_resource_view.GetAddressOf(), false, false);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\Moss_Normal.tif", normal1_detail_shader_resource_view.GetAddressOf(), true, true);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\MuddyGround_normal.tif", normal2_shader_resource_view.GetAddressOf(), true, true);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\Moss_MetallicSmooth.tif", metasmooth1_shader_resource_view.GetAddressOf(), true, true);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\MuddyGround_MetallicSmooth.tif", metasmooth2_shader_resource_view.GetAddressOf(), true, true);
//	
//	//rocks
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\SmallStones_Albedo.tif", rock_diffuse_shader_resource_view.GetAddressOf(), true, true);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\SmallStones_Normal.tif", rock_normal_shader_resource_view.GetAddressOf(), true, true);
//	load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\SmallStones_MetallicSmooth.tif", rock_metasmooth_shader_resource_view.GetAddressOf(), true, true);
//
//	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
//
//	float angle = 0;
//	direction.x = sinf(angle * 0.01745f);
//	direction.y = 0;
//	direction.z = cosf(angle * 0.01745f);
//}
//
//void Terrain::update(float elapsed_time)
//{
//	XMVECTOR X, Y, Z;
//	Y = XMVectorSet(0, 1, 0, 0);
//	Z = XMVector3Normalize(XMLoadFloat4(&direction));
//	X = XMVector3Cross(Y, Z);
//	Y = XMVector3Normalize(XMVector3Cross(Z, X));
//
//
//	XMMATRIX S, R, T;
//	float turn = 0;
//	R = DirectX::XMMatrixRotationAxis(Y, turn * 0.01745f * elapsed_time);
//	Z = DirectX::XMVector4Transform(Z, R);
//	XMStoreFloat4(&direction, Z);
//
//	position.x += velocity.x * elapsed_time;
//	position.y += velocity.y * elapsed_time;
//	position.z += velocity.z * elapsed_time;
//
//	R = DirectX::XMMatrixIdentity();
//	R.r[0] = X;
//	R.r[1] = Y;
//	R.r[2] = Z;
//	S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
//	T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
//	XMStoreFloat4x4(&world_transform, S * R * T);
//
//}
//
//void Terrain::render(ID3D11DeviceContext* immediate_context, Descartes::view_frustum view_frustum)
//{
//	ps->active(immediate_context);
//	vs->active(immediate_context);
//	terrain_constants_buffer->active(immediate_context,3);
//
//
//	//ground
//	immediate_context->PSSetShaderResources(5, 1, diffuse2_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(6, 1, normal1_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(7, 1, normal1_detail_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(8, 1, normal2_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(9, 1, metasmooth1_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(10, 1, metasmooth2_shader_resource_view.GetAddressOf());
//	//rocks
//	immediate_context->PSSetShaderResources(11, 1, rock_diffuse_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(12, 1, rock_normal_shader_resource_view.GetAddressOf());
//	immediate_context->PSSetShaderResources(13, 1, rock_metasmooth_shader_resource_view.GetAddressOf());
//
//	terrain->render(immediate_context, world_transform,view_frustum);
//	ps->inactive(immediate_context);
//	vs->inactive(immediate_context);
//	terrain_constants_buffer->inactive(immediate_context);
//}
//
//void Terrain::DebugDrawGUI()
//{
//#ifdef _DEBUG
//	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
//	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);
//
//	if (ImGui::Begin("Terrain", nullptr, ImGuiWindowFlags_None))
//	{
//		//トランスフォーム
//		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			//位置
//			ImGui::SliderFloat3("Position", &this->position.x, -100.0f, 100.0f);
//			//Scale
//			ImGui::InputFloat3("Scale", &this->scale.x);
//		}
//		if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			ImGui::SliderFloat("Blend01", &terrain_constants_buffer->data.Falloff01, 0.0f, 1.0f);
//			ImGui::SliderFloat("Blend02", &terrain_constants_buffer->data.Falloff02, 0.0f, 1.0f);
//			ImGui::SliderFloat("TextureScale01", &terrain_constants_buffer->data.TextureScale01, 0.0f, 1.0f);
//			ImGui::SliderFloat("TextureScale02", &terrain_constants_buffer->data.TextureScale02, 0.0f, 1.0f);
//			ImGui::SliderFloat("StoneTextureScale03", &terrain_constants_buffer->data.TextureScale03, 0.0f, 1.0f);
//
//		}
//	}
//	ImGui::End();
//#endif
//}

Terrain::Terrain(ID3D11Device* device, bool shadow_reciever)
{
	terrain = std::make_unique<terrain_mesh>(device, ".\\resources\\enviroments\\terrains\\stage_terrain.fbx",shadow_reciever);
	vs = std::make_unique<vertex_shader<terrain_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//terrain_ps.cso");

	velocity = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	float angle = 0;
	direction.x = sinf(angle * 0.01745f);
	direction.y = 0;
	direction.z = cosf(angle * 0.01745f);
}

void Terrain::update(float elapsed_time)
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

void Terrain::render(ID3D11DeviceContext* immediate_context, Descartes::view_frustum view_frustum)
{
	ps->active(immediate_context);
	vs->active(immediate_context);
	
	terrain->render(immediate_context, world_transform, view_frustum);
	ps->inactive(immediate_context);
	vs->inactive(immediate_context);
}

void Terrain::DebugDrawGUI()
{
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
	}
	ImGui::End();
}