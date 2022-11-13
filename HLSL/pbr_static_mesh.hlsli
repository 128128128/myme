
struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
};

struct VS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
};

cbuffer OBJECT_CONSTANTS : register(b0)
{
	column_major float4x4 world;
	float4 color;
};

cbuffer PBR_CONSTANTS:register(b3)
{
	float metallic_param;
	float smooth_param;
	float2 pad;
};