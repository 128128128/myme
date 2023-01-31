#include "grass_cluster.hlsli"
#include "scene_constants.hlsli"

Texture2D diffuse_map : register(t0);
SamplerState default_sampler_state : register(s2); //D3D11_FILTER_ANISOTROPIC

struct PS_OUT {
	float4 Color : SV_TARGET0;
	float4 Depth : SV_TARGET1;
	float4 Normal : SV_TARGET2;
	float4 Position : SV_TARGET3;
	//float4 RM : SV_TARGET4;
};


PS_OUT main(VS_OUT pin) : SV_TARGET
{
	PS_OUT ret;
	float4 diffuse_map_color = diffuse_map.Sample(default_sampler_state, pin.texcoord);
	float alpha = diffuse_map_color.a;

	clip(alpha - 0.5);

	float4 ambient_map_colour = diffuse_map_color /** ambient_intensity*/;
	float4 specular_map_colour = diffuse_map_color;

	float3 N = normalize(float3(0, 1, 0));
	float3 L = normalize(-light_direction.direction.xyz);
#if 0
	float diffuse_factor = max(dot(L, N), 0);
#else
	//half lambert 
	//float diffuse_factor = dot(L, N) * 0.5 + 0.5;
	float diffuse_factor = max(0, dot(L, N)) * 0.5 + 0.5;
	diffuse_factor = diffuse_factor * diffuse_factor;
#endif

	float specular_factor = 0.0;

	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
#if 0
	float3 H = normalize(E + L);
	specular_factor = max(dot(H, N), 0);
#else
	float3 R = normalize(reflect(-L, N));
	specular_factor = max(dot(R, E), 0);
#endif
	specular_factor = pow(specular_factor, 5.0) * pow(1 - pin.texcoord.y, 2);

	float3 fragment_color = float3(0, 0, 0);

	//directional light
#if 1
	//Gooch shading model
	float3 cool_color = something.cool_factor.xyz + something.cool_factor.w * ambient_map_colour.rgb;
	float3 warm_color = something.warm_factor.xyz + something.warm_factor.w * diffuse_map_color.rgb;
	float3 highlight_color = specular_map_colour.rgb * float3(2, 2, 2);
	float3 lit_colour = lerp(warm_color, highlight_color, specular_factor);
	float3 unlit_color = 0.5 * cool_color;
	fragment_color = unlit_color + diffuse_factor * light_direction.color.xyz * light_direction.color.w * lit_colour;
#else
	float3 Ka = ambient_map_colour.rgb * (1 - diffuse_factor);
	float3 Kd = diffuse_map_colour.rgb * diffuse_factor;
	float3 Ks = specular_map_colour.rgb * specular_factor;
	fragment_color = Ka + (Kd + Ks) * directional_light.colour.xyz * directional_light.colour.w;
#endif

	//point light
	float3 point_light_direction = point_light.position.xyz - pin.position.xyz;
	float distance = length(point_light_direction);
	float reciprocal_range = point_light.color.w;
	float attenuation = 1 - saturate(distance * reciprocal_range);
	fragment_color.rgb += max(0, dot(normalize(point_light_direction), N) * attenuation * attenuation) * point_light.color.rgb;

	//return float4(fragment_colour * (1.5 - pin.texcoord.y), alpha);
	//return float4(fragment_color, 1.0);

	ret.Color = float4(fragment_color, 1.0);
	float depth = length(pin.position - camera_constants.position);
	ret.Depth = float4(depth, 0, 0, 1);
	ret.Normal = float4(fragment_color, 1.0);
	ret.Position = float4(pin.position);
	return ret;
}
