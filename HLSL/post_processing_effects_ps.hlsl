//#include "scene_constants.hlsli"
#include "post_processing_effects.hlsli"

Texture2D color_map : register(t0);
Texture2D depth_map : register(t1);
Texture2D shadow_map : register(t2);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);
SamplerComparisonState comparison_sampler_state : register(s3);

float4 main(VS_OUT pin, uint sample_index: SV_SAMPLEINDEX) : SV_TARGET
{
	uint color_map_mip_level = 0, color_map_width, color_map_height, color_map_number_of_levels;
	color_map.GetDimensions(color_map_mip_level, color_map_width, color_map_height, color_map_number_of_levels);
	float4 color_map_color = color_map.Sample(linear_sampler_state, pin.texcoord);

	float4 depth_map_color = depth_map.Sample(linear_sampler_state, pin.texcoord);
	float3 fragment_color = color_map_color.rgb;
	float alpha = color_map_color.a;

	fragment_color = bokeh_effect(color_map, depth_map_color.x, linear_sampler_state, pin.texcoord);

	//shadow mapping
	float4 position_in_ndc;
	//texture space to ndc//テクスチャをndcへ変換
	position_in_ndc.x = pin.texcoord.x * 2 - 1;
	position_in_ndc.y = pin.texcoord.y * -2 + 1;
	position_in_ndc.z = depth_map_color.x;
	position_in_ndc.w = 1;

	//ndc to world space
	float4 position_in_world_space = mul(position_in_ndc, camera_constants.inverse_view_projection);
	position_in_world_space /= position_in_world_space.w;

	//if (time_of_day > 5.5 && time_of_day < 18.5)
	{
		//world space to light view clip space, and to ndc
		float4 position_in_light_space = mul(position_in_world_space, light_view_projection);
		position_in_light_space /= position_in_light_space.w;
		if (position_in_light_space.z < 1 && position_in_light_space.z > 0) // TODO:?
		{
			//ndc to texture space
			position_in_light_space.x = position_in_light_space.x * 0.5 + 0.5;
			position_in_light_space.y = position_in_light_space.y * -0.5 + 0.5;

			float shadow_threshold = 0.0f;
			//float shadow_bias = 0.0008;
			//float3 shadow_color = float3(0.65, 0.65, 0.65);
			//PCF:percentage closer filtering
			//http://developer.download.nvidia.com/presentations/2008/GDC/GDC08_SoftShadowMapping.pdf
			shadow_threshold = shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias);
			//shadow_threshold = shadow_map.GatherCmpRed(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadow_bias);
		#if 1
			//threshold...閾値
			int offset = 2;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(-offset, -offset)) * 1;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(-offset, 0)) * 2;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(-offset, offset)) * 1;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(0, -offset)) * 2;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(0, 0)) * 4;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(0, offset)) * 2;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(offset, -offset)) * 1;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(offset, 0)) * 2;
			shadow_threshold += shadow_map.SampleCmpLevelZero(comparison_sampler_state, position_in_light_space.xy, position_in_light_space.z - shadowmap.bias, int2(offset, offset)) * 1;
			shadow_threshold /= 16;
		#endif

			shadow_threshold = lerp(shadowmap.color, float3(1.0f, 1.0f, 1.0f), shadow_threshold).x;
			fragment_color *= shadow_threshold;
		}
	}

	//fragment_color = apply_fog(fragment_color, position_in_world_space.xyz, camera_constants.position.xyz);
	fragment_color = apply_fog(fragment_color, position_in_world_space.xyz, player.position.xyz);
	//fragment_color = atmospheric_effects(fragment_color, position_in_world_space.xyz, player.position.xyz);

	return float4(fragment_color, alpha);
}

