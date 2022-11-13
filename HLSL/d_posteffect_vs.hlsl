#include "d_posteffect.hlsli"

VS_OUT main(float3 Position : POSITION,
float3 Normal   : NORMAL,
float2 Tex      : TEXCOORD)
{
	VS_OUT output = (VS_OUT)0;
	output.position = float4(Position, 1);
	output.texcoord = Tex;
	return output;
}