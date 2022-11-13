#include "bloom.h"
#include "../load/texture.h"
#include "../../imgui/imgui.h"

bloom::bloom(ID3D11Device* device, uint32_t width, uint32_t height) : Descartes::fullscreen_quad(device)
{
	constant_buffer = std::make_unique<Descartes::constant_buffer<shader_constants>>(device);

	glow_extraction = std::make_unique<Descartes::framebuffer>(device, width, height,
		DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
		true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/);
	for (size_t index_of_downsampled = 0; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		gaussian_blur[index_of_downsampled][0] = std::make_unique<Descartes::framebuffer>(device, width >> index_of_downsampled, height >> index_of_downsampled,
			DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
			true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/);
		gaussian_blur[index_of_downsampled][1] = std::make_unique<Descartes::framebuffer>(device, width >> index_of_downsampled, height >> index_of_downsampled, 
			DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
			true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/);
	}
	glow_extraction_ps = std::make_unique<pixel_shader>(device, "shader//glow_extraction_ps.cso");
	gaussian_blur_horizontal_ps = std::make_unique<pixel_shader>(device, "shader//gaussian_blur_horizontal_ps.cso");
	gaussian_blur_vertical_ps = std::make_unique<pixel_shader>(device, "shader//gaussian_blur_vertical_ps.cso");
	gaussian_blur_convolution_ps = std::make_unique<pixel_shader>(device, "shader//gaussian_blur_convolution_ps.cso");
	gaussian_blur_downsampling_ps = std::make_unique<pixel_shader>(device, "shader//gaussian_blur_downsampling_ps.cso");

	lens_flare = std::make_unique<Descartes::framebuffer>(device, width >> 2, height >> 2,
		DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_UNKNOWN,
		true/*need_render_target_shader_resource_view*/, false/*need_depth_stencil_shader_resource_view*/);
	lens_flare_ps = std::make_unique<pixel_shader>(device, "shader//lens_flare_ps.cso");
	load_texture_from_file(device, ".\\resources\\sprite\\color_gradient.png", gradient_map.GetAddressOf(), true, true);
	load_texture_from_file(device, ".\\resources\\sprite\\noise\\noise.png", noise_map.GetAddressOf(), true, true);

	sampler_states[LINEAR_BORDER] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
	sampler_states[POINT] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
	sampler_states[LINEAR] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);
	sampler_states[ANISOTROPIC] = std::make_unique<Descartes::sampler_state>(device, D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS, 0.0f, 0.0f, 0.0f, 1.0f);

	blend_state = std::make_unique<Descartes::blend_state>(device, BLEND_MODE::ADD);
}

void bloom::generate(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture, bool enable_lens_flare)
{
	sampler_states[POINT]->active(immediate_context, 0, false);
	sampler_states[LINEAR]->active(immediate_context, 1, false);
	sampler_states[ANISOTROPIC]->active(immediate_context, 2, false);

	constant_buffer->active(immediate_context, 0, false, true);

	ID3D11ShaderResourceView* null_shader_resource_view = 0;
	//Extracting bright color
	glow_extraction->clear(immediate_context);
	glow_extraction->active(immediate_context);
	glow_extraction_ps->active(immediate_context);
	immediate_context->PSSetShaderResources(0, 1, &hdr_texture);
	Descartes::fullscreen_quad::blit(immediate_context);
	immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
	glow_extraction_ps->inactive(immediate_context);
	glow_extraction->inactive(immediate_context);

	//Lens flare
	//http://john-chapman-graphics.blogspot.com/2013/02/pseudo-lens-flare.html
	if (enable_lens_flare)
	{
		lens_flare->clear(immediate_context);
		lens_flare->active(immediate_context);
		lens_flare_ps->active(immediate_context);
		immediate_context->PSSetShaderResources(0, 1, glow_extraction->render_target_shader_resource_view.GetAddressOf());
		immediate_context->PSSetShaderResources(1, 1, gradient_map.GetAddressOf());
		immediate_context->PSSetShaderResources(2, 1, noise_map.GetAddressOf());
		Descartes::fullscreen_quad::blit(immediate_context);
		immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
		immediate_context->PSSetShaderResources(1, 1, &null_shader_resource_view);
		immediate_context->PSSetShaderResources(2, 1, &null_shader_resource_view);
		lens_flare_ps->inactive(immediate_context);
		lens_flare->inactive(immediate_context);

		blend_state->active(immediate_context);
		glow_extraction->active(immediate_context);
		immediate_context->PSSetShaderResources(0, 1, lens_flare->render_target_shader_resource_view.GetAddressOf());
		Descartes::fullscreen_quad::blit(immediate_context, true/*use_embedded_rasterizer_state*/, true/*use_embedded_depth_stencil_state*/, true/*use_embedded_pixel_shader*/);
		immediate_context->PSSetShaderResources(0, 1, &null_shader_resource_view);
		glow_extraction->inactive(immediate_context);
		blend_state->inactive(immediate_context);
	}

	sampler_states[POINT]->inactive(immediate_context);
	sampler_states[LINEAR]->inactive(immediate_context);
	sampler_states[ANISOTROPIC]->inactive(immediate_context);

	//Gaussian blur
	//Efficient Gaussian blur with linear sampling
	//http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
	sampler_states[LINEAR_BORDER]->active(immediate_context, 0, false);
	{
		// downsampling
		immediate_context->PSSetShaderResources(0, 1, glow_extraction->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[0][0]->clear(immediate_context);
		gaussian_blur[0][0]->active(immediate_context);
		gaussian_blur_downsampling_ps->active(immediate_context);
		Descartes::fullscreen_quad::blit(immediate_context);
		gaussian_blur_downsampling_ps->inactive(immediate_context);
		gaussian_blur[0][0]->inactive(immediate_context);
		// ping-pong gaussian blur
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[0][0]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[0][1]->clear(immediate_context);
		gaussian_blur[0][1]->active(immediate_context);
		gaussian_blur_horizontal_ps->active(immediate_context);
		Descartes::fullscreen_quad::blit(immediate_context);
		gaussian_blur_horizontal_ps->inactive(immediate_context);
		gaussian_blur[0][1]->inactive(immediate_context);
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[0][1]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[0][0]->clear(immediate_context);
		gaussian_blur[0][0]->active(immediate_context);
		gaussian_blur_vertical_ps->active(immediate_context);
		Descartes::fullscreen_quad::blit(immediate_context);
		gaussian_blur_vertical_ps->inactive(immediate_context);
		gaussian_blur[0][0]->inactive(immediate_context);
	}
	for (size_t index_of_downsampled = 1; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		// downsampling
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[index_of_downsampled - 1][0]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[index_of_downsampled][0]->clear(immediate_context);
		gaussian_blur[index_of_downsampled][0]->active(immediate_context);
		gaussian_blur_downsampling_ps->active(immediate_context);
		Descartes::fullscreen_quad::blit(immediate_context);
		gaussian_blur_downsampling_ps->inactive(immediate_context);
		gaussian_blur[index_of_downsampled][0]->inactive(immediate_context);
		// ping-pong gaussian blur
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[index_of_downsampled][0]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[index_of_downsampled][1]->clear(immediate_context);
		gaussian_blur[index_of_downsampled][1]->active(immediate_context);
		gaussian_blur_horizontal_ps->active(immediate_context);
		Descartes::fullscreen_quad::blit(immediate_context);
		gaussian_blur_horizontal_ps->inactive(immediate_context);
		gaussian_blur[index_of_downsampled][1]->inactive(immediate_context);
		immediate_context->PSSetShaderResources(0, 1, gaussian_blur[index_of_downsampled][1]->render_target_shader_resource_view.GetAddressOf());
		gaussian_blur[index_of_downsampled][0]->clear(immediate_context);
		gaussian_blur[index_of_downsampled][0]->active(immediate_context);
		gaussian_blur_vertical_ps->active(immediate_context);
		Descartes::fullscreen_quad::blit(immediate_context);
		gaussian_blur_vertical_ps->inactive(immediate_context);
		gaussian_blur[index_of_downsampled][0]->inactive(immediate_context);
	}
	sampler_states[LINEAR_BORDER]->inactive(immediate_context);
}

void bloom::blit(ID3D11DeviceContext* immediate_context)
{
	sampler_states[POINT]->active(immediate_context, 0, false);
	sampler_states[LINEAR]->active(immediate_context, 1, false);
	sampler_states[ANISOTROPIC]->active(immediate_context, 2, false);
	blend_state->active(immediate_context);
	constant_buffer->active(immediate_context, 0, false, true);

	std::vector< ID3D11ShaderResourceView*> shader_resource_views;
	for (size_t index_of_downsampled = 0; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		shader_resource_views.push_back(gaussian_blur[index_of_downsampled][0]->render_target_shader_resource_view.Get());
	}
	immediate_context->PSSetShaderResources(0, number_of_downsampled, &shader_resource_views.at(0));
	gaussian_blur_convolution_ps->active(immediate_context);
	Descartes::fullscreen_quad::blit(immediate_context);
	gaussian_blur_convolution_ps->inactive(immediate_context);
	ID3D11ShaderResourceView* null_shader_resource_views[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { 0 };
	immediate_context->PSSetShaderResources(0, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, null_shader_resource_views);

	constant_buffer->inactive(immediate_context);
	blend_state->inactive(immediate_context);
	sampler_states[POINT]->inactive(immediate_context);
	sampler_states[LINEAR]->inactive(immediate_context);
	sampler_states[ANISOTROPIC]->inactive(immediate_context);
}

void bloom::DrawDebugGUI()
{
#ifdef _DEBUG
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Bloom", nullptr, ImGuiWindowFlags_None))
	{
		ImGui::InputFloat("glow_extraction_threshold", &constant_buffer->data.glow_extraction_threshold, 0.01f, 0.1f);
		ImGui::InputFloat("blur_convolution_intensity", &constant_buffer->data.blur_convolution_intensity, 0.01f, 0.1f);
		ImGui::InputFloat("lens_flare_threshold", &constant_buffer->data.lens_flare_threshold, 0.01f, 0.1f);
		ImGui::InputFloat("lens_flare_ghost_dispersal", &constant_buffer->data.lens_flare_ghost_dispersal, 0.001f, 0.01f);
		ImGui::InputInt("number_of_ghosts", &constant_buffer->data.number_of_ghosts);
		ImGui::SliderFloat("lens_flare_intensity", &constant_buffer->data.lens_flare_intensity, 0.0f, 1.0f);
	}
	ImGui::End();
#endif

}