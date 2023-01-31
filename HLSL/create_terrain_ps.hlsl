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
	float3 N = 0;
	if (primitive_data.bool_smooth > 0.0) {
		// smooth shading by partial derivative
		const float depth_in_pixels = primitive_data.vertical_scale;
		float h = noise_map.Sample(sampler_states[LINEAR_WRAP], pin.texcoord).x;
		N = normalize(float3(-ddx(h) * depth_in_pixels, 1, -ddy(h) * depth_in_pixels));
	}
	else
	 N = pin.normal.xyz;

	float diffuse_factor = max(0, dot(N, L) * 0.8 + 0.8);
	return float4(color.rgb * diffuse_factor, 1);
}