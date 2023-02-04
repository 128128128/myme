#include "water_fall_particles.hlsli"
#include "scene_constants.hlsli"

[earlydepthstencil]
float4 main(GS_OUT pin) : SV_TARGET
{
	float3 color = float3(1,1,1);
	return float4(color/* * directional_light_color[0].rgb */ * light_direction.color.w, pin.color.a);
}
