#include "pbr_static_mesh.hlsli"
#include "scene_constants.hlsli"

// MRT
struct PS_OUT {
	float4 Color : SV_TARGET0;
	float4 Depth : SV_TARGET1;
	float4 Normal : SV_TARGET2;
	float4 Position : SV_TARGET3;
	float4 RM : SV_TARGET4;
};

Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);
//pbr metallic
Texture2D roughness_map : register(t6);
Texture2D metallic_map : register(t6);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

//Obtain normals from the normal map
//float3 get_normal_from_normalmap(float3 normal, float3 tangent, float3 biNormal, float2 uv)
//{
//	float3 bin_space_normal = normal_map.SampleLevel(linear_sampler_state, uv, 0.0f).xyz;
//	bin_space_normal = (bin_space_normal * 2.0f) - 1.0f;
//
//	float3 new_normal = tangent * bin_space_normal.x + biNormal * bin_space_normal.y + normal * bin_space_normal.z;
//
//	return new_normal;
//}
//
//
//PS_OUT main(VS_OUT pin)
//{
//	float4 color = diffuse_map.Sample(anisotropic_sampler_state, pin.texcoord);
//
//	PS_OUT ret;
//	ret.Color = color;
//
//	float3 N = normalize(pin.normal.xyz);
//	float3 T = float3(1.0001, 0, 0);
//	float3 B = normalize(cross(N, T));
//	T = normalize(cross(B, N));
//
//	//float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
//	//normal = (normal * 2.0) - 1.0;
//	//normal.w = 0;
//	//N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
//	////-1 <---> +1 ‚ð 0 <---> 1 ‚ÉR8G8B8A8‘Î‰ž
//	//N = N * 0.5 + 0.5;
//	//ret.Normal = float4(N, 1);
//
//	//Normal
//	ret.Normal.xyz = N;// get_normal_from_normalmap(pin.normal, pin.tangent, pin.binormal, pin.texcoord);
//	ret.Normal.w = 1.0;
//	// Position
//	ret.Position = pin.position;
//
//	float dist = length(
//		pin.position - camera_constants.position);
//	ret.Depth = float4(dist, 0, 0, 1);
//
//	//R+M
//	float roughness = smooth_param;// metallic_map.Sample(point_sampler_state, pin.texcoord).r;
//		float metallic = metallic_param;// metallic_map.Sample(point_sampler_state, pin.texcoord).a;
//	ret.RM = float4(metallic, 0,
//		0, roughness);
//
//	return ret;
//
//
//}

PS_OUT main(VS_OUT pin)
{
	PS_OUT ret;

	float4 color = diffuse_map.Sample(anisotropic_sampler_state, pin.texcoord);
	float alpha = 1.0;
	float3 N = normalize(pin.normal.xyz);
	float3 T = float3(1.0001, 0, 0);
	float3 B = normalize(cross(N, T));
	T = normalize(cross(B, N));

	float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	normal.w = 0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

	float3 L = normalize(-light_direction.direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L));

	float3 finalcolor = diffuse;

	ret.Color = float4(finalcolor, alpha);
	ret.Normal = float4(N, 0.0);
	ret.Position = pin.position;
	float dist = length(pin.position - camera_constants.position);
	ret.Depth = float4(dist, 0, 0, 1);
	float roughness = roughness_map.Sample(anisotropic_sampler_state, pin.texcoord).r;
	float metallic = roughness_map.Sample(anisotropic_sampler_state, pin.texcoord).a;
	ret.RM = float4(roughness, 0, metallic, 1);

	return ret;

}