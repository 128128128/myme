#include "ibl_scene.h"
#include "../Math/collision.h"
#include "../Input/Mouse.h"

using namespace DirectX;

bool ibl_scene::initialize(ID3D11Device* device, CONST LONG screen_width, CONST LONG screen_height)
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
	D3D11_SAMPLER_DESC sampler_desc[]{
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressU*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressV*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressU*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressV*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_ANISOTROPIC/*Filter*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressU*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressV*/, D3D11_TEXTURE_ADDRESS_WRAP/*AddressW*/, 0.0f/*MipLODBias*/, 8/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressU*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressV*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressU*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressV*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_ANISOTROPIC/*Filter*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressU*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressV*/, D3D11_TEXTURE_ADDRESS_CLAMP/*AddressW*/, 0.0f/*MipLODBias*/, 8/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {0, 0, 0, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {1, 1, 1, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_MIN_MAG_MIP_LINEAR/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 0/*MaxAnisotropy*/, D3D11_COMPARISON_NEVER/*ComparisonFunc*/, {1, 1, 1, 1}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
		{D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT/*Filter*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressU*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressV*/, D3D11_TEXTURE_ADDRESS_BORDER/*AddressW*/, 0.0f/*MipLODBias*/, 16/*MaxAnisotropy*/, D3D11_COMPARISON_LESS_EQUAL/*ComparisonFunc*/, {0, 0, 0, 0}/*BorderColor[4]*/, 0.0f/*MinLOD*/, D3D11_FLOAT32_MAX/*MaxLOD*/},
	};
	for (size_t sampler_index = 0; sampler_index < _countof(sampler_desc); ++sampler_index)
	{
		HRESULT hr = device->CreateSamplerState(&sampler_desc[sampler_index], sampler_states[sampler_index].GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

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
				pbr_ship = std::make_unique<pbr_Stage>(device);
				//pbr_ship->position = DirectX::XMFLOAT4(-14.286f, 8.204f, 24.0f, 1.0f);
				pbr_ship_1 = std::make_unique<pbr_Stage>(device);
				//pbr_ship_1->position = DirectX::XMFLOAT4(-14.286f, 16.204f, 24.0f, 1.0f );
				pbr_ship_1->position = DirectX::XMFLOAT4(-2.0f, 0.0f, 0.0f, 1.0f );
				
			}
		

			if (!sky)
			{
				sky = std::make_unique<skydome>();
				//skydome->skydome = std::make_unique<static_mesh>(device, ".\\resources\\Model\\Skydome\\skydome.fbx");
				sky->Initialize(device);
			}

			if (!eye_space_camera)
			{
				eye_space_camera = std::make_unique<camera>(device);
				//eye_space_camera->reset({ 106.6f,37.2f,51.0f,1.0 }, { 1.0f,1.0f,1.0f,1.0f }, focal_length, 1.8f/*height_above_ground*/);
				eye_space_camera->reset({ -0.174f,0.149f,-1.978f,1.0 }, { 1.0f,1.0f,1.0f,1.0f }, focal_length, 1.8f/*height_above_ground*/);
			}
			if (!light_space_camera)
			{
				light_space_camera = std::make_unique<camera>(device);
			}

    framebuffers[0] = std::make_unique<Descartes::framebuffer>(device, screen_width, screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
	framebuffers[1] = std::make_unique<Descartes::framebuffer>(device, screen_width, screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_R24G8_TYPELESS);
	framebuffers[2] = std::make_unique<Descartes::framebuffer>(device, screen_width, screen_height, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN);
	shadowmap = std::make_unique<Descartes::framebuffer>(device, 1024 * 5, 1024 * 5, DXGI_FORMAT_R32G32_FLOAT/*not needed*/, DXGI_FORMAT_R32_TYPELESS);
	//msaa_resolver = std::make_unique<Descartes::msaa_resolve>(device);

	//effects
	{
		post_effects = std::make_unique<post_processing_effects>(device);
		post_effects->i_archive(".\\configurations\\post_processing_effects.ini");

		bloom_effect = std::make_unique<bloom>(device, screen_width, screen_height);
		bloom_effect->i_archive(".\\configurations\\bloom.ini");
	}
	rasterizer = std::make_unique<Descartes::rasterizer>(device);
	fullscreen_quad = std::make_unique<Descartes::fullscreen_quad>(device);

	white = std::make_unique<playshader>(device, "resources//zatsu.png", true, true);

	std::vector<std::vector<char*>> environment_filesets =
	{
		{
			".\\environments\\sunset_jhbcentral_4k\\sunset_jhbcentral_4k.dds",
			"./environments/sunset_jhbcentral_4k/diffuse_iem.dds",
			"./environments/sunset_jhbcentral_4k/specular_pmrem.dds",
			"./environments/sunset_jhbcentral_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/blaubeuren_night_4k/blaubeuren_night_4k.dds",
			"./environments/blaubeuren_night_4k/diffuse_iem.dds",
			"./environments/blaubeuren_night_4k/specular_pmrem.dds",
			"./environments/blaubeuren_night_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/st_peters_square_night_4k/st_peters_square_night_4k.dds",
			"./environments/st_peters_square_night_4k/diffuse_iem.dds",
			"./environments/st_peters_square_night_4k/specular_pmrem.dds",
			"./environments/st_peters_square_night_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/winter_evening_4k/winter_evening_4k.dds",
			"./environments/winter_evening_4k/diffuse_iem.dds",
			"./environments/winter_evening_4k/specular_pmrem.dds",
			"./environments/winter_evening_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/gym_entrance_4k/gym_entrance_4k.dds",
			"./environments/gym_entrance_4k/diffuse_iem.dds",
			"./environments/gym_entrance_4k/specular_pmrem.dds",
			"./environments/gym_entrance_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/studio_small_01_4k/studio_small_01_4k.dds",
			"./environments/studio_small_01_4k/diffuse_iem.dds",
			"./environments/studio_small_01_4k/specular_pmrem.dds",
			"./environments/studio_small_01_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/Playa_Sunrise/Playa_Sunrise.dds",
			"./environments/Playa_Sunrise/diffuse_iem.dds",
			"./environments/Playa_Sunrise/specular_pmrem.dds",
			"./environments/Playa_Sunrise/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/EtniesPark_Center/Etnies_Park_Center_3k.dds",
			"./environments/EtniesPark_Center/diffuse_iem.dds",
			"./environments/EtniesPark_Center/specular_pmrem.dds",
			"./environments/EtniesPark_Center/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/JapanNightAlley_1K_hdri_sphere/JapanNightAlley_1K_hdri_sphere.dds",
			"./environments/JapanNightAlley_1K_hdri_sphere/diffuse_iem.dds",
			"./environments/JapanNightAlley_1K_hdri_sphere/specular_pmrem.dds",
			"./environments/JapanNightAlley_1K_hdri_sphere/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/tears_of_steel_bridge_4k/tears_of_steel_bridge_4k.dds",
			"./environments/tears_of_steel_bridge_4k/diffuse_iem.dds",
			"./environments/tears_of_steel_bridge_4k/specular_pmrem.dds",
			"./environments/tears_of_steel_bridge_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
		{
			"./environments/country_club_4k/country_club_4k.dds",
			"./environments/country_club_4k/diffuse_iem.dds",
			"./environments/country_club_4k/specular_pmrem.dds",
			"./environments/country_club_4k/sheen_pmrem.dds",

			"./environments/lut_ggx.dds",
			"./environments/lut_sheen_E.dds",
			"./environments/lut_charlie.dds",
		},
	};
	for (std::vector<std::vector<char*>>::const_reference environment_fileset : environment_filesets)
	{
		environment_textures.emplace_back(std::make_unique<struct environment_textures>(device, environment_fileset));
	}

	//shader load
	{
		dynamic_mesh_vs = std::make_unique<vertex_shader<dynamic_mesh::vertex>>(device, "shader//dynamic_mesh_vs.cso");
		dynamic_mesh_ps = std::make_unique<pixel_shader>(device, "shader//dynamic_mesh_ps.cso");
		pbr_dynamic_mesh_vs = std::make_unique<vertex_shader<pbr_dynamic_mesh::vertex>>(device, "shader//pbr_dynamic_vs.cso");
		pbr_dynamic_mesh_ps = std::make_unique<pixel_shader>(device, "shader//ibl_ps.cso");
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

	{
		bit_block_transfer = std::make_unique<Descartes::fullscreen_quad>(device);
		create_ps_from_cso(device, "shader//tone_map_ps.cso", tone_map_ps.ReleaseAndGetAddressOf());
		create_ps_from_cso(device, "shader//background_ps.cso", background_ps.ReleaseAndGetAddressOf());
	}

	//constant buffers
	{
		scene_constants_buffer = std::make_unique<Descartes::constant_buffer<scene_constants>>(device);
		scene_constants_buffer->data.wind.direction = XMFLOAT4(-1, 0, 0.2f, 0);
		XMStoreFloat4(&scene_constants_buffer->data.wind.position, XMLoadFloat4(&scene_constants_buffer->data.wind.direction) * -300);
		light_constants_buffer = std::make_unique<Descartes::constant_buffer<light_constants>>(device);
		renderer_constants_buffer = std::make_unique<Descartes::constant_buffer<renderer_constants>>(device);
		//ibl buffer
		D3D11_BUFFER_DESC buffer_desc{};
		buffer_desc.ByteWidth = sizeof(scene_constants);
		buffer_desc.Usage = D3D11_USAGE_DEFAULT;
		buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		buffer_desc.CPUAccessFlags = 0;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		HRESULT hr = device->CreateBuffer(&buffer_desc, nullptr, ibl_constants_buffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), hr_trace(hr));
	}

	//particles
	{
		XMFLOAT3 ppos{ player->position.x,player->position.y,player->position.z };
		snowfall = std::make_unique<snow_particles>(device, ppos);
	}

	return true;
}

const char* ibl_scene::update(float& elapsed_time/*Elapsed seconds from last frame*/)
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	if (gamePad.GetButtonDown() & GamePad::BTN_A)
	{
		active_environment_textures = (++active_environment_textures) % environment_textures.size();
	}
	
	//player->update(collision_mesh.get(),elapsed_time);
	scene_constants_buffer->data.player_object.position = player->position;
	scene_constants_buffer->data.player_object.direction = player->direction;

	pbr_ship->update(elapsed_time);
	pbr_ship_1->update(elapsed_time);

	//ground->update(elapsed_time);
	//vegetation_small->update(elapsed_time);
	sky->update(elapsed_time);

	if (freelook)
	{
		eye_space_camera->freelook_update(elapsed_time, {1.0f,1.0f,1.0f,1.0f});
	}
	else
	{
		//eye_space_camera->update(player->position, elapsed_time);
		eye_space_camera->firstperson_update(elapsed_time,DirectX::XMFLOAT4(player->position.x,player->position.y+2.0f ,player->position.z+player->direction.z*0.1f,0));
	}

	if(enable_grasss){
	//wind update
		{
			XMVECTOR D = XMLoadFloat4(&scene_constants_buffer->data.wind.direction);
			XMVECTOR P = XMLoadFloat4(&scene_constants_buffer->data.wind.position);

			static float interval_time = 0;
			interval_time += elapsed_time;
			if (interval_time > 20)
			{
				XMStoreFloat4(&scene_constants_buffer->data.wind.position, XMLoadFloat4(&scene_constants_buffer->data.wind.direction) * -300);
				interval_time = 0;
			}
			else
			{
				float wind_speed = 20;
				XMStoreFloat4(&scene_constants_buffer->data.wind.position, P + D * wind_speed * elapsed_time);
			}
		}
	}
	


	if (gamePad.GetButton() & GamePad::BTN_B)
		return "title";

	return 0;
}

void ibl_scene::render(ID3D11DeviceContext* immediate_context, float elapsed_time/*Elapsed seconds from last frame*/)
{
	rasterizer_states[SOLID]->active(immediate_context);
	blend_states[ALPHA]->active(immediate_context);
	depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);

	for (size_t sampler_index = 0; sampler_index < _countof(sampler_states); ++sampler_index)
	{
		immediate_context->PSSetSamplers(static_cast<UINT>(sampler_index), 1, sampler_states[sampler_index].GetAddressOf());
	}

	//viewport setting
	D3D11_VIEWPORT viewport;
	UINT num_viewports{ 1 };
	immediate_context->RSGetViewports(&num_viewports, &viewport);

	//camera setting
	float aspect_ratio{ viewport.Width / viewport.Height };

	
	// generate shadowmap
	{
		shadowmap->clear(immediate_context);
		shadowmap->active(immediate_context);

		XMVECTOR L = XMVector3Normalize(XMLoadFloat4(&scene_constants_buffer->data.directional_light.direction));
		XMVECTOR P = XMLoadFloat4(&player->position);
		XMStoreFloat4(&light_space_camera->position, P  * L);
		XMStoreFloat4(&light_space_camera->focus, P);
		light_space_camera->perspective_projection = false; //orthographic projection
		light_space_camera->fovy_or_view_width = 50;
		light_space_camera->aspect_ratio = aspect_ratio;
		light_space_camera->near_z = 1.0f;
		light_space_camera->far_z = 100.0f;
		light_space_camera->active(immediate_context, 1, true, true,false,false);

		Descartes::view_frustum view_frustum(light_space_camera->view_projection(), light_space_camera->inverse_view_projection());

		time += elapsed_time;
		scene_constants_buffer->data.directional_light.iTime = time;
		scene_constants_buffer->data.directional_light.elapseTime = elapsed_time;
		//immediate_context->GSGetConstantBuffers(2, 1, scene_constants_buffer->buffer_object.GetAddressOf());
		scene_constants_buffer->active(immediate_context, 2, true, true);
		
		void_ps->active(immediate_context);

		//dynamic_mesh_shadowcast_vs->active(immediate_context);
		//player->render(immediate_context);
		//dynamic_mesh_shadowcast_vs->inactive(immediate_context);

		static_shadowcast_vs->active(immediate_context);
		pbr_ship->render(immediate_context);
		pbr_ship_1->render(immediate_context);
		static_shadowcast_vs->inactive(immediate_context);

		//structures->render(immediate_context,view_frustum);

		void_ps->inactive(immediate_context);

		shadowmap->inactive(immediate_context);
	}


	framebuffers[0]->clear(immediate_context);
	framebuffers[0]->active(immediate_context);

	eye_space_camera->aspect_ratio = aspect_ratio;
	eye_space_camera->active(immediate_context, 1, true, true,false,false);

	//view frustum
	Descartes::view_frustum view_frustum(eye_space_camera->view_projection(), eye_space_camera->inverse_view_projection());

    //scene_constants_buffer->active(immediate_context, 2, true, true);
    renderer_constants_buffer->active(immediate_context, 3, true, true);
	light_constants_buffer->active(immediate_context, 4, true, true);
	immediate_context->UpdateSubresource(ibl_constants_buffer.Get(), 0, 0, &ibl, 0, 0);
	immediate_context->VSSetConstantBuffers(5, 1, ibl_constants_buffer.GetAddressOf());
	immediate_context->PSSetConstantBuffers(5, 1, ibl_constants_buffer.GetAddressOf());

	const std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& shader_resource_views = environment_textures.at(active_environment_textures)->shader_resource_views;
	for (int environment_texture_index = 0; environment_texture_index < shader_resource_views.size(); ++environment_texture_index)
	{
		immediate_context->PSSetShaderResources(32 + environment_texture_index, 1, shader_resource_views.at(environment_texture_index).GetAddressOf());
	}

	//objects render
	{
		//sky
		//white->render(immediate_context, 0, 0, 1280, 720, 1, 1, 1, 1, 0);
		depth_stencil_states[ZT_OFF_ZW_OFF]->active(immediate_context);
		rasterizer_states[SOLID_CULL_NONE]->active(immediate_context);
		immediate_context->PSSetShader(background_ps.Get(), 0, 0);
		bit_block_transfer->blit(immediate_context, false, false,  false);

		depth_stencil_states[ZT_OFF_ZW_OFF]->inactive(immediate_context);
		rasterizer_states[SOLID_CULL_NONE]->inactive(immediate_context);

		rasterizer_states[SOLID]->active(immediate_context);
		depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);

		//dynamic_mesh_vs->active(immediate_context);
		//dynamic_mesh_ps->active(immediate_context);
		//player->render(immediate_context);
		//test->render(immediate_context);
		//dynamic_mesh_vs->inactive(immediate_context);
		//dynamic_mesh_ps->inactive(immediate_context);

		//pbr
		pbr_dynamic_mesh_ps->active(immediate_context);
		pbr_static_mesh_vs->active(immediate_context);
		pbr_ship->render(immediate_context);
		pbr_static_mesh_vs->inactive(immediate_context);
		pbr_dynamic_mesh_ps->inactive(immediate_context);

		//skydome
		//skydome_vs->active(immediate_context);
		//skydome_ps->active(immediate_context);
		//sky->Render(immediate_context);
		//skydome_ps->inactive(immediate_context);
		//skydome_vs->inactive(immediate_context);

		//staticmesh render
		static_mesh_vs->active(immediate_context);
		static_mesh_ps->active(immediate_context);
		pbr_ship_1->render(immediate_context);
		//if(enable_grasss)
		//Other_terrain->render(immediate_context);
		static_mesh_ps->inactive(immediate_context);
		static_mesh_vs->inactive(immediate_context);


		//terrain renderer
		//if (!enable_grasss)
	
		//vegetation_small->render(immediate_context);

		rasterizer_states[SOLID]->inactive(immediate_context);
		rasterizer_states[SOLID_CULL_NONE]->active(immediate_context);

		// Snowfall particles drawing process
		//depth_stencil_states[ZT_ON_ZW_ON]->inactive(immediate_context);
		//depth_stencil_states[ZT_ON_ZW_OFF]->active(immediate_context);
		//XMFLOAT3 pos{ player->position.x,player->position.y,player->position.z };
		//snowfall->integrate(immediate_context, pos, eye_space_camera->view(), eye_space_camera->projection());

		//snowfall->render(immediate_context);

		//depth_stencil_states[ZT_ON_ZW_OFF]->inactive(immediate_context);
		//depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);

		rasterizer_states[SOLID_CULL_NONE]->inactive(immediate_context);

		rasterizer_states[SOLID]->active(immediate_context);

		
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

		framebuffers[0]->inactive(immediate_context);

		if (enable_post_effects)
		{
			//resolve msaa to non-msaa framebuffer
			//msaa_resolver->resolve(immediate_context, framebuffers[0].get(), framebuffers[1].get());
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

				//tone map
				framebuffers[2]->active(immediate_context);
				depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);
				rasterizer_states[SOLID_CULL_NONE]->active(immediate_context);
				blend_states[NONE]->active(immediate_context);

				immediate_context->PSSetShader(tone_map_ps.Get(), 0, 0);
				rasterizer->blit(immediate_context, framebuffers[2]->render_target_shader_resource_view.Get(), 0, 0, viewport.Width, viewport.Height,0,0,viewport.Width,viewport.Height,0,1,1,1,1,true,false,false,false,false);
				depth_stencil_states[ZT_ON_ZW_ON]->inactive(immediate_context);
				rasterizer_states[SOLID_CULL_NONE]->inactive(immediate_context);
				blend_states[NONE]->inactive(immediate_context);

				rasterizer_states[SOLID]->active(immediate_context);
				blend_states[ALPHA]->active(immediate_context);
				depth_stencil_states[ZT_ON_ZW_ON]->active(immediate_context);
				framebuffers[2]->inactive(immediate_context);
			}
			rasterizer->blit(immediate_context, framebuffers[2]->render_target_shader_resource_view.Get(), 0, 0, viewport.Width, viewport.Height);
		}
		else
		{
			rasterizer->blit(immediate_context, framebuffers[0]->render_target_shader_resource_view.Get(), 0, 0, viewport.Width, viewport.Height);
		}

		depth_stencil_states[ZT_ON_ZW_ON]->inactive(immediate_context);
		blend_states[ALPHA]->inactive(immediate_context);
		rasterizer_states[SOLID]->inactive(immediate_context);
		
	}

	

	ImGui::SliderFloat("pure_white", &ibl.pure_white, 0.0f, +100.0f);
	ImGui::SliderFloat("emissive_intensity", &ibl.emissive_intensity, 0.0f, +100.0f);
	ImGui::SliderFloat("emissive_factor", &ibl.emissive_factor, 0.0f, +1.0f);
	ImGui::SliderFloat("occlusion_factor", &ibl.occlusion_factor, 0.0f, +10.0f);
	ImGui::SliderFloat("occlusion_strength", &ibl.occlusion_strength, 0.0f, +10.0f);
	ImGui::SliderFloat("roughness_factor", &ibl.roughness_factor, 0.0f, +10.0f);
	ImGui::SliderFloat("metallic_factor", &ibl.metallic_factor, 0.0f, +10.0f);
	ImGui::Checkbox("image_based_lighting", reinterpret_cast<bool*>(&ibl.image_based_lighting));


	//posteffects
	ImGui::Checkbox("enable_post_effects", &enable_post_effects);
	ImGui::Checkbox("post_blooming", &post_blooming);
	ImGui::Checkbox("lens_flare", &enable_lens_flare);

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

	ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(1280, 100), ImGuiCond_FirstUseEver);

	//ImGui::Begin("imgui", nullptr, ImGuiWindowFlags_MenuBar);

	static bool test_im = false;

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("light"))
		{
			ImGui::MenuItem("light_direction", NULL, &test_im);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	//ImGui::End();
		//sky->DrawDebugGUI();
		//eye_space_camera->DrawDebugGUI();
		//player->DebugDrawGUI();
		//post_effects->DrawDebugGUI();
		//bloom_effect->DrawDebugGUI();
		//white->DebugDrawGUI();
		//pbr_ship_1->DebugDrawGUI();
		//pbr_ship->DebugDrawGUI();

	if(test_im)
	{
		//if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		//{
		//	//light direction
		//	ImGui::SliderFloat4("light_direction", &scene_constants_buffer->data.directional_light.direction.x, -1.0f, 1.0f);
		//	ImGui::SliderFloat4("light_color", &light_constants_buffer->data.point_light.color.x, 0, 256.0f);
		//}
		ImGui::Begin("TEst");
		ImGui::Text("aaaaaaaaaaaaaa");
		ImGui::End();
	}
    ImGui::Render();

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}