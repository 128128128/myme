#include "bloom.hlsli"

// if you change value of 'number_of_downsampled', you must change 'number_of_downsampled' in bloom.h to this same value.
const static uint number_of_downsampled = 5;
Texture2D downsampled_textures[number_of_downsampled] : register(t0);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float3 fragment_color = 0;
	[unroll]
	for (uint index_of_downsampled = 0; index_of_downsampled < number_of_downsampled; ++index_of_downsampled)
	{
		fragment_color += downsampled_textures[index_of_downsampled].Sample(anisotropic_sampler_state, texcoord).xyz;
	}
	return float4(fragment_color * blur_convolution_intensity, 1);
}