#include "differred_light.hlsli"

PS_IN main(VS_IN input)
{
	PS_IN output = (PS_IN)0;

	float4 P = float4(input.position, 1.0);
	output.position = P;
	output.texcoord = input.texcoord;
	return output;
}
