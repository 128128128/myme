#pragma once

#include <memory>

#include <iostream>
#include <fstream>
#include <shlwapi.h>
#undef max
#undef min
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>

#include "../others/rasterizer.h"
#include "../others/constant_buffer.h"
#include "../others/framebuffer.h"
#include "../others/shader.h"
#include "../others/render_state.h"

class bloom : Descartes::fullscreen_quad
{
public:
	struct shader_constants
	{
		float glow_extraction_threshold = 1.000f;
		float blur_convolution_intensity = 0.500f;
		float lens_flare_threshold = 1.000f;
		float lens_flare_ghost_dispersal = 0.300f; //dispersion factor
		int number_of_ghosts = 6;
		float lens_flare_intensity = 0.35f;
		int options[2];

		template<class T>
		void serialize(T& archive)
		{
			archive(CEREAL_NVP(glow_extraction_threshold), CEREAL_NVP(blur_convolution_intensity), CEREAL_NVP(lens_flare_threshold), CEREAL_NVP(lens_flare_ghost_dispersal));
		}
	};
	std::unique_ptr<Descartes::constant_buffer<shader_constants>> constant_buffer;
	void i_archive(const char* filename)
	{
		if (PathFileExistsA(filename) && !PathIsDirectoryA(filename))
		{
			std::ifstream ifs;
			ifs.open(filename, std::ios::binary);
			cereal::JSONInputArchive i_archive(ifs);
			i_archive(cereal::make_nvp(filename, constant_buffer->data));
		}
	}
	void o_archive(const char* filename)
	{
		std::ofstream ofs;
		ofs.open(filename, std::ios::binary);
		cereal::JSONOutputArchive o_archive(ofs);
		o_archive(cereal::make_nvp(filename, constant_buffer->data));
	}

	bloom(ID3D11Device* device, uint32_t width, uint32_t height);
	~bloom() = default;
	bloom(bloom&) = delete;
	bloom& operator =(bloom&) = delete;

	void generate(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* hdr_texture, bool enable_lens_flare);
	void blit(ID3D11DeviceContext* immediate_context);
	void DrawDebugGUI();

	std::unique_ptr<Descartes::framebuffer> glow_extraction;
	// if you change value of 'number_of_downsampled', you must change 'number_of_downsampled' in gaussian_blur_convolution_ps.hlsl to this same value.
	const static size_t number_of_downsampled = 5;
	std::unique_ptr<Descartes::framebuffer> gaussian_blur[number_of_downsampled][2];
	std::unique_ptr<Descartes::framebuffer> lens_flare;

private:
	enum { LINEAR_BORDER, POINT, LINEAR, ANISOTROPIC };
	std::unique_ptr<Descartes::sampler_state> sampler_states[4];

	std::unique_ptr<Descartes::blend_state> blend_state;

	std::unique_ptr<pixel_shader> glow_extraction_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_horizontal_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_vertical_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_convolution_ps;
	std::unique_ptr<pixel_shader> gaussian_blur_downsampling_ps;

	std::unique_ptr<pixel_shader> lens_flare_ps;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> gradient_map;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> noise_map;


};