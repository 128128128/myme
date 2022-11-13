#include "static_mesh.hlsli"
#include "scene_constants.hlsli"
#include "render_constants.hlsli"

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

	//get out dot surface normal and ground normal
	float t = dot(N, hemisphere_light.ground_normal);
	//in 0~1
	t = saturate(t);

	float3 hemiLight = lerp(hemisphere_light.ground_color, hemisphere_light.sky_color, t);

    float3 finalcolor = diffuse + specular;

	//amibient color(ŠgŽUŒõ)
	finalcolor.x += ambient_intensity;
	finalcolor.y += ambient_intensity;
	finalcolor.z += ambient_intensity;

	finalcolor += hemiLight;
	return float4(finalcolor, alpha) /** pin.color*/;


}