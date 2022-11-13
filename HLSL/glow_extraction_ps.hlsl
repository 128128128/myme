#include "bloom.hlsli"

Texture2D hdr_color_map : register(t0);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(float4 position : SV_POSITION, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float4 color_map_color = hdr_color_map.Sample(point_sampler_state, texcoord);
	float luminance = dot(color_map_color.rgb, float3(0.2126, 0.7152, 0.0722));
	return  luminance > glow_extraction_threshold ? float4(color_map_color.rgb, 1.0) : float4(0.0, 0.0, 0.0, 1.0);
}
