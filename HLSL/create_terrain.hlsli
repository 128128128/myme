struct VS_OUT
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};
typedef VS_OUT VS_CONTROL_POINT_OUTPUT;
typedef VS_OUT HS_CONTROL_POINT_OUTPUT;

struct HS_CONSTANT_DATA_OUTPUT
{
	float tess_factor[3] : SV_TessFactor;
	float inside_tess_factor : SV_InsideTessFactor;
};
#define CONTROL_POINT_COUNT 3

struct DS_OUT
{
	float4 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct GS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct primitive_constants
{
	row_major float4x4 world;
	float4 eye_in_local_space; // w:tesselation_max_subdivision
	float mix_rate;
	float texture1_num;
	float texture2_num;
	float vertical_scale;
	float bool_smooth;
	float3 pad;
};
cbuffer primitive_constants : register(b8)
{
	primitive_constants primitive_data;
}