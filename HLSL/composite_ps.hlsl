#include "differred_light.hlsli"

//	テクスチャ
Texture2D DiffuseTexture : register(t0);
Texture2D LightTexture : register(t1);
Texture2D PositionTexture : register(t2);
Texture2D RMTexture : register(t3);

Texture2D EnvTexture : register(t15);

SamplerState DecalSampler : register(s0);

// カラーとライトの乗算
float4 main(PS_IN input)
	: SV_TARGET0
{
	float4 color = DiffuseTexture.Sample(DecalSampler, input.texcoord);
	float4 light = LightTexture.Sample(DecalSampler, input.texcoord);

	//Metallic
	float metallic = RMTexture.Sample(
		DecalSampler, input.texcoord).g;
	//metallic = 0.5;
	//color.rgb = float3(0, 0, 0);
	//light.rgb *= 3;
	float3 m1 = color.rgb * light.rgb;
	float3 m0 = color.rgb * 0.4;
	m0 += light.rgb * 0.25;
	color.rgb = lerp(m0, m1, metallic);
	return color;

}