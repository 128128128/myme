#include "scene_constants.hlsli"
#include "tree_branch.hlsli"

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	
	float3 ase_worldPos = mul(vin.position.xyz,(float3x3)world); //頂点のワールド座標
	float temp_output_60_0 = ((wind_speed*0.05) * (light_direction.triple_speed_iTime));
	float2 appendResult249 = (float2(wind_direction.x, wind_direction.z));
	float3 WindVectors99 = windVectors.SampleLevel(anisotropic_sampler_state, (wind_amplitude_multiplier * wind_amplitude * (ase_worldPos.xz * 0.01) + temp_output_60_0 * appendResult249), 0.0);
	WindVectors99 = WindVectors99 * 2.0 - 1.0;//-1~1
	float3 ase_objectScale = float3(length(world[0].xyz), length(world[1].xyz), length(world[2].xyz));
	float3 appendResult250 = (float3(wind_direction.x, 0.0, wind_direction.z));
	float3 _Vector2 = float3(1, 1, 1);
	float3 break282 = (((float3(0, 0, 0) + (sin(((temp_output_60_0 * ase_objectScale * appendResult250)) - (float3(-1, -1, -1) )) * (_Vector2 - float3(0, 0, 0)) / (_Vector2 - (float3(-1, -1, -1) )))) * vin.texcoord.xy.y));
	float3 appendResult283 = (float3(break282.x, 0.0, break282.z));
	float3 Wind17 = (((WindVectors99 *  vin.color.g) * max_wind_strength * wind_strength) + appendResult283);
	vin.position.xyz += Wind17*20.0;
	float3 ase_vertexNormal = vin.normal.xyz;
	float3 _Vector0 = float3(0, 1, 0);//up
	float3 lerpResult94 = lerp(ase_vertexNormal, _Vector0, flat_lighting);
	vin.normal.xyz = lerpResult94;
    
	float3 ase_worldlightDir = 0;

	//float3 ase_worldlightDir = normalize(UnityWorldSpaceLightDir(ase_worldPos));

	float3 normalizeResult236 = normalize(ase_worldlightDir);
	float3 ase_worldViewDir = normalize(camera_constants.position.xyz - ase_worldPos);
	float dotResult36 = dot(normalizeResult236, (1.0 - ase_worldViewDir));
//#if defined(LIGHTMAP_ON) && ( UNITY_VERSION < 560 || ( defined(LIGHTMAP_SHADOW_MIXING) && !defined(SHADOWS_SHADOWMASK) && defined(SHADOWS_SCREEN) ) )//aselc
//	float4 ase_lightColor = 0;
//#else //aselc
	float4 ase_lightColor = _LightColor0;
//#endif //aselc
	vout.vertexToFrag332 = (((max(dotResult36, 0.0) * vin.color.b) * _TransmissionColor.a) * (_TransmissionColor * ase_lightColor));

	vout.sv_position = mul(vin.position, mul(world, camera_constants.view_projection));

	vout.position = mul(vin.position, world);//world position

	vout.texcoord = vin.texcoord;

	vout.color = vin.color;

	vout.normal = vin.normal;
	return vout;
}