#include "static_mesh.hlsli"
#include "scene_constants.hlsli"

Texture2D bump_map : register(t4);
SamplerState linear_sampler_state : register(s1);

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.sv_position = mul(vin.position, mul(world, camera_constants.view_projection));

	vin.normal.w = 0;
	vout.normal = normalize(mul(vin.normal, world));

	vout.tangent.xyz = normalize(mul(vin.tangent.xyz, (float3x3)world));
	vout.tangent.w = vin.tangent.w;

	vout.texcoord = vin.texcoord;

	vout.color = vin.color;

	vout.position = mul(vin.position, world);
	/*float2 temp_cast_0 = float2(0.266, 0.266);
	float2 panner21 = (vin.texcoord.xy + something.iTime.x * temp_cast_0);
	float3 ase_vertexNormal = vin.normal.xyz;
	vout.position.xyz += (bump_map.SampleLevel(linear_sampler_state, float4(panner21, 0, 0.0),0.0) * float4((ase_vertexNormal * 0.0), 0.0)).rgb;*/

	

	return vout;
}

//float4 main( float4 pos : POSITION ) : SV_POSITION
//{
//	v2f o;
//	o.pos = UnityObjectToClipPos(v.vertex);
//
//
//	// scroll bump waves
//	float4 temp;
//	float4 wpos = mul(unity_ObjectToWorld, v.vertex);
//	temp.xyzw = wpos.xzxz * _WaveScale4 + _WaveOffset;
//	o.bumpuv0 = temp.xy;
//	o.bumpuv1 = temp.wz;
//
//	// object space view direction (will normalize per pixel)
//	o.viewDir.xzy = WorldSpaceViewDir(v.vertex);
//
//	#if defined(HAS_REFLECTION) || defined(HAS_REFRACTION)
//	o.ref = ComputeNonStereoScreenPos(o.pos);
//	#endif
//
//	UNITY_TRANSFER_FOG(o,o.pos);
//	return o;
//}

