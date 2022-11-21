#include "fullscreen_quad.hlsli"
#include "image_based_lighting.hlsli"
#include "scene_constants.hlsli"

float4 main(VS_OUT pin) : SV_TARGET
{
	//ndc position
	float4 ndc;
	ndc.x = (pin.texcoord.x * 2.0) - 1.0;
	ndc.y = 1.0 - (pin.texcoord.y * 2.0);
	ndc.z = 1;
	ndc.w = 1;
	
	float4 R = mul(ndc, camera_constants.inverse_view_projection);
	R /= R.w;

	//fixation
	float roughness = 0.0;// skybox_roughness;
#if 1
	return sample_skybox(R.xyz, roughness);
#else
	return sample_diffuse_iem(R.xyz);
#endif
}