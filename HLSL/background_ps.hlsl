#include "fullscreen_quad.hlsli"
#include "image_based_lighting.hlsli"

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 ndc;
	ndc.x = (pin.texcoord.x * 2.0) - 1.0;
	ndc.y = 1.0 - (pin.texcoord.y * 2.0);
	ndc.z = 1;
	ndc.w = 1;
	
	float4 R = mul(ndc, scene_data.inv_view_projection);
	R /= R.w;
	
	float roughness = scene_data.skybox_roughness;
#if 1
	return sample_skybox(R.xyz, roughness);
#else
	return sample_diffuse_iem(R.xyz);
#endif
}