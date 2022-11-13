#include "snowfall_particles.hlsli"

StructuredBuffer<particle> particle_buffer : register(t9);

[maxvertexcount(4)]
void main(point VS_OUT input[1] : SV_POSITION, inout TriangleStream<GS_OUT> output)
{
	const float3 billboard[] =
	{
		float3(-1.0f, -1.0f, 0.0f), // Bottom-left corner
		float3(+1.0f, -1.0f, 0.0f),	// Bottom-right corner
		float3(-1.0f, +1.0f, 0.0f),	// Top-left corner
		float3(+1.0f, +1.0f, 0.0f),	// Top-right corner
	};
	const float2 texcoords[] =
	{
		float2(0.0f, 1.0f),			// Bottom-left 
		float2(1.0f, 1.0f),			// Bottom-right
		float2(0.0f, 0.0f),			// Top-left
		float2(1.0f, 0.0f),			// Top-right
	};

	particle p = particle_buffer[input[0].vertex_id];

	float4 view_space_pos = mul(float4(p.position, 1.0), camera_view);
	//float4 view_space_pos = mul(float4(0.0,2.0,0.0, 1.0), camera_constants.view);

	[unroll]
	for (uint vertex_index = 0; vertex_index < 4; ++vertex_index)
	{
		GS_OUT element;

		float3 corner_pos = billboard[vertex_index] * particle_size;
		//float3 corner_pos = billboard[vertex_index] * 0.01;

		element.position = mul(float4(view_space_pos.xyz + corner_pos, 1), camera_projection);
		//element.position = float4(billboard[vertex_index] * 0.1f, 1);
		//element.position.x += 0.11f * input[0].vertex_id;
		element.color = float4(1.0, 1.0, 1.0, 0.8);
		element.texcoord = texcoords[vertex_index];
		output.Append(element);
	}
	output.RestartStrip();
}

