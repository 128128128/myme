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

// MRT information
struct PS_OUT {
	float4 Color : SV_TARGET0;
	float4 Depth : SV_TARGET1;
	float4 Normal : SV_TARGET2;
	float4 Position : SV_TARGET3;
	float4 RM : SV_TARGET4;
	float4 Shadow_param : SV_TARGET5;
};

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
	N = N * 0.5 + 0.5;

	ret.Color = float4(color.xyz, alpha);
	ret.Normal =float4(N,1.0);
	ret.Position = pin.position;
	float dist = length(pin.position - camera_constants.position);
	ret.Depth = float4(dist, 0, 0, 1);
	ret.RM = float4(0,0,1,1);
	//if shadow_reciever=0 = no shadow
	ret.Shadow_param = float4(255.0f * shadow_reciever, 255.0f * shadow_reciever, 255.0f * shadow_reciever, 255.0f * shadow_reciever);

    return ret;
}
