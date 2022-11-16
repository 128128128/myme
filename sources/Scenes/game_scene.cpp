#include "game_scene.h"
#include "../Math/collision.h"

using namespace DirectX;

bool game_scene::initialize(ID3D11Device* device, CONST LONG screen_width, CONST LONG screen_height)
{
    //blend state
	blend_states[NONE] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::NONE);
	blend_states[ALPHA] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA);
	blend_states[ADD] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ADD);
	blend_states[ALPHA_TO_COVERAGE] = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ALPHA_TO_COVERAGE);

	//rasterizer state
	rasterizer_states[WIREFRAME] = std::make_unique<Descartes::rasterizer_state>(device, D3D11_FILL_WIREFRAME);
	rasterizer_states[SOLID] = std::make_unique<Descartes::rasterizer_state>(device, D3D11_FILL_SOLID);
	rasterizer_states[SOLID_CULL_NONE] = std::make_unique<Descartes::rasterizer_state>(device, D3D11_FILL_SOLID, D3D11_CULL_NONE);

	//sampler state
	sampler_states[POINT] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT);
	sampler_states[LINEAR] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR);
	sampler_states[ANISOTROPIC] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_ANISOTROPIC);

	//depthstencil states
	depth_stencil_states[ZT_ON_ZW_ON] = std::make_unique<Descartes::depth_stencil_state>(device, TRUE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
	depth_stencil_states[ZT_ON_ZW_OFF] = std::make_unique<Descartes::depth_stencil_state>(device, TRUE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS);
	depth_stencil_states[ZT_OFF_ZW_ON] = std::make_unique<Descartes::depth_stencil_state>(device, FALSE, D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_LESS);
	depth_stencil_states[ZT_OFF_ZW_OFF] = std::make_unique<Descartes::depth_stencil_state>(device, FALSE, D3D11_DEPTH_WRITE_MASK_ZERO, D3D11_COMPARISON_LESS);

			if (!player)
			{
				player = std::make_unique<Player>();
				// player->player=std::make_unique<dynamic_mesh>(device, ".\\resources\\nico.fbx");
				player->player = std::make_unique<dynamic_mesh>(device, ".\\resources\\rock_girl\\rock_girl.fbx", true);
				//player->player= std::make_unique<pbr_dynamic_mesh>(device, ".\\resources\\objects\\chest\\chest1.fbx", ".\\resources\\objects\\chest\\WoodChest_Wood_Chest_MetallicSmoothness.png");
				//pbr_ship = std::make_unique<pbr_Stage>(device);
				//pbr_ship_1 = std::make_unique<pbr_Stage>(device);
				//pbr_ship_1->position.z += 5.5f;
				//pbr_ship->position.z -= 5.5f;
			}

	//to factory model
			/*if (!ground)
			{
				ground = std::make_unique<Ground>();
				ground->mesh = std::make_unique<static_mesh>(device, ".\\resources\\factory\\factory_cut_0714.fbx");
			}*/
			//if (!collision_mesh)
			//{
			//	collision_mesh = std::make_unique<Descartes::collision::mesh>(".\\resources\\enviroments\\structures\\structures_collision.fbx");
			//	//collision_mesh = std::make_unique<Descartes::collision::mesh>(".\\resources\\enviroments\\terrains\\terrains_0813.fbx");
			//}


			if(!terrains)
			{
				terrains = std::make_unique<Terrain>(device,true);
			}

			if (!rocks)
			{
				rocks = std::make_unique<Rocks>(device,true);
			}
            if (!trees)
			{
				trees = std::make_unique<Trees>(device);
				shadow_trees = std::make_unique<Ground>();
				shadow_trees->mesh = std::make_unique<static_mesh>(device, ".\\resources\\enviroments\\trees\\tree_T.fbx");
			}
            if (!water_fall)
			{
				water_fall = std::make_unique<Water_Fall>(device);
			}
            if (!water)
			{
				water = std::make_unique<Water>(device);
			}
			/*if (!structures)
			{
				structures = std::make_unique<Structures>(device);
			}*/
			/*if(!vegetation_small)
			{
				vegetation_small = std::make_unique < VegetationSmall > (device);
			}*/
			//if (!sky)
			//{
			//	sky = std::make_unique<skydome>();
			//	//skydome->skydome = std::make_unique<static_mesh>(device, ".\\resources\\Model\\Skydome\\skydome.fbx");
			//	sky->Initialize(device);
			//}

			if (!eye_space_camera)
			{
				eye_space_camera = std::make_unique<camera>(device);
				
				//eye_space_camera->reset(player->position, player->direction, focal_length, 0.8f/*height_above_ground*/);
			}
			if (!light_space_camera)
			{
				light_space_camera = std::make_unique<camera>(device);
			}

    framebuffers[0] = std::make_unique<Descartes::framebuffer>(device, screen_width, screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
	framebuffers[1] = std::make_unique<Descartes::framebuffer>(device, screen_width, screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
	framebuffers[2] = std::make_unique<Descartes::framebuffer>(device, screen_width, screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	shadowmap = std::make_unique<Descartes::framebuffer>(device, 1024 * 5, 1024 * 5, DXGI_FORMAT_R32G32_FLOAT/*not needed*/, DXGI_FORMAT_R32_TYPELESS);
    shadowmap_df = std::make_unique <shadow_map>();
    shadowmap_df->initialize(device);
	DirectX::XMFLOAT3 dir = { 0.0f, -1.0f, 1.0f };
	screen = std::make_unique <gbuffer>();
	screen->initialize(device,dir);

	//effects
	{
		post_effects = std::make_unique<post_processing_effects>(device);
		post_effects->i_archive(".\\configurations\\post_processing_effects.ini");

		bloom_effect = std::make_unique<bloom>(device, screen_width, screen_height);
		bloom_effect->i_archive(".\\configurations\\bloom.ini");
	}
	rasterizer = std::make_unique<Descartes::rasterizer>(device);
	fullscreen_quad = std::make_unique<Descartes::fullscreen_quad>(device);

	white = std::make_unique<playshader>(device, "resources//sprite//noise//zatsu.png", true, true);

	//shader load
	{
		dynamic_mesh_vs = std::make_unique<vertex_shader<dynamic_mesh::vertex>>(device, "shader//dynamic_mesh_vs.cso");
		dynamic_mesh_ps = std::make_unique<pixel_shader>(device, "shader//dynamic_mesh_ps.cso");
		pbr_dynamic_mesh_vs = std::make_unique<vertex_shader<pbr_dynamic_mesh::vertex>>(device, "shader//pbr_dynamic_vs.cso");
		//pbr_dynamic_mesh_ps = std::make_unique<pixel_shader>(device, "shader//pbr_dynamic_ps.cso");
		pbr_dynamic_mesh_ps = std::make_unique<pixel_shader>(device, "shader//d_pbr_static_mesh_ps.cso");
		pbr_static_mesh_vs = std::make_unique<vertex_shader<pbr_static_mesh::vertex>>(device, "shader//pbr_static_vs.cso");
		static_mesh_vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//static_mesh_vs.cso");
		static_mesh_ps = std::make_unique<pixel_shader>(device, "shader//static_mesh_ps.cso");
		lim_light_vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//lim_light_vs.cso");
		lim_light_ps = std::make_unique<pixel_shader>(device, "shader//lim_light_ps.cso");
		spot_light_mesh_ps = std::make_unique<pixel_shader>(device, "shader//spot_light_ps.cso");
		point_light_mesh_ps = std::make_unique<pixel_shader>(device, "shader//point_light_ps.cso");
		hemi_light_ps = std::make_unique<pixel_shader>(device, "shader//hemisphere_light_ps.cso");
		skydome_vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//skymap_vs.cso");
		skydome_ps = std::make_unique<pixel_shader>(device, "shader//skymap_ps.cso");

		//shadowcast
		static_shadowcast_vs = std::make_unique<vertex_shader<static_mesh::vertex>>(device, "shader//static_shadowcast_vs.cso");
		dynamic_mesh_shadowcast_vs = std::make_unique<vertex_shader<dynamic_mesh::vertex>>(device, "shader//dynamic_shadowcast_vs.cso");
		void_ps = std::make_unique<pixel_shader>(device, nullptr);
	}

	//constant buffers
	{
		scene_constants_buffer = std::make_unique<Descartes::constant_buffer<scene_constants>>(device);
		light_constants_buffer = std::make_unique<Descartes::constant_buffer<light_constants>>(device);
		renderer_constants_buffer = std::make_unique<Descartes::constant_buffer<renderer_constants>>(device);

		d_shadow_constant_buffer = std::make_unique < Descartes::constant_buffer <d_shadow_param >> (device);
	}

	//particles
	{
		//XMFLOAT3 ppos{ player->position.x,player->position.y,player->position.z };
		//snowfall = std::make_unique<snow_particles>(device, ppos);
	}

	return true;
}

const char* game_scene::update(float& elapsed_time/*Elapsed seconds from last frame*/)
{
	
	//player->update(collision_mesh.get(),elapsed_time);
	scene_constants_buffer->data.player_object.position = player->position;
	scene_constants_buffer->data.player_object.direction = player->direction;

	//pbr_ship->update(elapsed_time);
	//pbr_ship_1->update(elapsed_time);
	//ground->update(elapsed_time);
	terrains->update(elapsed_time);
	rocks->update(elapsed_time);
	trees->update(elapsed_time);
	water_fall->update(elapsed_time);
	water->update(elapsed_time);
	//structures->update(elapsed_time);
	//vegetation_small->update(elapsed_time);
	//sky->update(elapsed_time);

	if (freelook)
	{
		eye_space_camera->freelook_update(elapsed_time, DirectX::XMFLOAT4(player->position.x, player->position.y + 2.0f, player->position.z + player->direction.z * 0.1f, 0));
	}
	else
	{
		//eye_space_camera->update(player->position, elapsed_time);
		eye_space_camera->firstperson_update(elapsed_time,DirectX::XMFLOAT4(player->position.x,player->position.y+2.0f ,player->position.z+player->direction.z*0.1f,0));
	}


	//posteffects
	//ImGui::Checkbox("enable_post_effects", &enable_post_effects);
	//ImGui::Checkbox("post_blooming", &post_blooming);
	//ImGui::Checkbox("lens_flare", &enable_lens_flare);

	//point light options
    /* ImGui::SliderFloat3("point_position", &light_constants_buffer->data.point_light.position.x, -100.0f, 100.0f);
    ImGui::SliderFloat("point_range", &light_constants_buffer->data.point_light.range, 0.0f, 100.0f);
	ImGui::ColorPicker4("point_color", &light_constants_buffer->data.point_light.color.x, 0);*/
	
	//spot light options
	//ImGui::SliderFloat3("spot_position", &light_constants_buffer->data.spot_light.position.x, -100.0f, 100.0f);
	//ImGui::SliderFloat3("spot_direction", &light_constants_buffer->data.spot_light.direction.x, -1.0f, 1.0f);
	//ImGui::SliderFloat("spot_range", &light_constants_buffer->data.spot_light.range, 0.0f, 500.0f);
	//ImGui::ColorPicker3("spot_color", &light_constants_buffer->data.spot_light.color.x, 0);
	//ImGui::SliderFloat("spot_angle", &light_constants_buffer->data.spot_light.angle, -180.0f,180.0f);

	//camera
	if (ImGui::Button("freeLook"))
		freelook = !freelook;


	return 0;
}

void game_scene::render(ID3D11DeviceContext* immediate_context, float elapsed_time/*Elapsed seconds from last frame*/)
{
	rasterizer_states[SOLID]->active(immediate_context);
	blend_states[ALPHA]->active(immediate_context);
	depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);

	sampler_states[POINT]->active(immediate_context, 0, false);
	sampler_states[LINEAR]->active(immediate_context, 1, false);
	sampler_states[ANISOTROPIC]->active(immediate_context, 2, false);

	//viewport setting
	D3D11_VIEWPORT viewport;
	UINT num_viewports{ 1 };
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	//camera setting
	float aspect_ratio{ viewport.Width / viewport.Height };

	time += elapsed_time;
	triple_speed_time += elapsed_time*3.0f;
	scene_constants_buffer->data.cb_somthing.iTime = time;
	scene_constants_buffer->data.cb_somthing.triple_speed_iTime = triple_speed_time;
	scene_constants_buffer->data.cb_somthing.elapse_time = elapsed_time;
	/*Vector3 light_dir;
	light_dir.vec = XMFLOAT3{ scene_constants_buffer->data.directional_light.direction.x,scene_constants_buffer->data.directional_light.direction.y,scene_constants_buffer->data.directional_light.direction.z };
	light_dir.v[0] = scene_constants_buffer->data.directional_light.direction.x;
	light_dir.v[1] = scene_constants_buffer->data.directional_light.direction.y;
	light_dir.v[2] = scene_constants_buffer->data.directional_light.direction.z;
	light_dir.x= scene_constants_buffer->data.directional_light.direction.x;
	light_dir.y= scene_constants_buffer->data.directional_light.direction.y;
	light_dir.z= scene_constants_buffer->data.directional_light.direction.z;

	m_cascadeShadowMapMatrix.CalcLightViewProjectionCropMatrix(light_dir,eye_space_camera->get_far_z(),eye_space_camera->get_near_z(),eye_space_camera->get_front(),eye_space_camera->get_right(), eye_space_camera->get_position());
	for (int i = 0; i < NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT; i++)
	{
		for (int areaNo = 0; areaNo < NUM_SHADOW_MAP; areaNo++)
		{
			scene_constants_buffer->data.directional_light.shadow_map_num = areaNo;
			scene_constants_buffer->data.directional_light.mlvp[i][areaNo] = m_cascadeShadowMapMatrix.GetLightViewProjectionCropMatrix(areaNo);
		}
	}*/

	scene_constants_buffer->active(immediate_context, 3, true, true);

	// generate shadowmap
	//{
	//	//shadowmap->clear(immediate_context);
	//	shadowmap_df->active(immediate_context);

	//	XMVECTOR L = XMVector3Normalize(XMLoadFloat4(&scene_constants_buffer->data.directional_light.direction));
	//	XMVECTOR P = XMLoadFloat4(&player->position);
	//	XMStoreFloat4(&light_space_camera->position, P  * L);
	//	XMStoreFloat4(&light_space_camera->focus, P);
	//	light_space_camera->perspective_projection = false; //orthographic projection
	//	light_space_camera->fovy_or_view_width = 100;
	//	light_space_camera->aspect_ratio = aspect_ratio;
	//	light_space_camera->near_z = 1.0f;
	//	light_space_camera->far_z = 100.0f;
	//	light_space_camera->active(immediate_context, 1, true, true,false,false);

	//	Descartes::view_frustum view_frustum(light_space_camera->view_projection(),light_space_camera->inverse_view_projection());
	//
	//	//void_ps->active(immediate_context);

	//	//dynamic_mesh_shadowcast_vs->active(immediate_context);
	//	//player->render(immediate_context);
	//	//dynamic_mesh_shadowcast_vs->inactive(immediate_context);

	//	static_shadowcast_vs->active(immediate_context);
	//	//pbr_ship->render(immediate_context);
	//	//pbr_ship_1->render(immediate_context);
	//	shadow_trees->render(immediate_context);
	//	//trees->render(immediate_context);
	//	static_shadowcast_vs->inactive(immediate_context);

	//	//structures->render(immediate_context,view_frustum);

	//	//void_ps->inactive(immediate_context);

	//	shadowmap_df->inactive(immediate_context);
	//}


	screen->active(immediate_context);

	eye_space_camera->aspect_ratio = aspect_ratio;
	eye_space_camera->active(immediate_context, 1, true, true,false,false);

	//view frustum
	const Descartes::view_frustum view_frustum(eye_space_camera->view_projection(),eye_space_camera->inverse_view_projection());

    //renderer_constants_buffer->active(immediate_context, 3, true, true);
	//light_constants_buffer->active(immediate_context, 4, true, true);

	d_shadow_constant_buffer->data.light_view_projection = light_space_camera->view_projection();
	d_shadow_constant_buffer->active(immediate_context, 10);
	//objects render
	{
		//sky
		white->render(immediate_context, 0, 0, 1280, 720, 1, 1, 1, 1, 0);
		//screen->set_color_resource(immediate_context, 8);
		//water
	    water->render(immediate_context);

		trees->render(immediate_context);

		//dynamic_mesh_vs->active(immediate_context);
		//dynamic_mesh_ps->active(immediate_context);
		//player->render(immediate_context);
		//test->render(immediate_context);
		//dynamic_mesh_vs->inactive(immediate_context);
		//dynamic_mesh_ps->inactive(immediate_context);

		//pbr
		/*pbr_dynamic_mesh_ps->active(immediate_context);
		pbr_static_mesh_vs->active(immediate_context);
		pbr_ship->render(immediate_context);
		pbr_static_mesh_vs->inactive(immediate_context);
		pbr_dynamic_mesh_ps->inactive(immediate_context);*/

		//skydome
		//skydome_vs->active(immediate_context);
		//skydome_ps->active(immediate_context);
		//sky->Render(immediate_context);
		//skydome_ps->inactive(immediate_context);
		//skydome_vs->inactive(immediate_context);

		//staticmesh render
		//static_mesh_vs->active(immediate_context);
		//static_mesh_ps->active(immediate_context);
		//pbr_ship_1->render(immediate_context);
		////ground->render(immediate_context);
		//static_mesh_ps->inactive(immediate_context);
		//static_mesh_vs->inactive(immediate_context);

		//terrain renderer
		terrains->render(immediate_context,view_frustum);
		//structures->render(immediate_context,view_frustum);
		//vegetation_small->render(immediate_context);

		rocks->render(immediate_context);
		water_fall->render(immediate_context);

		// Snowfall particles drawing process
		//depth_stencil_states[ZT_ON_ZW_ON]->inactive(immediate_context);
		//depth_stencil_states[ZT_ON_ZW_OFF]->active(immediate_context);
		//XMFLOAT3 pos{ player->position.x,player->position.y,player->position.z };
		//snowfall->integrate(immediate_context, pos, eye_space_camera->view(), eye_space_camera->projection());

		//snowfall->render(immediate_context);

		//depth_stencil_states[ZT_ON_ZW_OFF]->inactive(immediate_context);
		//depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);

		
		//point light mesh render
		/*static_mesh_vs->active(immediate_context);
		point_light_mesh_ps->active(immediate_context);
		pbr_stage->render(immediate_context);
		static_mesh_vs->inactive(immediate_context);
		point_light_mesh_ps->inactive(immediate_context);*/

		//spot light mesh render
		//#ifdef 0
	/*		static_mesh_vs->active(immediate_context);
			spot_light_mesh_ps->active(immediate_context);
			
			static_mesh_vs->inactive(immediate_context);
			spot_light_mesh_ps->inactive(immediate_context);*/

	//#endif

	//lim_light for static mesh
	//lim_light_vs->active(immediate_context);
	//lim_light_ps->active(immediate_context);
	//lim_light_vs->inactive(immediate_context);
	//lim_light_ps->inactive(immediate_context);

	//hemisphere light
	/*static_mesh_vs->active(immediate_context);
	hemi_light_ps->active(immediate_context);
	hemi_light_ps->inactive(immediate_context);
	static_mesh_vs->inactive(immediate_context);*/

	//sprite

		screen->inactive(immediate_context);
	
		//framebuffers[0]->inactive(immediate_context);

		if (enable_post_effects)
		{
			//rasterizer->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), 0, 0, 1680, 715);

			if (post_blooming)
			{
				//post effects (shadow, fog and bokeh)
				framebuffers[2]->active(immediate_context);
				post_effects->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), framebuffers[0]->depth_stencil_shader_resource_view.Get(), shadowmap->depth_stencil_shader_resource_view.Get(), light_space_camera->view_projection());
				framebuffers[2]->inactive(immediate_context);

				//generate bloom texture from scene framebuffer
				bloom_effect->generate(immediate_context, framebuffers[2]->render_target_shader_resource_view.Get(), enable_lens_flare);

				//convolute bloom texture to scene framebuffer
				framebuffers[2]->active(immediate_context);
				bloom_effect->blit(immediate_context);
				framebuffers[2]->inactive(immediate_context);
				//rasterizer->blit(immediate_context, bloomer->gaussian_blur->render_target_shader_resource_view.Get(), 0, 0, 1680, 715);
			}
			else
			{
				//generate bloom texture from scene framebuffer
				bloom_effect->generate(immediate_context, framebuffers[1]->render_target_shader_resource_view.Get(), enable_lens_flare);

				//convolute bloom texture to scene framebuffer
				//blend_states[ADD]->activate(immediate_context);
				framebuffers[1]->active(immediate_context);
				bloom_effect->blit(immediate_context);
				framebuffers[1]->inactive(immediate_context);
				//blend_states[ADD]->deactivate(immediate_context);
				//rasterizer->blit(immediate_context, bloomer->gaussian_blur->render_target_shader_resource_view.Get(), 0, 0, 1680, 715);

				//post effects (shadow, fog and bokeh)
				framebuffers[2]->active(immediate_context);
				post_effects->blit(immediate_context, framebuffers[1]->render_target_shader_resource_view.Get(), framebuffers[1]->depth_stencil_shader_resource_view.Get(), shadowmap->depth_stencil_shader_resource_view.Get(), light_space_camera->view_projection());
				framebuffers[2]->inactive(immediate_context);
			}
			rasterizer->blit(immediate_context, framebuffers[2]->render_target_shader_resource_view.Get(), 0, 0, viewport.Width, viewport.Height);
		}
		else
		{
			screen->render(immediate_context);
			//rasterizer->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), 0, 0, viewport.Width, viewport.Height);
		}

		depth_stencil_states[ZT_ON_ZW_ON]->inactive(immediate_context);
		blend_states[ALPHA]->inactive(immediate_context);
		rasterizer_states[SOLID]->inactive(immediate_context);
		sampler_states[POINT]->inactive(immediate_context);
		sampler_states[LINEAR]->inactive(immediate_context);
		sampler_states[ANISOTROPIC]->inactive(immediate_context);
	}

	terrains->DebugDrawGUI();
	rocks->DebugDrawGUI();
	trees->DebugDrawGUI();
	water_fall->DebugDrawGUI();
	//water->DebugDrawGUI();
	//structures->DebugDrawGUI();
	//vegetation_small->DebugDrawGUI();
	//sky->DrawDebugGUI();
	eye_space_camera->DrawDebugGUI();
	//light_space_camera->DrawDebugGUI();
	//ground->DebugDrawGUI();
	//player->DebugDrawGUI();
	//post_effects->DrawDebugGUI();
	//bloom_effect->DrawDebugGUI();
	white->DebugDrawGUI();
	shadowmap_df->DebugDrawGUI();
	screen->DebugDrawGUI();
	//pbr_ship_1->DebugDrawGUI();
	//pbr_ship->DebugDrawGUI();
    ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}