#include "create_terrain.hlsli"

HS_CONSTANT_DATA_OUTPUT patch_constant_function(InputPatch<VS_CONTROL_POINT_OUTPUT, CONTROL_POINT_COUNT> input_patch, uint patch_id : SV_PrimitiveID)
{
    HS_CONSTANT_DATA_OUTPUT output;

    const float tesselation_max_subdivision = primitive_data.eye_in_local_space.w;
    //#define ENABLE_LOD
#ifdef ENABLE_LOD
    const float tesselation_max_distance = 500;
    float3 midpoint = (input_patch[0].position.xyz + input_patch[1].position.xyz + input_patch[2].position.xyz) / 3.0;
    const float distance = length(primitive_data.eye_in_local_space.xyz - midpoint);
    const float subdivision_factor = saturate((tesselation_max_distance - distance) / tesselation_max_distance);
    const float subdivision = tesselation_max_subdivision * subdivision_factor + 1.0;
#else
    const float subdivision = tesselation_max_subdivision;//...Number of divisions : ï™äÑêî
#endif
    output.tess_factor[0] = subdivision;
    output.tess_factor[1] = subdivision;
    output.tess_factor[2] = subdivision;
    output.inside_tess_factor = subdivision;
    return output;
}

[domain("tri")]
[partitioning("pow2")] // integer, fractional_odd, fractional_even, pow2
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patch_constant_function")]
HS_CONTROL_POINT_OUTPUT main(
    InputPatch<VS_CONTROL_POINT_OUTPUT, CONTROL_POINT_COUNT> input_patch,
    uint output_control_point_id : SV_OutputControlPointID,
    uint primitive_id : SV_PrimitiveID)
{
    HS_CONTROL_POINT_OUTPUT output;
    output = input_patch[output_control_point_id];
    return output;
}
