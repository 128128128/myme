
//	“ü—Í’¸“_
struct VS_IN {
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float2 texcoord   : TEXCOORD;
};

struct PS_IN
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldpos : TEXCOORD2;
};


cbuffer cb_shadoow_map  : register(b10)
{
	column_major  float4x4 shadowVP; //light camera shadow view projection
}