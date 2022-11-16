#include "static_mesh.hlsli"
#include "scene_constants.hlsli"
#include "shadow_df.hlsli"

PS_IN main(float4 position : POSITION)
{
	/*PS_IN output = (PS_IN)0;
    output.position = mul(position, mul(world, camera_constants.view_projection));
    output.shadow_param = output.position;
    
    return output;*/

    PS_IN pin;
    pin.position = mul( something.mlvp[0][something.shadow_map_num],position);
    pin.shadow_param = pin.position;

    return pin;
}
