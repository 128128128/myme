#include "dynamic_mesh.hlsli"
#include "scene_constants.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);

//texture maps
Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 color = diffuse_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
	float alpha = color.a;
	float3 N = normalize(pin.normal.xyz);

	//float3 T = normalize(pin.tangent.xyz);
	//float sigma = pin.tangent.w;
	//T = normalize(T - N*dot(N, T));
	//float3 B = normalize(cross(N, T) * sigma);
	//float4 normal = texture_maps[3].Sample(sampler_states[LINEAR], pin.texcoord);
	//normal = (normal * 2.0) - 1.0;
	//N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));


	float3 L = normalize(-light_direction.direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L)*0.5+0.5);
	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
	float3 specular = pow(max(0, dot(N, normalize(E + L))), 128);

	
	return float4(diffuse + specular, alpha) /** pin.color*/;
}
