#include "scene_constants.hlsli"
#include "shadow_df.hlsli"

PS_IN main(float3 position : POSITION,
float3 normal   : NORMAL,
float2 texcoord      : TEXCOORD)
{
	PS_IN output = (PS_IN)0;
	output.position = float4 (position, 1);
	output.texcoord = texcoord;
	return output;
}