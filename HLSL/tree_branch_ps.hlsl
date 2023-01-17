#include "tree_branch.hlsli"
#include "scene_constants.hlsli"

Texture2D main_texture: register(t0);
Texture2D leaves_texture: register(t5);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

struct PS_OUT {
	float4 Color : SV_TARGET0;
	float4 Depth : SV_TARGET1;
	float4 Normal : SV_TARGET2;
	float4 Position : SV_TARGET3;
	float4 RM : SV_TARGET4;
};

PS_OUT main(VS_OUT pin) :SV_TARGET0
{
	PS_OUT ret;
	float4 tex2d_node19 = main_texture.Sample(anisotropic_sampler_state, pin.texcoord);
	
	float4 color = float4(1, 1, 1, 1);
	float4 temp_output_343_0 = color * tex2d_node19;
	float4 lerp_result_246 = GradientBrightness * temp_output_343_0;
	float4 transform_204 = mul(world, float4(0, 0, 0, 1));
	float4 lerp_result_20 = lerp(lerp_result_246, HueVariation, (HueVariation.a * frac(((transform_204.x + transform_204.y) + transform_204.z))));
	float4 color_56 = saturate(lerp_result_20);
	
	float3 ase_world_pos = pin.position.xyz;
	float temp_output_60_0 = ((wind_speed * 0.05) * something.triple_speed_iTime);
	float2 append_result_249 = (float2(wind_direction.x, wind_direction.z));
	float3 wind_vectors_99 = wind_vectors.SampleLevel(anisotropic_sampler_state, wind_amplitude_multiplier * wind_amplitude * (ase_world_pos.xz *1) + temp_output_60_0 * append_result_249, 0.0);
	wind_vectors_99 = wind_vectors_99 * 2.0 - 1.0;
	float debug = 0.0;
	
	float4 lerp_result_97 = lerp(color_56, float4(wind_vectors_99, 0.0), debug);
    float3 albedo = lerp_result_97.rgb;
	float4 sss45 = pin.vertexToFrag332;
	float3 emission = sss45.rgb;
	float3 smoothness = Smoothness;
	float lerp_result_53 = lerp(1.0, 0.0, (AmbientOcclusion * (1.0 - pin.color.r)));
	float ambient_occlusion_218 = lerp_result_53;
	float occlusion = ambient_occlusion_218;
	float alpha_31 = tex2d_node19.a;
	float lerp_result_101 = lerp(alpha_31, 1.0, debug);
	clip(lerp_result_101 - mask);
	float4 final_color = float4(albedo, lerp_result_101);

	pin.normal = pin.normal * 0.5 + 0.5;
	ret.Color = final_color;
	ret.Position = pin.position;
	float dist = length(pin.position - camera_constants.position);
	ret.Depth = float4(dist, 0, 0, 1);
	ret.Normal = pin.normal;
	ret.RM = float4(0.0,0.0,1.0,1.0);//r..metallic g...pure white b...roghness
	return ret;
}