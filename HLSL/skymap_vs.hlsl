#include "skymap.hlsli"
#include "scene_constants.hlsli"

VS_OUT main(VS_IN vin)
{

	VS_OUT vout;

	vout.position = mul(vin.position, mul(world, camera_constants.view_projection));
	vout.texcoord = normalize(vin.position.xyz);

	//vout.texcoord = normalize(mul(vin.position, mul(world, camera_constants.view_projection)).xyz);

	return vout;
}
