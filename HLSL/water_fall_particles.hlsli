struct VS_OUT
{
	uint vertex_id : VERTEXID;
};
struct GS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};
struct particle
{
	float3 position;
	float3 velocity;
	float3 direction;
};

cbuffer PARTICLE_CONSTANTS : register(b10)
{
	float4 current_eye_position;
	float4 previous_eye_position;
	row_major float4x4 camera_view;
	row_major float4x4 camera_projection;

	// Radius of outermost orbit 
	float outermost_radius;
	// Height of snowfall area
	float snowfall_area_height;

	float particle_size;
	uint particle_count;

	float elapsed_time;
	float time;
	float2 pad;
};
