
struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};
cbuffer CONSTATS:register(b5)
{
	float itime;
	float3 mouse;
};