#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <memory>

#include <iostream>
#include <fstream>
#include <shlwapi.h>
#undef max
#undef min
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>

#include "../../Graphics/others/rasterizer.h"
#include "../../Graphics/others/framebuffer.h"
#include "../../Graphics/others/render_state.h"
#include "../../Graphics/others/shader.h"
#include "../../Graphics/others/constant_buffer.h"
#include "../../Functions/utility_graphics.h"

class post_processing_effects : Descartes::fullscreen_quad
{
public:
	struct shader_constants
	{
		//view-projection matrix from the light's point of view
		DirectX::XMFLOAT4X4 light_view_projection;

		//Distance based & Height based fog 
		struct fog_shader_constants
		{
			DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(176 / 255.0f, 223 / 255.0f, 255 / 255.0f);
			float highlight_intensity = 1;
			DirectX::XMFLOAT3 highlight_color = DirectX::XMFLOAT3(255 / 255.0f, 255 / 255.0f, 255 / 255.0f);
			float highlight_power = 18.0f;
			float global_density = 0.0f;
			float height_falloff = 0.050f;
			float start_depth = 0.0f;
			float start_height = -25.0f;

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(CEREAL_NVP(color.x), CEREAL_NVP(color.y), CEREAL_NVP(color.z), CEREAL_NVP(highlight_intensity),
					CEREAL_NVP(highlight_color.x), CEREAL_NVP(highlight_color.y), CEREAL_NVP(highlight_color.z), CEREAL_NVP(highlight_power),
					CEREAL_NVP(global_density), CEREAL_NVP(height_falloff), CEREAL_NVP(start_depth), CEREAL_NVP(start_height));
			}
		};
		fog_shader_constants fog;

		//Bokeh Effect
		//Max Blur: The maximum amount of blurring.Ranges from 0 to 1
		//Aperture: Bigger values create a shallower depth of field
		//Focus: Controls the focus of the effect
		//Aspect: Controls the blurring effect
		struct bokeh_shader_constants
		{
			float max_blur = 1.0f;
			float aperture = 0.0f;
			float focus = 1.0f;
			float aspect = 0.0f;

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(CEREAL_NVP(max_blur), CEREAL_NVP(aperture), CEREAL_NVP(focus), CEREAL_NVP(aspect));
			}
		};
		bokeh_shader_constants bokeh;

		struct shadowmap_shader_constants
		{
			DirectX::XMFLOAT3 color = DirectX::XMFLOAT3(0.65f, 0.65f, 0.65f);
			float bias = 0.0008f;

			template<class Archive>
			void serialize(Archive& archive)
			{
				archive(CEREAL_NVP(color.x), CEREAL_NVP(color.y), CEREAL_NVP(color.z), CEREAL_NVP(bias));
			}
		};
		shadowmap_shader_constants shadowmap;

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(CEREAL_NVP(fog), CEREAL_NVP(bokeh), CEREAL_NVP(shadowmap));
		}
	};
	std::unique_ptr<Descartes::constant_buffer<shader_constants>> constant_buffer;
	void i_archive(const char* filename)
	{
		if (PathFileExistsA(filename))
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

	post_processing_effects(ID3D11Device* device);
	virtual ~post_processing_effects() = default;
	post_processing_effects(post_processing_effects&) = delete;
	post_processing_effects& operator =(post_processing_effects&) = delete;
	void blit(ID3D11DeviceContext* immediate_context, ID3D11ShaderResourceView* color_map, ID3D11ShaderResourceView* depth_map, ID3D11ShaderResourceView* shadow_map, const DirectX::XMFLOAT4X4& light_view_projection/* view-projection matrix from the light's point of view*/);
	void DrawDebugGUI();
private:
	std::unique_ptr<pixel_shader> post_processing_effects_ps;
	std::unique_ptr<Descartes::sampler_state> comparison_sampler_state;
};