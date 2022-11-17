#pragma once

#include <memory>

#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>

#include "../Input/Input.h"

#include "scene.h"
#include "../Graphics/others/render_state.h"
#include "../Graphics/mesh/dynamic_mesh.h"
#include "../Graphics/mesh/pbr_dynamic_mesh.h"
#include "../Graphics/mesh/static_mesh.h"

#include "../Graphics/others/framebuffer.h"
#include "../Graphics/others/rasterizer.h"
#include "../Graphics/post effects/post_processing_effects.h"
#include "../Graphics/post effects/bloom.h"
#include "../Graphics/others/shader.h"

//------Objects------//
#include "../Objects/character.h"
#include "../Objects/objects.h"
#include "../Objects/terrain.h"
#include "../Objects/structures.h"
#include "../Objects/skymap.h"

//------Camera------//
#include "../Camera/Camera.h"

//----collision----//
#include "../Math/collision.h"

///---sprite----//
#include "../Graphics/sprite/sprite.h"
#include "../Graphics/sprite/playshader.h"

//----particle-----//
#include "../Particles/snow_particles.h"

//----IMGUI---//
#ifdef USE_IMGUI
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imgui_impl_dx11.h"
#include "../../imgui/imgui_impl_win32.h"


namespace descartes
{
    namespace collision
    {
        class mesh;
    }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];
#endif

struct environment_textures
{
	environment_textures(ID3D11Device* device, const std::vector<char*>& filenames)
	{
		for (std::vector<char*>::const_reference& filename : filenames)
		{
			load_texture_from_file(device, filename, shader_resource_views.emplace_back().GetAddressOf(), true, true);
		}
	}
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> shader_resource_views;
};


class ibl_scene : public Scene
{
public:
	bool initialize(ID3D11Device* device, CONST LONG screen_width, CONST LONG screen_height);
	const char* update(float& elapsed_time/*Elapsed seconds from last frame*/);
	void render(ID3D11DeviceContext* immediate_context, float elapsed_time/*Elapsed seconds from last frame*/);
	void finilize(){};


	enum { NONE, ALPHA, ADD, ALPHA_TO_COVERAGE };
	std::unique_ptr<Descartes::blend_state> blend_states[4];

	enum { WIREFRAME, SOLID, SOLID_CULL_NONE };
	std::unique_ptr<Descartes::rasterizer_state> rasterizer_states[3];

	enum class SAMPLER_STATE {
		POINT_WRAP, LINEAR_WRAP, ANISOTROPIC_WRAP, POINT_CLAMP, LINEAR_CLAMP, ANISOTROPIC_CLAMP,
		POINT_BORDER_OPAQUE_BLACK, LINEAR_BORDER_OPAQUE_BLACK, POINT_BORDER_OPAQUE_WHITE, LINEAR_BORDER_OPAQUE_WHITE,
		COMPARISON_DEPTH, COUNT
	};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[static_cast<size_t>(SAMPLER_STATE::COUNT)];

	enum { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF };
	std::unique_ptr<Descartes::depth_stencil_state> depth_stencil_states[4];

	//objects
	std::unique_ptr<Player> player;
	std::unique_ptr<pbr_Stage>pbr_ship;
	std::unique_ptr<pbr_Stage>pbr_ship_1;
	//std::unique_ptr<Ground>ground;//factory
	//std::unique_ptr<dynamic_mesh> test;
	//std::unique_ptr<VegetationSmall> vegetation_small;


	//scenery[s]
	std::unique_ptr<skydome> sky;

	//framebuffers
    std::unique_ptr<Descartes::framebuffer> shadowmap;
	std::unique_ptr<Descartes::framebuffer> framebuffers[3];

	//post processing
	//std::unique_ptr<Descartes::msaa_resolve> msaa_resolver;
	std::unique_ptr<bloom> bloom_effect;
	std::unique_ptr<post_processing_effects> post_effects;

	//rasterizers
	std::unique_ptr<Descartes::rasterizer> rasterizer;
	std::unique_ptr<Descartes::fullscreen_quad> fullscreen_quad;

	//shader
	std::unique_ptr<vertex_shader<dynamic_mesh::vertex>> dynamic_mesh_vs;
	std::unique_ptr<pixel_shader> dynamic_mesh_ps;
	std::unique_ptr<vertex_shader<pbr_dynamic_mesh::vertex>> pbr_dynamic_mesh_vs;
	std::unique_ptr<vertex_shader<pbr_static_mesh::vertex>> pbr_static_mesh_vs;
	std::unique_ptr<pixel_shader> pbr_dynamic_mesh_ps;
	std::unique_ptr<vertex_shader<static_mesh::vertex>>static_mesh_vs;
	std::unique_ptr<pixel_shader> static_mesh_ps;
	std::unique_ptr<pixel_shader> spot_light_mesh_ps;
	std::unique_ptr<pixel_shader> point_light_mesh_ps;
	std::unique_ptr<vertex_shader<static_mesh::vertex>>lim_light_vs;
	std::unique_ptr<pixel_shader> lim_light_ps;
	std::unique_ptr<pixel_shader> hemi_light_ps;
	std::unique_ptr<vertex_shader<static_mesh::vertex>>skydome_vs;
	std::unique_ptr<pixel_shader> skydome_ps;
	std::unique_ptr<playshader> white;

	//shadowcast shaders
	std::unique_ptr<vertex_shader<dynamic_mesh::vertex>> dynamic_mesh_shadowcast_vs;
	std::unique_ptr<vertex_shader<static_mesh::vertex>> static_shadowcast_vs;
	std::unique_ptr<pixel_shader> void_ps;

	//camera
	float focal_length = 1;
	std::unique_ptr<camera> eye_space_camera;
	std::unique_ptr<camera> light_space_camera;
	bool freelook = true;
	bool enable_grasss = true;
	//particles
	std::unique_ptr<snow_particles> snowfall;


	struct scene_constants
	{
		//directional
		struct directional_light_constants
		{
			XMFLOAT4 direction = XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f);
			XMFLOAT4 color{1.0f,1.0f,1.0f,1.0f};
			float iTime = 0;
			float elapseTime = 0;
			XMFLOAT2 pad;
		};
		directional_light_constants directional_light;

		struct player_shader_constants
		{
			XMFLOAT4 position;
			XMFLOAT4 direction;
		};
		player_shader_constants player_object;
		struct wind_shader_constants
		{
			XMFLOAT4 position;
			XMFLOAT4 direction;
		};
		wind_shader_constants wind;
	};
	std::unique_ptr<Descartes::constant_buffer<scene_constants>> scene_constants_buffer;

	struct light_constants
	{
		//point light
		struct point_light_constants
		{
			XMFLOAT4 color{ 1,1,1,1 };
			XMFLOAT3 position = { 0,-3.0f,0 };
			float range = 10.0f;//influence power
		};
		point_light_constants point_light;

		//spot light
		struct spot_light_constants
		{
			XMFLOAT3 position{ 0.0f,-2.0f,0.0f };//spot light position
			float pad;
			XMFLOAT3 color{ 10.0f,10.0f,10.0f };//spot color
			float range = 300.0f;//influence power
			XMFLOAT3 direction{ 0.0f,1.0f,0.0f };//spot direction
			float angle = XMConvertToRadians(25.0f);//angle
		};
		spot_light_constants spot_light;

		struct hemisphere_light_constants
		{
			XMFLOAT4 sky_color{0.15f,0.7f,0.95f,0.0f};
			XMFLOAT4 ground_color{0.7f,0.5f,0.3f,0.0f};
			XMFLOAT4 ground_normal{0.0f,1.0f,0.0f,0.0f};
		};
		hemisphere_light_constants hemisphere_light;

	};
	std::unique_ptr<Descartes::constant_buffer<light_constants>> light_constants_buffer;


	struct renderer_constants
	{
		XMFLOAT4 warm_factor = XMFLOAT4(5 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f); // w:intensity 
		XMFLOAT4 cool_factor = XMFLOAT4(0 / 255.0f, 0 / 255.0f, 1 / 255.0f, 5 / 255.0f); // w:intensity

		float ambient_intensity = 0.25f;
		float specular_intensity = 0.020f;
		float specular_power = 20;

		uint32_t tone_number = 2;
		
	};
	std::unique_ptr<Descartes::constant_buffer<renderer_constants>> renderer_constants_buffer;

	struct ibl_constants
	{
		int image_based_lighting = 1;
		float pure_white{ 3.0f };
		float emissive_intensity{ 1.0f };
		float roughness_factor =0.0f;
		float metallic_factor =1.0f;
		float emissive_factor =1.0f;
		float occlusion_factor =1.0f;
		float occlusion_strength =1.0f;
	};
	ibl_constants ibl;
	Microsoft::WRL::ComPtr<ID3D11Buffer> ibl_constants_buffer;

	XMFLOAT4X4 world_transform{0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,1};

private:
	//time from scene start
	float time = 0;

	bool post_blooming = false;
	bool enable_lens_flare = false;
	bool enable_post_effects = false;


	std::vector<std::unique_ptr<environment_textures>> environment_textures;
	size_t active_environment_textures = 0;

	std::unique_ptr<Descartes::fullscreen_quad> bit_block_transfer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> tone_map_ps;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> background_ps;
};