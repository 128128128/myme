#include "snowfall_particles.hlsli"
#include "scene_constants.hlsli"

[earlydepthstencil]
float4 main(GS_OUT pin) : SV_TARGET
{
	return float4(pin.color.rgb/* * directional_light_color[0].rgb */* light_direction.color.w, pin.color.a);
}
