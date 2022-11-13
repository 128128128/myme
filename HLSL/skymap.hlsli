struct VS_IN
{
	float4 position : POSITION;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float3 texcoord : TEXCOORD;
};

cbuffer CONSTANT_BUFFER : register(b0)
{
	column_major float4x4 world;
	float4 color;
};

