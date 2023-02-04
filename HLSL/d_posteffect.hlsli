
struct VS_OUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

struct FOG_BUFFER
{
	float3 color;
	float highlight_intensity;//ハイライト強度
	float3 highlight_color;//...ハイライトカラー
	float highlight_power;//ハイライト強さ
	float global_density;//密度
	float height_falloff;//フォグ高さ
	float start_depth;//開始深度
	float start_height;//開始高度
};

cbuffer posteffect_buffer : register(b9)
{
	float contrast;
	float saturation;
	float cb_vignette;
	float cb_bloom;
	float cb_outline;
	float cb_color_grading;
	float2 pad;
	float4 colorlv;
	FOG_BUFFER fog;
};

//Volumetric Fog
float3 apply_fog(float3 default_color, float3 pixel_position/*world space*/,float depth, float3 eye_position/*world space*/)
{
	float3 eye_to_pixel = pixel_position - eye_position;
	float eye_to_pixel_distance = length(eye_to_pixel);// pixel_position.z;

	//Find the fog starting distance to pixel distance
	eye_to_pixel_distance = max(0, eye_to_pixel_distance - fog.start_depth);

	//float fog_height_density_at_view = exp(-fog.height_falloff * eye_position.y);
	float fog_height_density_at_view = exp(-fog.height_falloff * -fog.start_height); //霧の高さ密度
	float fog_intensity = eye_to_pixel_distance * fog_height_density_at_view;//霧の強さ

	const float threshold = 0.01;
	if (eye_to_pixel.y > threshold)
	{
		float t = fog.height_falloff * eye_to_pixel.y;
		fog_intensity *= (1.0 - exp(-t)) / t;
	}

	//Combine both factors to get the final factor
	float fog_final_factor = exp(-fog.global_density * fog_intensity);

	//case of having fog & sun
	//Find the sum highlight and use it to blend the fog color
	const float distance_to_sun = 0; //TODO://太陽までの距離
	float sun_highlight_factor = saturate(dot(normalize(eye_to_pixel), normalize((float3(1.0f, 1.0f, 1.0f )* distance_to_sun) - eye_position)));
	sun_highlight_factor = pow(sun_highlight_factor, fog.highlight_power);
	float3 fog_final_color = lerp(fog.color.rgb, fog.highlight_color * fog.highlight_intensity, sun_highlight_factor * smoothstep(0, 100, eye_to_pixel_distance));
	//float3 fog_final_color = lerp(fog.color.rgb, fog.highlight_color * fog.highlight_intensity, 0);

	return lerp(fog_final_color, default_color, fog_final_factor);
	return lerp(fog.color.rgb, default_color, fog_final_factor);

	return lerp(fog.color.rgb, default_color, fog_final_factor);
}

//float3 apply_fog(float3 default_color, float3 pixel_position/*world space*/, float3 eye_position/*world space*/)
//{
//	float3 eye_to_pixel = pixel_position - eye_position;
//	float eye_to_pixel_distance =  length(eye_to_pixel);
//
//	//Find the fog starting distance to pixel distance
//	eye_to_pixel_distance = max(0, eye_to_pixel_distance - fog.start_depth);
//
//	//float fog_height_density_at_view = exp(-fog.height_falloff * eye_position.y);
//	float fog_height_density_at_view = exp(-fog.height_falloff * -fog.start_height); //霧の高さ密度
//	float fog_intensity = eye_to_pixel_distance * fog_height_density_at_view;//霧の強さ
//
//	const float threshold = 0.01;
//	if (eye_to_pixel.y > threshold)
//	{
//		float t = fog.height_falloff * eye_to_pixel.y;
//		fog_intensity *= (1.0 - exp(-t)) / t;
//	}
//
//	//Combine both factors to get the final factor
//	float fog_final_factor = exp(-fog.global_density * fog_intensity);
//
//	//case of having fog & sun
//	//Find the sum highlight and use it to blend the fog color
//	//const float distance_to_sun = 0; //TODO://太陽までの距離
//	//float sun_highlight_factor = saturate(dot(normalize(eye_to_pixel), normalize((-light_direction.direction.xyz * distance_to_sun) - eye_position)));
//	//sun_highlight_factor = pow(sun_highlight_factor, fog.highlight_power);
//	//float3 fog_final_color = lerp(fog.color.rgb, fog.highlight_color * fog.highlight_intensity, sun_highlight_factor * smoothstep(0, 100, eye_to_pixel_distance));
//	//float3 fog_final_color = lerp(fog.color.rgb, fog.highlight_color * fog.highlight_intensity, 0);
//
//	//return lerp(fog_final_color, default_color, fog_final_factor);
//	return lerp(fog.color.rgb, default_color, fog_final_factor);
//}
//

