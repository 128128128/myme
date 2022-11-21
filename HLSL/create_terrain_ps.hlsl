#include "create_terrain.hlsli"
#include "common.hlsli"

Texture2D color_map1 : register(t0);
Texture2D color_map2 : register(t1);
Texture2D noise_map : register(t2);

float4 main(GS_OUT pin) : SV_TARGET
{
	float4 color1 = color_map1.Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
	float4 color2 = color_map2.Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);
	float4 noise = noise_map.Sample(sampler_states[ANISOTROPIC_WRAP], pin.texcoord);

	//mix texture
	float4 color = lerp(color2, color1, noise*primitive_data.mix_rate);

	float3 L = -scene_data.light_direction.xyz;
	float3 N = pin.normal.xyz;

	float diffuse_factor = max(0, dot(N, L) * 0.8 + 0.8);
	return float4(color.rgb * diffuse_factor, 1);
}