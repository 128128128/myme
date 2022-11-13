#include "scene_constants.hlsli"
#include "skymap.hlsli"

//TextureCube cubemap : register(t0);
Texture2D cubemap : register(t0);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(VS_OUT pin) : SV_TARGET
{
	//float luminance = max(1.0, dot(color.rgb, float3(0.2126, 0.7152, 0.0722))) * directional_light.color.w;
	//float3 light_color = normalize(color.rgb) * luminance;
	return cubemap.Sample(anisotropic_sampler_state,pin.texcoord);
	return float4(cubemap.Sample(anisotropic_sampler_state, pin.texcoord).rgb * color.rgb * light_direction.color.w, color.a);
}
