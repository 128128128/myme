#include "geometric_substance.hlsli"

VS_OUT main(float4 position : POSITION, float4 normal : NORMAL, float4 tangent : TANGENT, float2 texcoord : TEXCOORD, uint4 joints : JOINTS, float4 weights : WEIGHTS)
{
	VS_OUT vout;

    float sigma = tangent.w;

#if 1
	if (primitive_data.skin > -1)
    {
		row_major float4x4 skin_matrix = weights.x * primitive_joint_data.matrices[joints.x] + weights.y * primitive_joint_data.matrices[joints.y] + weights.z * primitive_joint_data.matrices[joints.z] + weights.w * primitive_joint_data.matrices[joints.w];
        position = mul(float4(position.xyz, 1), skin_matrix);
        normal = normalize(mul(float4(normal.xyz, 0), skin_matrix));
        tangent = normalize(mul(float4(tangent.xyz, 0), skin_matrix));
    }
#endif

    position.w = 1;
	vout.position = mul(position, mul(primitive_data.world, scene_data.view_projection));
	vout.w_position = mul(position, primitive_data.world);

	normal.w = 0;
	vout.w_normal = normalize(mul(normal, primitive_data.world));

	tangent.w = 0;
	vout.w_tangent = normalize(mul(tangent, primitive_data.world));
	vout.w_tangent.w = sigma;

	vout.texcoord = texcoord;

	return vout;
}
