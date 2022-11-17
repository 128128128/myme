#include "common.hlsli"
#include "fullscreen_quad.hlsli"

Texture2D textures[2] : register(t0);

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
column_major const static matrix<float, 3, 3> aces_input_mat =
{
	0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
column_major const static matrix<float, 3, 3> aces_output_mat =
{
	1.60475, -0.10208, -0.00327,
    -0.53108, 1.10813, -0.07276,
    -0.07367, -0.00605, 1.07602
};

// ACES tone map (faster approximation)
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 tonemap_aces_narkowicz(float3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0);
}

// ACES filmic tone map approximation
// see https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
float3 rrt_and_odt_fit(float3 color)
{
	float3 a = color * (color + 0.0245786) - 0.000090537;
	float3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
	return a / b;
}

// tone mapping 
float3 tonemap_aces_hill(float3 color)
{
	color = mul(color, aces_input_mat);

    // Apply RRT and ODT
	color = rrt_and_odt_fit(color);

	color = mul(color, aces_output_mat);

    // Clamp to [0, 1]
	color = clamp(color, 0.0, 1.0);

	return color;
}

#define TONEMAP_ACES_NARKOWICZ
//#define TONEMAP_ACES_HILL
//#define TONEMAP_ACES_HILL_EXPOSURE_BOOST
float3 tonemap(float3 color)
{
	color *= 1.0;

#ifdef TONEMAP_ACES_NARKOWICZ
	color = tonemap_aces_narkowicz(color);
#endif

#ifdef TONEMAP_ACES_HILL
    color = tonemap_aces_hill(color);
#endif

#ifdef TONEMAP_ACES_HILL_EXPOSURE_BOOST
    // boost exposure as discussed in https://github.com/mrdoob/three.js/pull/19621
    // this factor is based on the exposure correction of Krzysztof Narkowicz in his
    // implemetation of ACES tone mapping
    color /= 0.6;
    color = tonemap_aces_hill(color);
#endif

	return pow(color, inv_gamma);
}

// https://www.geeks3d.com/20110405/fxaa-fast-approximate-anti-aliasing-demo-glsl-opengl-test-radeon-geforce/
#define FXAA_REDUCE_MIN (1.0/128.0)
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_SPAN_MAX 8.0
#define FXAA_SUBPIX_SHIFT (1.0 / 4.0)
#define FXAA_SAMPLER LINEAR_CLAMP
float3 fxaa(Texture2D framebuffer, float4 pos_pos, float2 rcp_frame)
{
	float3 sampled_nw = framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.zw).xyz;
	float3 sampled_ne = framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.zw, int2(1, 0)).xyz;
	float3 sampled_sw = framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.zw, int2(0, 1)).xyz;
	float3 sampled_se = framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.zw, int2(1, 1)).xyz;
	float3 sampled_m = framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.xy).xyz;

	float3 luma = float3(0.299, 0.587, 0.114);
	float luma_nw = dot(sampled_nw, luma);
	float luma_ne = dot(sampled_ne, luma);
	float luma_sw = dot(sampled_sw, luma);
	float luma_se = dot(sampled_se, luma);
	float luma_m = dot(sampled_m, luma);

	float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
	float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));

	float2 dir;
	dir.x = -((luma_nw + luma_ne) - (luma_sw + luma_se));
	dir.y = ((luma_nw + luma_sw) - (luma_ne + luma_se));

	float dir_reduce = max((luma_nw + luma_ne + luma_sw + luma_se) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	float rcp_dir_min = 1.0 / (min(abs(dir.x), abs(dir.y)) + dir_reduce);
	dir = min(float2(FXAA_SPAN_MAX, FXAA_SPAN_MAX), max(float2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), dir * rcp_dir_min)) * rcp_frame;

	float3 rgb_a = (1.0 / 2.0) * (
		framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.xy + dir * (1.0 / 3.0 - 0.5)).xyz +
		framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.xy + dir * (2.0 / 3.0 - 0.5)).xyz);
	float3 rgb_b = rgb_a * (1.0 / 2.0) + (1.0 / 4.0) * (
		framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.xy + dir * (0.0 / 3.0 - 0.5)).xyz +
		framebuffer.Sample(sampler_states[FXAA_SAMPLER], pos_pos.xy + dir * (3.0 / 3.0 - 0.5)).xyz);
	float luma_b = dot(rgb_b, luma);

	return ((luma_b < luma_min) || (luma_b > luma_max)) ? rgb_a : rgb_b;
}

float4 main(VS_OUT pin) : SV_TARGET
{
	uint mip_level = 0, width, height, number_of_levels;
	textures[0].GetDimensions(mip_level, width, height, number_of_levels);
	
#if 0
	float4 sampled_color = textures[0].Sample(sampler_states[POINT_WRAP], pin.texcoord);
#else
	// FXAA(Fast approximate anti-aliasing)
	float2 rcp_frame = float2(1.0 / width, 1.0 / height);
	float4 pos_pos;
	pos_pos.xy = pin.texcoord.xy;
	pos_pos.zw = pin.texcoord.xy - (rcp_frame * (0.5 + FXAA_SUBPIX_SHIFT));

	float4 sampled_color = 1.0;
	sampled_color.rgb = fxaa(textures[0], pos_pos, rcp_frame);
#endif	
	
	// Apply tone mapping.
	sampled_color.rgb = tonemap(sampled_color.rgb);
  
	return sampled_color;
}
