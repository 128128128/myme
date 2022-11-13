#include "lim_light.hlsli"
#include "scene_constants.hlsli"
#include "render_constants.hlsli"
#include "shader_functions.hlsli"

Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 color = diffuse_map.Sample(anisotropic_sampler_state, pin.texcoord);
	float alpha = color.a;
	float3 N = normalize(pin.normal.xyz);
	float3 T = float3(1.0001, 0, 0);
	float3 B = normalize(cross(N, T));
	T = normalize(cross(B, N));

	float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	normal.w = 0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

	float3 L = normalize(-light_direction.direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L) * 0.5 + 0.5);

	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
	float3 specular = pow(max(0, dot(N, normalize(E + L))), 3);//specular coordination

	//get out lim power depend on light dir & surface normal(サーフェイスの法線と光の入射方向に依存するlimの強さ)
	float power1 = 1.0f - max(0.0f, dot(light_direction.direction, pin.normal));
	//get out lim power depend on camera dir & surface normal(サーフェイスの法線と視線方向に依存するlimの強さ)
	float power2 = 1.0f - max(0.0f, pin.normal_in_view.z * -1.0f);

	//final lim power
	float limpower = power1 * power2;
	
	//like exponential function(指数関数)
	limpower = pow(limpower, 1.3f);

	float3 limcolor = limpower * diffuse;
	limcolor.x += ambient_intensity;
	limcolor.y += ambient_intensity;
	limcolor.z += ambient_intensity;
	return float4(limcolor, alpha) /** pin.color*/;

	/*float3 limcolor = Calc_lim_light(N, E, L, light_direction.color.rgb);
	return (limcolor, alpha);*/
}