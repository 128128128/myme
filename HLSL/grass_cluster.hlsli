struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	column_major float4x4 transform : TRANSFORM;
};

struct VS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

