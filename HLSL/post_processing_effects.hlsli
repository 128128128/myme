#include "scene_constants.hlsli"

struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 texcoord: TEXCOORD;
};

cbuffer POST_PROCESSING_EFFECTS_CONSTANTS : register(b0)
{
	//view-projection matrix from the light's point of view
	column_major float4x4 light_view_projection;

	//Volumetric Fog
	struct FOG
	{
		float3 color;
		float highlight_intensity;
		float3 highlight_color;
		float highlight_power;
		float global_density;
		float height_falloff;
		float start_depth;
		float start_height;
	};
	FOG fog;

	//Bokeh Effect
	//Max Blur: The maximum amount of blurring.Ranges from 0 to 1
	//Aperture: Bigger values create a shallower depth of field
	//Focus: Controls the focus of the effect
	//Aspect: Controls the blurring effect
	struct BOKEH
	{
		float max_blur;
		float aperture;
		float focus;
		float aspect;
	};
	BOKEH bokeh;

	struct SHADOWMAP
	{
		float3 color;
		float bias;
	};
	SHADOWMAP shadowmap;
};

//Volumetric Fog
float3 apply_fog(float3 default_color, float3 pixel_position/*world space*/, float3 eye_position/*world space*/)
{
	float3 eye_to_pixel = pixel_position - eye_position;
	float eye_to_pixel_distance = length(eye_to_pixel);

	//Find the fog starting distance to pixel distance
	eye_to_pixel_distance = max(0, eye_to_pixel_distance - fog.start_depth);

	//float fog_height_density_at_view = exp(-fog.height_falloff * eye_position.y);
	float fog_height_density_at_view = exp(-fog.height_falloff * -fog.start_height);
	float fog_intensity = eye_to_pixel_distance * fog_height_density_at_view;

	const float threshold = 0.01;
	if (eye_to_pixel.y > threshold)
	{
		float t = fog.height_falloff * eye_to_pixel.y;
		fog_intensity *= (1.0 - exp(-t)) / t;
	}

	//Combine both factors to get the final factor
	float fog_final_factor = exp(-fog.global_density * fog_intensity);

	//fog‚Æ‘¾—z‚ ‚è
	//Find the sum highlight and use it to blend the fog color
	//const float distance_to_sun = 0; //TODO://‘¾—z‚Ü‚Å‚Ì‹——£
	//float sun_highlight_factor = saturate(dot(normalize(eye_to_pixel), normalize((-light_direction.direction.xyz * distance_to_sun) - eye_position)));
	//sun_highlight_factor = pow(sun_highlight_factor, fog.highlight_power);
	//float3 fog_final_color = lerp(fog.color.rgb, fog.highlight_color * fog.highlight_intensity, sun_highlight_factor * smoothstep(0, 100, eye_to_pixel_distance));
	//float3 fog_final_color = lerp(fog.color.rgb, fog.highlight_color * fog.highlight_intensity, 0);

	//return lerp(fog_final_color, default_color, fog_final_factor);
	return lerp(fog.color.rgb, default_color, fog_final_factor);
}

//http://in2gpu.com/2014/07/22/create-fog-shader/
float3 atmospheric_effects(float3 default_color, float3 pixel_position/*world space*/, float3 eye_position/*world space*/)
{
	float3 eye_to_pixel = pixel_position - eye_position;
	float eye_to_pixel_distance = max(0, length(eye_to_pixel) - fog.start_depth);
	eye_to_pixel = normalize(eye_to_pixel);

	float fog_factor = 0;
	float b1 = fog.global_density * 0.001 * smoothstep(-2.2, 2.2, fog.height_falloff - eye_to_pixel.y);
	float b2 = fog.global_density * 0.001 * smoothstep(-2.2, 2.2, fog.height_falloff - eye_to_pixel.y);
	float in_scattering = exp(-eye_to_pixel_distance * b1);
	float extinction = exp(-eye_to_pixel_distance * b2);

	//Find the sum highlight and use it to blend the fog color
	const float sun_distance = 255;
	float sun_highlight_factor = saturate(dot(eye_to_pixel, normalize((-light_direction.direction.xyz * sun_distance) - eye_position)));
	sun_highlight_factor = pow(sun_highlight_factor, 16.0);
	float3 fog_color = lerp(fog.color.rgb, fog.highlight_color.rgb, sun_highlight_factor);

	return (1 - in_scattering) * fog_color + extinction * default_color;
}

float3 bokeh_effect(Texture2D color_map, float depth, SamplerState linear_sampler_state, float2 texcoord)
{
	//Bokeh Effect
	//Max Blur: The maximum amount of blurring.Ranges from 0 to 1
	//Aperture: Bigger values create a shallower depth of field
	//Focus: Controls the focus of the effect
	//Aspect: Controls the blurring effect
	//float max_blur = 0.1;
	//float aperture = 0.02;
	//float focus = 0.8;
	//float aspect = 0;
	float2 aspect_correct = float2(1.0, bokeh.aspect);

	//float4 depth1 = depth_map.Sample(linear_sampler_state, texcoord);
	float factor = depth - bokeh.focus;
	float2 dofblur = clamp(factor * bokeh.aperture, -bokeh.max_blur, bokeh.max_blur);
	float2 dofblur9 = dofblur * 0.9;
	float2 dofblur7 = dofblur * 0.7;
	float2 dofblur4 = dofblur * 0.4;

	float4 color;
	color = color_map.Sample(linear_sampler_state, texcoord);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.15, 0.37) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.37, 0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.40, 0.0) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.37, -0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.15, -0.37) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.15, 0.37) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.37, 0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.37, -0.15) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.15, -0.37) * aspect_correct) * dofblur);

	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.15, 0.37) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.37, 0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.37, -0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.15, -0.37) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.15, 0.37) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.37, 0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.37, -0.15) * aspect_correct) * dofblur9);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.15, -0.37) * aspect_correct) * dofblur9);

	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.40, 0.0) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur7);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur7);

	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.4, 0.0) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur4);
	color += color_map.Sample(linear_sampler_state, texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur4);

	return color.rgb / 41.0;
}

float3 bokeh_effect_ms(Texture2DMS<float4> color_map, uint sample_index, uint width, uint height, float depth, float2 texcoord)
{
	//Bokeh Effect
	//Max Blur: The maximum amount of blurring.Ranges from 0 to 1
	//Aperture: Bigger values create a shallower depth of field
	//Focus: Controls the focus of the effect
	//Aspect: Controls the blurring effect
	//float max_blur = 0.1;
	//float aperture = 0.02;
	//float focus = 0.8;
	//float aspect = 0;
	float2 aspect_correct = float2(1.0, bokeh.aspect);

	float factor = depth - bokeh.focus;
	float2 dofblur = clamp(factor * bokeh.aperture, -bokeh.max_blur, bokeh.max_blur);
	float2 dofblur9 = dofblur * 0.9;
	float2 dofblur7 = dofblur * 0.7;
	float2 dofblur4 = dofblur * 0.4;

	float4 color;
	color = color_map.Load(texcoord * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.15, 0.37) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.37, 0.15) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.40, 0.0) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.37, -0.15) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.15, -0.37) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.15, 0.37) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.37, 0.15) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.37, -0.15) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.15, -0.37) * aspect_correct) * dofblur) * float2(width, height), sample_index);

	color += color_map.Load((texcoord + (float2(0.15, 0.37) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.37, 0.15) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.37, -0.15) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.15, -0.37) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.15, 0.37) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.37, 0.15) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.37, -0.15) * aspect_correct) * dofblur9) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.15, -0.37) * aspect_correct) * dofblur9) * float2(width, height), sample_index);

	color += color_map.Load((texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.40, 0.0) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur7) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur7) * float2(width, height), sample_index);

	color += color_map.Load((texcoord + (float2(0.29, 0.29) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.4, 0.0) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.29, -0.29) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.0, -0.4) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.29, 0.29) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.4, 0.0) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(-0.29, -0.29) * aspect_correct) * dofblur4) * float2(width, height), sample_index);
	color += color_map.Load((texcoord + (float2(0.0, 0.4) * aspect_correct) * dofblur4) * float2(width, height), sample_index);

	return color.rgb / 41.0;
}
