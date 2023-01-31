#include "terrain.h"

#include <random>

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


grass_cluster::grass_cluster(ID3D11Device* device, const char* filename)
{
	substance = std::make_unique<Descartes::substance<vertex>>(filename, false, -1);
	substance->create_render_objects(device, true);

	char media_directory[256];
	strcpy_s(media_directory, filename);
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	D3D11_INPUT_ELEMENT_DESC input_element_desc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TRANSFORM", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		{ "TRANSFORM", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	};
	create_vs_from_cso(device, "shader//grass_cluster_vs.cso", vertex_shader.GetAddressOf(), input_layout.GetAddressOf(), input_element_desc, _countof(input_element_desc));
	create_ps_from_cso(device, "shader//grass_cluster_ps.cso", pixel_shader.GetAddressOf());

	rasterizer_state = std::make_unique<Descartes::rasterizer_state>(device, D3D11_FILL_SOLID, D3D11_CULL_NONE, false/*front_counter_clockwise*/, false/*antialiasing*/, true/*depth_clip_enable*/);
	blend_state = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA);
}

//#define OPTIMIZATION
void grass_cluster::create_instance_buffer(ID3D11Device* device)
{
	if (grasses.size() == 0)
	{
		return;
	}

	const float scale = 0.025f;
	for (const grass& grass : grasses)
	{
		DirectX::XMFLOAT3 pos = grass.position;
		pos.y += 1.0f;
		instance instance;
		XMStoreFloat4x4(&instance.transform, XMMatrixTranspose(XMMatrixScaling(grass.dimension.x, grass.dimension.y, grass.dimension.z) * XMMatrixTranslation(pos.x, pos.y, pos.z)));
		instances.push_back(instance);
	}

	D3D11_BUFFER_DESC buffer_desc = {};
	D3D11_SUBRESOURCE_DATA subresource_data = {};
	buffer_desc.ByteWidth = static_cast<uint32_t>(sizeof(instance) * instances.size());
#ifdef OPTIMIZATION 
	buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE; // D3D11_CPU_ACCESS_WRITE 0
#else
	buffer_desc.Usage = D3D11_USAGE_DEFAULT;
	buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	buffer_desc.CPUAccessFlags = 0;
#endif
	buffer_desc.MiscFlags = 0;
	buffer_desc.StructureByteStride = 0;
	subresource_data.pSysMem = &(instances.at(0));
	subresource_data.SysMemPitch = 0;
	subresource_data.SysMemSlicePitch = 0;
	HRESULT hr = device->CreateBuffer(&buffer_desc, &subresource_data, instance_buffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
}
void grass_cluster::render
(
	ID3D11DeviceContext* immediate_context,
	const XMFLOAT4X4& view,
	float draw_distance,
	float elapsed_time
)
{
	immediate_context->VSSetShader(vertex_shader.Get(), 0, 0);
	immediate_context->PSSetShader(pixel_shader.Get(), 0, 0);

	immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	immediate_context->IASetInputLayout(input_layout.Get());

	rasterizer_state->active(immediate_context);
	blend_state->active(immediate_context);

#ifdef OPTIMIZATION 
	D3D11_MAP map = D3D11_MAP_WRITE_DISCARD;
	D3D11_MAPPED_SUBRESOURCE mapped_buffer;
	HRESULT hr = immediate_context->Map(instance_buffer.Get(), 0, map, 0, &mapped_buffer);
	_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));

	// Extract camera's position and direction from view matrix.
	XMVECTOR P;	// camera_position
	XMVECTOR D;	// camera_direction
	XMMATRIX T = XMMatrixInverse(0, XMLoadFloat4x4(&view));
	P = T.r[3];
	D = T.r[2];

	instance* mapped_instances = static_cast<instance*>(mapped_buffer.pData);
	size_t number_of_instances = 0;

	size_t number_of_grasses = grasses.size();
	for (size_t index_of_grass = 0; index_of_grass < number_of_grasses; index_of_grass++)
	{
		const grass& grass = grasses.at(index_of_grass);

		// Optimaizations
		XMVECTOR G = XMLoadFloat3(&grass.position) - P;
		if (XMVectorGetByIndex(XMVector3Dot(G, D), 0) < 0)
		{
			continue;
		}
		//const float draw_distance = 50;
		if (XMVectorGetByIndex(XMVector3LengthSq(G), 0) > draw_distance * draw_distance)
		{
			continue;
		}

		mapped_instances[number_of_instances].transform = instances.at(index_of_grass).transform;
		number_of_instances++;
	}
	immediate_context->Unmap(instance_buffer.Get(), 0);
#endif

	for (Descartes::substance<vertex>::mesh& mesh : substance->meshes)
	{
		uint32_t strides[2] = { sizeof(vertex), sizeof(instance) };
		uint32_t offsets[2] = { 0, 0 };
		ID3D11Buffer* vbs[2] = { mesh.vertex_buffer.Get(), instance_buffer.Get() };
		immediate_context->IASetVertexBuffers(0, 2, vbs, strides, offsets);
		immediate_context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		for (Descartes::subset& subset : mesh.subsets)
		{
			const Descartes::material* material = substance->find_material(subset.name.c_str());
			immediate_context->PSSetShaderResources(0, 1, material->properties.at(0).shader_resource_view.GetAddressOf());
#ifdef OPTIMIZATION 
			immediate_context->DrawInstanced(subset.index_count, number_of_instances, subset.index_start, 0);
#else
			immediate_context->DrawInstanced(subset.index_count, static_cast<uint32_t>(grasses.size()), subset.index_start, 0);
#endif
		}
	}
	rasterizer_state->inactive(immediate_context);
	blend_state->inactive(immediate_context);
}

Terrain::Terrain(ID3D11Device* device, bool shadow_reciever)
{
	terrain = std::make_unique<terrain_mesh>(device, ".\\resources\\enviroments\\terrains\\stage_terrain_1119.fbx",shadow_reciever);


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

void Terrain::DebugDrawGUI(bool flag)
{
	if (flag) {
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
}


grass_Terrain::grass_Terrain(ID3D11Device* device)
{
	substance = std::make_unique<Descartes::substance<vertex>>(".\\resources\\enviroments\\terrains\\stage_terrain_1119_1.fbx", true);

	meadow = std::make_unique<grass_cluster>(device, ".\\resources\\enviroments\\terrains\\1206_grass_short_set.fbx");
	//pampas = std::make_unique<grass_cluster>(device, ".\\resources\\enviroments\\terrains\\1206_grass_short_set.fbx");
	flower = std::make_unique<grass_cluster>(device, ".\\resources\\enviroments\\terrains\\0227_flower.fbx");

	//load_texture_from_file(device, ".\\resources\\enviroments\\terrains\\grass_brown.png", pampas->substance->materials.at("grass_short_mtl").properties.at(0).shader_resource_view.ReleaseAndGetAddressOf(), true/*force_srgb*/, true/*enable_caching*/);

	char media_directory[256];
	strcpy_s(media_directory, ".\\resources\\enviroments\\terrains\\stage_terrain_1119.fbx");
	PathFindFileNameA(media_directory)[0] = '\0';
	substance->create_shader_resource_views(device, media_directory);

	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);

	blend_states[NONE] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::NONE);
	blend_states[ALPHA] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA);
	blend_states[ADD] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ADD);
	blend_states[ALPHA_TO_COVERAGE] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA_TO_COVERAGE);

	rasterizer_state = std::make_unique<Descartes::rasterizer_state>(device, D3D11_FILL_SOLID, D3D11_CULL_NONE, true/*front_counter_clockwise*/, true/*antialiasing*/, true/*depth_clip_enable*/);

	plant("Ground", meadow->grasses, 100, 0.022f, 0.018f, 0.25f);

	//plant("Ground", pampas->grasses, 150, 0.05f, 0.025f, 0.15f);


	meadow->create_instance_buffer(device);
	//pampas->create_instance_buffer(device);
	flower->create_instance_buffer(device);

	//this should be called after calling 'plant' method 
	substance->create_render_objects(device, true);

	vs = std::make_unique<vertex_shader<terrain_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
	ps = std::make_unique<pixel_shader>(device, "shader//terrain_ps.cso");
}

void grass_Terrain::render(ID3D11DeviceContext* immediate_context)
{
	vs->active(immediate_context);
	ps->active(immediate_context);
	for (Descartes::substance<vertex>::mesh& mesh : substance->meshes)
	{
		//if (mesh.name == "trees_mdl" || mesh.name == "grasses_mdl" || mesh.name == "bigtree_mdl")
		//{
			blend_states[ALPHA_TO_COVERAGE]->active(immediate_context);
		//}
		uint32_t stride = sizeof(vertex);
		uint32_t offset = 0;

		immediate_context->IASetVertexBuffers(0, 1, mesh.vertex_buffer.GetAddressOf(), &stride, &offset);
		immediate_context->IASetIndexBuffer(mesh.index_buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		immediate_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		XMMATRIX W = XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * XMMatrixTranslation(translation.x, translation.y, translation.z);
		XMStoreFloat4x4
		(
			&constant_buffer->data.world,
			XMMatrixTranspose
			(
				XMLoadFloat4x4(&mesh.global_transform) *
				XMLoadFloat4x4(&substance->system_unit_transform) *
				XMLoadFloat4x4(&substance->axis_system_transform) *
				W
			)
		);

		for (Descartes::subset& subset : mesh.subsets)
		{
			const Descartes::material* material = substance->find_material(subset.name.c_str());
			constant_buffer->data.options.z = 1.0f; //z:alpha
			constant_buffer->data.options.w = 0.0f; //w:skymap reflection factor

			constant_buffer->data.options.x = material->properties.at(0).u_scale;
			constant_buffer->data.options.y = material->properties.at(0).v_scale;

			constant_buffer->active(immediate_context, 0, true, true);

			immediate_context->PSSetShaderResources(0, 1, material->properties.at(0).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(1, 1, material->properties.at(1).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(2, 1, material->properties.at(2).shader_resource_view.GetAddressOf());
			immediate_context->PSSetShaderResources(3, 1, material->properties.at(3).shader_resource_view.GetAddressOf());
		    immediate_context->PSSetShaderResources(4, 1, material->properties.at(4).shader_resource_view.GetAddressOf());
			immediate_context->DrawIndexed(subset.index_count, subset.index_start, 0);


			constant_buffer->inactive(immediate_context);
		}

		//if (mesh.name == "trees_mdl" || mesh.name == "grasses_mdl" || mesh.name == "bigtree_mdl")
		//{
			blend_states[ALPHA_TO_COVERAGE]->inactive(immediate_context);
		//}
	}
	vs->inactive(immediate_context);
	ps->inactive(immediate_context);
}
void grass_Terrain::plant(const char* mesh_name, std::vector<grass_cluster::grass>& grasses, int pitch_cm, float scale_w, float scale_h, float noise/*ratio for height*/)
{
	std::vector<grass_cluster::grass> provisional_grasses;

	if (PathFileExistsA((std::string(mesh_name) + "." + std::to_string(pitch_cm) + ".locations.bin").c_str()))
	{
		std::ifstream ifs;
		ifs.open((std::string(mesh_name) + "." + std::to_string(pitch_cm) + ".locations.bin").c_str(), std::ios::binary);
		cereal::BinaryInputArchive i_archive(ifs);
		i_archive(provisional_grasses);
	}
	else
	{
		std::default_random_engine generator(0);
		std::uniform_real_distribution<float> distribution(-1.0f, +1.0f);

		const Descartes::substance<vertex>::mesh* mesh = substance->find_mesh(mesh_name);

		XMMATRIX W = XMMatrixScaling(scaling.x, scaling.y, scaling.z) * XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z) * XMMatrixTranslation(translation.x, translation.y, translation.z);
		W = XMMatrixTranspose
		(
			XMLoadFloat4x4(&mesh->global_transform) *
			XMLoadFloat4x4(&substance->system_unit_transform) *
			XMLoadFloat4x4(&substance->axis_system_transform) *
			W
		);
		XMMATRIX T = XMMatrixInverse(0, W);

		XMFLOAT3 bounding_box_min = mesh->bounding_box.min;
		XMFLOAT3 bounding_box_max = mesh->bounding_box.max;

		float pitch_m = pitch_cm * 0.01f; // metric units in world space
		XMFLOAT3 pitch;
		DirectX::XMStoreFloat3(&pitch, XMVector3Transform(XMVectorSet(pitch_m, 0, pitch_m, 0), T)); // transform to model space
		pitch.x = fabsf(pitch.x);
		pitch.z = fabsf(pitch.z);
		XMFLOAT3 fluctuation(pitch.x * 1.0f, 0, pitch.z * 1.0f); // model space
		for (float x = bounding_box_min.x + pitch.x; x < bounding_box_max.x - pitch.x; x += pitch.x)
		{
			for (float z = bounding_box_min.z + pitch.z; z < bounding_box_max.z - pitch.z; z += pitch.z)
			{
				XMFLOAT3 intersection;

				XMFLOAT3 ray_position_in_model_space(x + distribution(generator) * fluctuation.x, bounding_box_max.y + 100, z + distribution(generator) * fluctuation.z);
				XMFLOAT3 ray_direction_in_model_space(0, -1, 0);

				int index_of_intersected_triangle = Descartes::collision::ray_triangles_intersection
				(
					&(mesh->vertices.at(0).position.x),
					sizeof(vertex),
					&(mesh->indices.at(0)),
					mesh->indices.size(),
					ray_position_in_model_space,
					ray_direction_in_model_space,
					FLT_MAX,
					&intersection
				);
				if (index_of_intersected_triangle > -1)
				{
					grass_cluster::grass grass;
					DirectX::XMStoreFloat3(&intersection, XMVector3Transform(XMLoadFloat3(&intersection), W));
					grass.position = intersection;
					provisional_grasses.push_back(grass);
				}
			}
		}
		std::ofstream ofs;
		ofs.open((std::string(mesh_name) + "." + std::to_string(pitch_cm) + ".locations.bin").c_str(), std::ios::binary);
		cereal::BinaryOutputArchive o_archive(ofs);
		o_archive(provisional_grasses);
	}

	static std::default_random_engine generator(0);
	std::uniform_real_distribution<float> distribution(0.0f, scale_h * noise);
	for (grass_cluster::grass& grass : provisional_grasses)
	{
		grass.dimension.x = scale_w + distribution(generator);
		grass.dimension.z = scale_w + distribution(generator);
		grass.dimension.y = scale_h + distribution(generator);
		grasses.push_back(grass);
	}
}