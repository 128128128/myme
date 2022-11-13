#include "terrain.hlsli"
#include "scene_constants.hlsli"

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;
	vout.sv_position = mul(vin.position, mul(world, camera_constants.view_projection));

	vout.position = mul(vin.position, world);
	vin.normal.w = 0;
	vout.normal = normalize(mul(vin.normal, world));

	vout.tangent.xyz = normalize(mul(vin.tangent.xyz, (float3x3)world));
	vout.tangent.w = vin.tangent.w;

	vout.texcoord = vin.texcoord;

	vout.color = vin.color;

	vout.binormal = vin.binormal;

	return vout;
}