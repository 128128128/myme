#include "bloom.hlsli"

Texture2D hdr_color_map : register(t0);
Texture2D gradient_map : register(t1);
Texture2D noise_map : register(t2);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

//https://john-chapman.github.io/2017/11/05/pseudo-lens-flare.html
float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{

	float2 mimic_texcoord = -texcoord + float2(1, 1);

	uint mip_level = 0, width, height, number_of_levels;
	hdr_color_map.GetDimensions(mip_level, width, height, number_of_levels);
	float2 texel_size = 1.0 / float2(width, height);

	//float lens_flare_ghost_dispersal = 0.3; //dispersion factor
	//ghost vector to image centre
	float2 ghost_vector = float2(0.5 - mimic_texcoord.x, 0.5 - mimic_texcoord.y) * lens_flare_ghost_dispersal;

	// sample ghosts;
	float3 fragment_color = 0;

	//int number_of_ghosts = 15; //number of ghost samples
	for (int i = 0; i < number_of_ghosts; ++i)
	{
		//float2 offset = frac(mimic_texcoord + ghost_vector * i);
		float2 offset = mimic_texcoord + ghost_vector * i;

		float3 color = hdr_color_map.Sample(anisotropic_sampler_state, offset).rgb;

		//float lens_flare_threshold = 1.5;
		color = max(color - lens_flare_threshold, 0);

		//float luminance = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
		//if (luminance > flare_threshold)
		{
#if 1
			float d = distance(offset, float2(0.5, 0.5));
			//float d = distance(texcoord, float2(0.5, 0.5));
			float weight = 1.0 - smoothstep(0.0, 0.75, d); // reduce contributions from samples at the screen edge
			fragment_color += color * weight;
#else
			fragment_color += color;
#endif
		}
	}
	float luminance = dot(fragment_color.xyz, float3(0.2126, 0.7152, 0.0722));
	fragment_color = luminance;

	fragment_color *= gradient_map.Sample(anisotropic_sampler_state, length(float2(0.5, 0.5) - mimic_texcoord) / length(float2(0.5, 0.5))).rgb;
	//fragment_color *= gradient_map.Sample(anisotropic_sampler_state, texcoord);

	fragment_color *= noise_map.Sample(anisotropic_sampler_state, texcoord).r;
	//fragment_color *= smoothstep(0.2, 0.65, noise_map.Sample(anisotropic_sampler_state, texcoord).r);

	return float4(fragment_color * lens_flare_intensity, 1.0);
}