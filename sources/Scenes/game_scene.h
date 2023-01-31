#pragma once
#include <memory>

#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>

//-----------scene----------//
#include "scene.h"

///----------light------------//
#include "../Graphics/lights/light.h"

///---------------mesh-------------//
#include "../Graphics/mesh/dynamic_mesh.h"
#include "../Graphics/mesh/pbr_dynamic_mesh.h"
#include "../Graphics/mesh/static_mesh.h"

#include "../Graphics/others/render_state.h"
#include "../Graphics/others/framebuffer.h"
#include "../Graphics/deferred/gbuffer.h"
#include "../Graphics/shadow/shadow_map_df.h"
#include "../Graphics/others/rasterizer.h"
#include "../Graphics/post effects/post_processing_effects.h"
#include "../Graphics/post effects/bloom.h"
#include "../Graphics/others/shader.h"

//------Objects------//
#include "../Objects/character.h"
#include "../Objects/objects.h"
#include "../Objects/terrain.h"
#include "../Objects/rocks.h"
#include "../Objects/trees.h"
#include "../Objects/water_fall.h"
#include "../Objects/water.h"
//#include "../Objects/structures.h"
//#include "../Objects/skymap.h"

//------Camera------//
#include "../Camera/Camera.h"

//----Math----//
#include "../Math/collision.h"
#include "../Math/vector.h"
#include "../Graphics/shadow/cascade_shadow_map_matrix.h"

///---sprite----//
#include "../Graphics/sprite/sprite.h"
#include "../Graphics/sprite/playshader.h"

//----particle-----//
#include "../Particles/snow_particles.h"

//----IMGUI---//
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imgui_impl_dx11.h"
#include "../../imgui/imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];

namespace descartes
{
	namespace collision
	{
		class mesh;
	}
}


class game_scene : public Scene
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

	enum { POINT, LINEAR, ANISOTROPIC,POINT_CLAMP,LINEAR_CLAMP, ANISOTROPIC_CLAMP};
	std::unique_ptr<Descartes::sampler_state> sampler_states[6];

	enum { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF };
	std::unique_ptr<Descartes::depth_stencil_state> depth_stencil_states[4];

	//objects
	std::unique_ptr<Player> player;
	std::unique_ptr<pbr_Stage>pbr_ship;
	std::unique_ptr<pbr_Base>base;
	//std::unique_ptr<Ground>ground;//factory
	//std::unique_ptr<dynamic_mesh> test;
	std::unique_ptr<Terrain> terrains;
	std::unique_ptr<grass_Terrain> grass_terrains;
	std::unique_ptr<Rocks> rocks;
	std::unique_ptr<Trees> trees;
	std::unique_ptr<Ground>shadow_trees;
	std::unique_ptr<Water_Fall> water_fall;
	std::unique_ptr<Water> water;
	//std::unique_ptr<Structures> structures;
	//std::unique_ptr<VegetationSmall> vegetation_small;

	//particles
	std::unique_ptr<snow_particles> snow;

	//std::unique_ptr<Descartes::collision::mesh> collision_mesh;
	//scenery[s]
	//std::unique_ptr<skydome> sky;

	//framebuffers
    std::unique_ptr<Descartes::framebuffer> shadowmap;
	std::unique_ptr<Descartes::framebuffer> framebuffers[3];
	std::unique_ptr <gbuffer> screen;
	std::unique_ptr<shadow_map>shadowmap_df;

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
	std::unique_ptr<pixel_shader> pbr_loadtexture_ps;
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
	
	//particles
	//std::unique_ptr<snow_particles> snowfall;


	struct scene_constants
	{
		//directional
		struct something_constants
		{
			float iTime = 0.0f;//speed
			float triple_speed_iTime = 0.0f;//triple speed
			float elapse_time = 0;
			float shadow_map_num=0;
			Matrix mlvp[NUM_DEFERRED_LIGHTING_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];
			XMFLOAT4 warm_factor = XMFLOAT4(5 / 255.0f, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f); // w:intensity 
			XMFLOAT4 cool_factor = XMFLOAT4(0 / 255.0f, 0 / 255.0f, 1 / 255.0f, 5 / 255.0f); // w:intensity
			XMFLOAT4 wind_position;
			XMFLOAT4 wind_direction;
		};
		something_constants cb_somthing;

		struct player_shader_constants
		{
			XMFLOAT4 position;
			XMFLOAT4 direction;
		};
		player_shader_constants player_object;

	};
	std::unique_ptr<Descartes::constant_buffer<scene_constants>> scene_constants_buffer;

	struct light_constants
	{
		//point light
		struct point_light_constants
		{
			XMFLOAT4 color{ 15,1,1,1 };
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
	
		float ambient_intensity = 0.25f;
		float specular_intensity = 0.020f;
		float specular_power = 20;

		uint32_t tone_number = 2;
		
	};
	std::unique_ptr<Descartes::constant_buffer<renderer_constants>> renderer_constants_buffer;

		

		struct d_pointlight_param {
			XMFLOAT4 position;
			XMFLOAT4 color;
		};

		struct d_shadow_param {
			//d_PointLightParam point_light[8];
			//view-projection matrix from the light's point of view
			DirectX::XMFLOAT4X4 light_view_projection;
		};
		    std::unique_ptr<Descartes::constant_buffer<d_shadow_param>> d_shadow_constant_buffer;
	

	XMFLOAT4X4 world_transform{0,0,0,0,
	0,0,0,0,
	0,0,0,0,
	0,0,0,1};

private:
	//time from scene start
	float time = 0;
	float triple_speed_time = 0;

	bool post_blooming = false;
	bool enable_lens_flare = false;
	bool enable_post_effects = false;

	
    shadow::CascadeShadowMapMatrix m_cascadeShadowMapMatrix;    // カスケードシャドウマップの行列を扱うオブジェクト

};