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
	// ���C�g�^�C�v����(0:���s�� 1:�_����)
	float lighttype = step(0.01, LightDir.w);
	// ��������(���s��:w=0.0 �_����:w>0)
	//float3 dir = LightDir.w < 0.001 ? LightDir.xyz : PLightDir;
	float3 dir = lerp(
	  LightDir.xyz, PLightDir, lighttype);
	//����
	float attenuation = LightDir.w < 0.001 ?
		1.0 :
		1.0 - (length(PLightDir) / LightDir.w);
	attenuation = saturate(attenuation); //0.0<--->1.0
	// ���C�g�v�Z
	float3 L = dir; //����
	float3 C = LightColor.rgb; //�J���[
	C *= attenuation; // ����
	float3 N = tex * 2.0 - 1.0; //�@��
	L = normalize(L);
	N = normalize(N);
	// ������ = -cos = -����
	float i = -dot(N, L);
	i = saturate(i); // 0.0�`1.0
	tex.rgb = C * i; // ���C�g�J���[����
	//0701 Deferred Shadow
	//�s�N�Z�����W�����C�g��Ԃɕϊ�
	float4 param = mul(shadowVP, P);
	//ShadowMap
	float2 uv = param.xy;
	uv.y = -uv.y;
	uv = uv * 0.5 + 0.5;
	//���U�V���h�E�}�b�v(Variance Shadowmap)
	float2 d = ShadowMap.Sample(ShadowSampler, uv).rg;
	float t = param.z / param.w;
	float d2 = d.y - (d.x * d.x); // �Ђ̂Q��
	float md = t - d.x;
	float p = d2 / (d2 + (md * md));
	float shadow = p;
	if (d.x >= t) shadow = 1; //�͈͊O�͔�e
	shadow = saturate(shadow); //0---1

	tex.rgb *= shadow;
	return tex;
}
