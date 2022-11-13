#include "differred_light.hlsli"

//texture

Texture2D ShadowMap : register(t14);
SamplerState ShadowSampler : register(s14);

Texture2D DiffuseTexture : register(t0);
Texture2D LightTexture : register(t1);
Texture2D PositionTexture : register(t2);
Texture2D RMTexture : register(t3);

Texture2D EnvTexture : register(t15);

SamplerState DecalSampler : register(s0);

//DeferredShadow
float4 main(PS_IN input) : SV_TARGET0
{
	float4 tex = DiffuseTexture.Sample(DecalSampler, input.texcoord);// *input.Color;

	//point light
	float4 LightDir= float4(-1.0, -1.0, 1.0,0.0);
	float4 LightColor = float4(1.0, 1.0, 1.0,1.0);
	float3 PLightPos =  LightDir.xyz;
	float4 P = PositionTexture.Sample(DecalSampler, input.texcoord);
	float3 PLightDir = P - PLightPos;
	// ライトタイプ判定(0:平行光 1:点光源)
	float lighttype = step(0.01, LightDir.w);
	// 方向決定(平行光:w=0.0 点光源:w>0)
	//float3 dir = LightDir.w < 0.001 ? LightDir.xyz : PLightDir;
	float3 dir = lerp(
	  LightDir.xyz, PLightDir, lighttype);
	//減衰
	float attenuation = LightDir.w < 0.001 ?
		1.0 :
		1.0 - (length(PLightDir) / LightDir.w);
	attenuation = saturate(attenuation); //0.0<--->1.0
	// ライト計算
	float3 L = dir; //方向
	float3 C = LightColor.rgb; //カラー
	C *= attenuation; // 減衰
	float3 N = tex * 2.0 - 1.0; //法線
	L = normalize(L);
	N = normalize(N);
	// 当たり具合 = -cos = -内積
	float i = -dot(N, L);
	i = saturate(i); // 0.0～1.0
	tex.rgb = C * i; // ライトカラー決定
	//0701 Deferred Shadow
	//ピクセル座標をライト空間に変換
	float4 param = mul(shadowVP, P);
	//ShadowMap
	float2 uv = param.xy;
	uv.y = -uv.y;
	uv = uv * 0.5 + 0.5;
	//分散シャドウマップ(Variance Shadowmap)
	float2 d = ShadowMap.Sample(ShadowSampler, uv).rg;
	float t = param.z / param.w;
	float d2 = d.y - (d.x * d.x); // σの２乗
	float md = t - d.x;
	float p = d2 / (d2 + (md * md));
	float shadow = p;
	if (d.x >= t) shadow = 1; //範囲外は非影
	shadow = saturate(shadow); //0---1

	tex.rgb *= shadow;
	return tex;
}
