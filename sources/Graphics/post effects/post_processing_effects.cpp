#include "post_processing_effects.h"
#include "../load/texture.h"
#include "../../imgui/imgui.h"

post_processing_effects::post_processing_effects(ID3D11Device* device) : Descartes::fullscreen_quad(device)
{
	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);
	post_processing_effects_ps = std::make_unique<pixel_shader>(device, "shader//post_processing_effects_ps.cso");

	D3D11_SAMPLER_DESC sampler_desc;
	sampler_desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; //D3D11_FILTER_MIN_MAG_MIP_POINT D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR
	sampler_desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; //D3D11_TEXTURE_ADDRESS_BORDER D3D11_TEXTURE_ADDRESS_CLAMP D3D11_TEXTURE_ADDRESS_MIRROR
	sampler_desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	sampler_desc.MipLODBias = 0;
	sampler_desc.MaxAnisotropy = 16;
	sampler_desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL; //D3D11_COMPARISON_LESS_EQUAL D3D11_COMPARISON_ALWAYS
	float color[4] = { 1, 1, 1, 1 };
	memcpy(sampler_desc.BorderColor, color, sizeof(color));
	sampler_desc.MinLOD = 0;
	sampler_desc.MaxLOD = D3D11_FLOAT32_MAX;
	comparison_sampler_state = std::make_unique<Descartes::sampler_state>(device, &sampler_desc);
}

void post_processing_effects::blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* color_map, ID3D11ShaderResourceView* depth_map, ID3D11ShaderResourceView* shadow_map, const DirectX::XMFLOAT4X4& light_view_projection)
{
	DirectX::XMStoreFloat4x4(&constant_buffer->data.light_view_projection, DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(&light_view_projection)));
	constant_buffer->active(immediate_context, 0, false, true);

	immediate_context->PSSetShaderResources(0, 1, &color_map);
	immediate_context->PSSetShaderResources(1, 1, &depth_map);
	immediate_context->PSSetShaderResources(2, 1, &shadow_map);

	post_processing_effects_ps->active(immediate_context);
	comparison_sampler_state->active(immediate_context, 3, false);
	Descartes::fullscreen_quad::blit(immediate_context);
	comparison_sampler_state->inactive(immediate_context);
	post_processing_effects_ps->inactive(immediate_context);

	ID3D11ShaderResourceView* null_shader_resource_views[3] = {};
	immediate_context->PSSetShaderResources(0, 3, null_shader_resource_views);
}

void post_processing_effects::DrawDebugGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("PostEffects", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("posteffect configuration"))
		{
			ImGui::ColorEdit3("fog_color", reinterpret_cast<float*>(&constant_buffer->data.fog.color));
			ImGui::ColorEdit3("fog_highlight_color", reinterpret_cast<float*>(&constant_buffer->data.fog.highlight_color));
			ImGui::InputFloat("highlight_intensity", &constant_buffer->data.fog.highlight_intensity, 0.1f, 1.0f);
			ImGui::InputFloat("fog_highlight_power", &constant_buffer->data.fog.highlight_power, 0.1f, 1.0f);
			ImGui::InputFloat("fog_global_density", &constant_buffer->data.fog.global_density, 0.0001f, 0.001f, "%.4f");
			ImGui::InputFloat("fog_height_falloff", &constant_buffer->data.fog.height_falloff, 0.0001f, 0.001f, "%.4f");
			ImGui::InputFloat("fog_start_depth", &constant_buffer->data.fog.start_depth, 0.1f, 1.0f);
			ImGui::InputFloat("fog_start_height", &constant_buffer->data.fog.start_height, 0.1f, 1.0f);

			ImGui::SliderFloat("max_blur", &constant_buffer->data.bokeh.max_blur, 0.0f, 1.0f, "bokeh = %.3f");
			ImGui::SliderFloat("aperture", &constant_buffer->data.bokeh.aperture, 0.0f, 0.5f, "bokeh = %.3f");
			ImGui::SliderFloat("focus", &constant_buffer->data.bokeh.focus, 0.0f, 1.0f, "bokeh = %.3f");
			ImGui::SliderFloat("aspect", &constant_buffer->data.bokeh.aspect, 0.0f, 1.0f, "bokeh = %.3f");

			ImGui::ColorEdit3("shadowmap_color", reinterpret_cast<float*>(&constant_buffer->data.shadowmap.color));
			ImGui::InputFloat("shadowmap_bias", &constant_buffer->data.shadowmap.bias, 0.00001f, 0.0001f, "%.5f");
		}
	}
	ImGui::End();
#endif

}