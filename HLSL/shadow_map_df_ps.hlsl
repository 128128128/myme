#include "shadow_df.hlsli"

float4 main(PS_IN pin) : SV_TARGET
{
	//rendering length from light 
	/*float depth = pin.shadow_param.z /
					pin.shadow_param.w;
	return float4(depth,depth * depth,0,1);*/

    return float4(pin.position.z, pin.position.z, pin.position.z, 1.0f);

}