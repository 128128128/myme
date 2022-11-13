struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
	float4 color:COLOR;
};

struct VS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
	float4 color:COLOR;
};

cbuffer OBJECT_CONSTANTS : register(b0)
{
	column_major float4x4 world;
	float4 color;
	float shadow_reciever;
	float3 pad;
};
