#include "dynamic_mesh.hlsli"
#include "scene_constants.hlsli"

float4 main(float4 position : POSITION, float4 bone_weights : WEIGHTS, uint4  bone_indices : BONES) : SV_POSITION
{
	float3 blended_position = { 0, 0, 0 };
	for (int i = 0; i < 4; i++)
	{
		blended_position += (bone_weights[i] * mul(position, bone_transforms[bone_indices[i]])).xyz;
	}
	position = float4(blended_position, 1.0f);

	return mul(position, mul(world, camera_constants.view_projection));
}