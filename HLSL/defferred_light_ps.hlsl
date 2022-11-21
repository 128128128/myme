#include "scene_constants.hlsli"
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
	float4 tex = normal_texture.Sample(sceneSampler, pin.texcoord);

	float3 L = light_direction.direction.xyz;

	float3 N = tex.xyz * 2.0 - 1.0;//-1~1
	
	float3 C = light_direction.color.rgb * 2;
	L = normalize(L);
	N = normalize(N);
	float d = -dot(N, L); //= -cos
	d = saturate(d); // 0Å`1
	tex.rgb =  C*d; 

	return tex;
}