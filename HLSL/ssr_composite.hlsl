#include "differred_light.hlsli"

Texture2D normal_texture : register(t0);
Texture2D light_texture : register(t1);
Texture2D position_texture : register(t2);
Texture2D RM_texture : register(t3);
Texture2D depth_texture : register(t4);
Texture2D color_texture : register(t5);


Texture2D env_texture : register(t15);

SamplerState sceneSampler : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 tex = color_texture.Sample(sceneSampler, pin.texcoord);
	float4 light = light_texture.Sample(sceneSampler, pin.texcoord);

	float4 N = normal_texture.Sample(sceneSampler, pin.texcoord);
	float dist = length(N.xyz * 2.0 - 1.0);
	float n = step(0.5, dist);

	float4 color = tex * lerp(1.0, light, n);
	//color = light;
	return color;
}