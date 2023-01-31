#pragma once
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include <cereal/archives/json.hpp>

#include "../others/shader.h"
#include "../others/render_state.h"
#include "../../Graphics/others/constant_buffer.h"


#include "../lights/light_manager.h"

class gbuffer
{
public:
	gbuffer(){};
	~gbuffer(){};
	void initialize(ID3D11Device* device, DirectX::XMFLOAT3 dir);
	void active(ID3D11DeviceContext* immediate_context);
	void inactive(ID3D11DeviceContext* immediate_context);
	void render(ID3D11DeviceContext* immediate_context);

	void DebugDrawGUI(bool flag=false);
	void ef_DebugDrawGUI(bool flag=false);
	void ssr_DebugDrawGUI(bool flag=false);
protected:
	struct ssr_param {
		float noise = 0.0f;
		DirectX::XMFLOAT3 pad;
	};
	std::unique_ptr<Descartes::constant_buffer<ssr_param>> ssr_buffer;

	struct d_posteffect_param {
		float contrast = 1.5f;
		float saturate = 0.5f;
		float vignette = 1.0f;
		float bloom = 0.3f;
		float outline = 0.0f;
		float color_grading = 0.0f;
		DirectX::XMFLOAT2 pad;
		DirectX::XMFLOAT4 color = { 1.0f, 1.1f, 1.2f, 1.0f };

		struct fog_constants_buffer
		{
			DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(176 / 255.0f, 223 / 255.0f, 255 / 255.0f);
			float highlight_intensity = 1;
			DirectX::XMFLOAT3 highlight_color = DirectX::XMFLOAT3(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
			float highlight_power = 18.0f;
			float global_density = 0.0110f;
			float height_falloff = 0.050f;
			float start_depth = 75.0f;
			float start_height = 0.0f;

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(CEREAL_NVP(color.x), CEREAL_NVP(color.y), CEREAL_NVP(color.z), CEREAL_NVP(highlight_intensity),
					CEREAL_NVP(highlight_color.x), CEREAL_NVP(highlight_color.y), CEREAL_NVP(highlight_color.z), CEREAL_NVP(highlight_power),
					CEREAL_NVP(global_density), CEREAL_NVP(height_falloff), CEREAL_NVP(start_depth), CEREAL_NVP(start_height));
			}
		};
		fog_constants_buffer fog_buffer;
	};
	std::unique_ptr<Descartes::constant_buffer<d_posteffect_param>> d_posteffect_buffer;

private:
	//depth buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> depth;
	Microsoft::WRL::ComPtr <ID3D11DepthStencilView> depth_stencil_view;
	// color buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_color;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_color;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_color;
	// depth buffer(length from camera)
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_depth;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView>rtv_depth;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_depth;
	// normal buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_normal;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_normal;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_normal;
	// 3D position buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_position;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_position;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_position;
	//R:Roughness G:Metallic B: A:
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_RM;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_RM;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_RM;

	//light buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_light;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_light;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_light;

	//enviroment buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_env;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_env;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_env;

	// Shadow Param buffer
	Microsoft::WRL::ComPtr <ID3D11Texture2D> tex_shadow_param;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> rtv_shadow_param;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> srv_shadow_param;

	//Shader	envlight;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> env_shader_resource_view;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> diffuse_shader_resource_view;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> specular_shader_resource_view;
	Microsoft::WRL::ComPtr <ID3D11ShaderResourceView> lut_ggx_shader_resource_view;

	//Deferred
	//Shader	dirlight;
	Microsoft::WRL::ComPtr < ID3D11Texture2D> tex_composite;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> rtv_composite;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> srv_composite;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> tex_ref_composite;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> rtv_ref_composite;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> srv_ref_composite;

	void lightning(ID3D11DeviceContext* immediate_context);

	enum { NONE, ALPHA, ADD, ALPHA_TO_COVERAGE };
	std::unique_ptr<Descartes::blend_state> blend_states[4];

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> default_render_target_view;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> default_depth_stencil_view;

	struct vertex {
		float x, y, z;		// position
		float nx, ny, nz;	// normal
		float tu, tv;		// UV
		float r, g, b, a;		// vertex color
		float tanx, tany, tanz;	// tangent
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertex_buffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> index_buffer;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> input_layout_1;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> differred_light_vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> env_light_ps;
	std::unique_ptr<pixel_shader> composite_ps;
	std::unique_ptr<pixel_shader> ssr_composite_ps;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> post_effect_vs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> post_effect_ps;

	//lights
	std::shared_ptr<directional_light> dir_light;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_state;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depth_stencil_state;

};
