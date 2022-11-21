#include "differred_light.hlsli"

//texture

Texture2D shadow_map : register(t14);
SamplerState shadow_sampler : register(s14);

Texture2D diffuse_texture : register(t0);
Texture2D light_texture : register(t1);
Texture2D position_texture : register(t2);
Texture2D rm_texture : register(t3);

Texture2D env_texture : register(t15);

SamplerState decal_sampler : register(s0);

//DeferredShadow
float4 main(PS_IN input) : SV_TARGET0
{
	float4 tex = diffuse_texture.Sample(decal_sampler, input.texcoord);// *input.Color;

	//point light
	float4 light_dir= float4(-1.0, -1.0, 1.0,0.0);
	float4 light_color = float4(1.0, 1.0, 1.0,1.0);
	float3 p_light_pos = light_dir.xyz;
	float4 P = position_texture.Sample(decal_sampler, input.texcoord);
	float3 p_light_dir = P - p_light_pos;
	// ライトタイプ判定(0:平行光 1:点光源)
	float lighttype = step(0.01, light_dir.w);
	// direction decision(dir light:w=0.0 point light:w>0)
	//float3 dir = LightDir.w < 0.001 ? LightDir.xyz : PLightDir;
	float3 dir = lerp(
		light_dir.xyz, p_light_dir, lighttype);
	//attenuation...減衰
	float attenuation = light_dir.w < 0.001 ?
		1.0 :
		1.0 - (length(p_light_dir) / light_dir.w);
	attenuation = saturate(attenuation); //0.0<--->1.0
	//light calculation
	float3 L = dir; //direction
	float3 C = light_color.rgb; //color
	C *= attenuation; // attenuation
	float3 N = tex * 2.0 - 1.0; //normal
	L = normalize(L);
	N = normalize(N);//-cos
	float i = -dot(N, L);
	i = saturate(i); // 0.0～1.0
	tex.rgb = C * i; // Light color decision
	//0701 Deferred Shadow
	//Convert pixel coordinates to light space...ピクセル座標をライト空間に変換
	float4 param = mul(shadowVP, P);
	//ShadowMap
	float2 uv = param.xy;
	uv.y = -uv.y;
	uv = uv * 0.5 + 0.5;
	//分散シャドウマップ(Variance Shadowmap)
	float2 d = shadow_map.Sample(shadow_sampler, uv).rg;
	float t = param.z / param.w;
	float d2 = d.y - (d.x * d.x); // σの２乗
	float md = t - d.x;
	float p = d2 / (d2 + (md * md));
	float shadow = p;
	if (d.x >= t) shadow = 1; //Out of range is non-shadowed...範囲外は非影
	shadow = saturate(shadow); //0---1

	tex.rgb *= shadow;
	return tex;
}
