#include "create_terrain.hlsli"
#include "common.hlsli"

[maxvertexcount(3)]
void main(triangle DS_OUT input[3], inout TriangleStream<GS_OUT> output)
{
	GS_OUT element;

	float4 positions[3];
	positions[0] = mul(input[0].position, primitive_data.world);
	positions[1] = mul(input[1].position, primitive_data.world);
	positions[2] = mul(input[2].position, primitive_data.world);

	float3 normal = normalize(cross(positions[2].xyz - positions[0].xyz, positions[1].xyz - positions[0].xyz));
	element.normal = float4(normal, 0);

	element.sv_position = mul(positions[0], scene_data.view_projection);
	element.position = positions[0];
	element.texcoord = input[0].texcoord;
	output.Append(element);

	element.sv_position = mul(positions[1], scene_data.view_projection);
	element.position = positions[1];
	element.texcoord = input[1].texcoord;
	output.Append(element);

	element.sv_position = mul(positions[2], scene_data.view_projection);
	element.position = positions[2];
	element.texcoord = input[2].texcoord;
	output.Append(element);

	output.RestartStrip();
}