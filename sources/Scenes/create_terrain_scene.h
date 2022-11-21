#pragma once


#include <DirectXMath.h>
#include <memory>

#include <sstream>

#include "scene.h"
#include "../Graphics/others/render_state.h"

#include "../CreateTerrain/create_terrain.h"
#include "../Graphics/sprite/noise_map.h"

//----IMGUI---//
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_internal.h"
#include "../../imgui/imgui_impl_dx11.h"
#include "../../imgui/imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern ImWchar glyphRangesJapanese[];


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

class create_terrain_scene : public Scene
{
public:
	bool initialize(ID3D11Device* device, CONST LONG screen_width, CONST LONG screen_height);
	const char* update(float& elapsed_time/*Elapsed seconds from last frame*/);
	void render(ID3D11DeviceContext* immediate_context, float elapsed_time/*Elapsed seconds from last frame*/);
	void finilize() {};

	enum class BLEND_STATE { NONE, ALPHA, ADD, MULTIPLY, TRANSMISSION, COUNT };
	Microsoft::WRL::ComPtr<ID3D11BlendState> blend_states[static_cast<size_t>(BLEND_STATE::COUNT)];

	enum class RASTER_STATE { SOLID, SOLID_CW, WIREFRAME, CULL_NONE, WIREFRAME_CULL_NONE, COUNT };
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_states[static_cast<size_t>(RASTER_STATE::COUNT)];

	enum class SAMPLER_STATE {
		POINT_WRAP, LINEAR_WRAP, ANISOTROPIC_WRAP, POINT_CLAMP, LINEAR_CLAMP, ANISOTROPIC_CLAMP,
		POINT_BORDER_OPAQUE_BLACK, LINEAR_BORDER_OPAQUE_BLACK, POINT_BORDER_OPAQUE_WHITE, LINEAR_BORDER_OPAQUE_WHITE,
		COMPARISON_DEPTH, COUNT
	};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_states[static_cast<size_t>(SAMPLER_STATE::COUNT)];

	enum class DEPTH_STATE { ZT_ON_ZW_ON, ZT_ON_ZW_OFF, ZT_OFF_ZW_ON, ZT_OFF_ZW_OFF, COUNT };
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_states[static_cast<size_t>(DEPTH_STATE::COUNT)];

	struct scene_constants
	{
		DirectX::XMFLOAT4X4 view_projection;
		DirectX::XMFLOAT4X4 inv_view_projection;
		DirectX::XMFLOAT4 light_direction{ -1.0f, -1.0f, -1.0f, 0.0f };
		DirectX::XMFLOAT4 eye_position;
		DirectX::XMFLOAT4 focus_position;
	};
	scene_constants scene_data;

	Microsoft::WRL::ComPtr<ID3D11Buffer> constant_buffers[8];

	struct spherical_coordinate_system
	{
		float radius = 80.0f; /// Rho or Radius is the distance from the center of the sphere.
		float theta = DirectX::XMConvertToRadians(20);  /// Theta is the angle around the x axis (latitude angle counterclockwise), values range from 0 to PI.
		float phi = DirectX::XMConvertToRadians(0);    /// Phi is the angle around the y axis (longitude angle counterclockwise), values range from 0 to 2PI.

		//cartesian...íºåç¿ïWÇ÷	
		DirectX::XMFLOAT4 to_cartesian()
		{
			theta = min(DirectX::XMConvertToRadians(+89), max(DirectX::XMConvertToRadians(-89), theta));
			return { radius * std::cosf(theta) * std::sinf(phi), radius * std::sinf(theta), radius * std::cosf(theta) * std::cosf(phi), 1.0f };
		}
	};
	spherical_coordinate_system camera_manipulator;


protected:
	std::unique_ptr<terrain> terrain;
	std::unique_ptr<noise_map> noise;

	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_noise;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_noise;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_noise;

	//depth buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> depth;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView> depth_stencil_view;

	//default render target
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> default_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> default_depth_stencil_view;

};