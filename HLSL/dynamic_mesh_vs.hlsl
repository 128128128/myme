
#include "dynamic_mesh.hlsli"
#include "scene_constants.hlsli"

VS_OUT main(VS_IN vin)
{
	float3 blended_position = { 0, 0, 0 };
	float3 blended_normal = { 0, 0, 0 };
	for (int i = 0; i < 4; i++)
	{
		blended_position += (vin.bone_weights[i] * mul(vin.position, bone_transforms[vin.bone_indices[i]])).xyz;
		blended_normal += (vin.bone_weights[i] * mul(vin.normal.xyz, (float3x3)bone_transforms[vin.bone_indices[i]]));
	}
	vin.position = float4(blended_position, 1.0f);
	vin.normal = float4(blended_normal, 0.0f);

	VS_OUT vout;
	vout.sv_position = mul(vin.position, mul(world, camera_constants.view_projection));

	vout.position = mul(vin.position, world);
	vin.normal.w = 0;
	vout.normal = normalize(mul(vin.normal, world));

	vout.tangent.xyz = normalize(mul(vin.tangent.xyz, (float3x3)world));
	vout.tangent.w = vin.tangent.w;

	vout.texcoord = vin.texcoord;

	return vout;
}

