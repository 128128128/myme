struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float2 texcoord : TEXCOORD;
};

struct VS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float2 texcoord : TEXCOORD;
	float3 normal_in_view:TEXCOORD2;//normal in camera
};

cbuffer OBJECT_CONSTANTS : register(b0)
{
	column_major float4x4 world;
	float4 color;
};