#include "pbr_dynamic_mesh.hlsli"
#include "render_constants.hlsli"
#include "scene_constants.hlsli"
#include "shader_functions.hlsli"

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
//texture maps
Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);
Texture2D emissive_map : register(t5);
//pbr metallic
Texture2D metallic_map : register(t6);

static const int NUM_DIRECTIONAL_LIGHT = 1; // ディレクションライトの本数

float3 GetNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
	float3 binSpaceNormal = normal_map.SampleLevel(sampler_states[LINEAR], uv, 0.0f).xyz;
	binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

	float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

	return newNormal;
}

//float4 main(VS_OUT pin) : SV_TARGET
//{
//	// 法線を計算
//     float3 N = GetNormal(pin.normal, pin.tangent, pin.binormal, pin.texcoord);
//
//	float4 albedo_color = diffuse_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//
//	float alpha = albedo_color.a;
//
//	// スペキュラカラーはアルベドカラーと同じにする
//	float3 spec_color = albedo_color;
//
//	// 金属度
//	float metallic = metallic_map.Sample(sampler_states[POINT], pin.texcoord).r;
//
//	// 滑らかさ
//	float smooth = metallic_map.Sample(sampler_states[POINT], pin.texcoord).a;
//
//	float3 L = normalize(-light_direction.direction.xyz);
//	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
//
//	//光源に向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
//	float3 H = normalize(L + E);
//	// 各種ベクトルがどれくらい似ているかを内積を利用して求める
//	float NdotH = saturate(dot(N, H));
//	float VdotH = saturate(dot(E, H));
//	float NdotL = saturate(dot(N, L));
//	float NdotV = saturate(dot(N, E));
//
//	float PI = 3.1415926f;
//
//
//	//-----< フレネル反射を考慮した拡散反射光 >-----//
//	//粗さは0.5で固定
//	float roughness = smooth;
//
//	float energyBias = lerp(0.0f, 0.5f, roughness);
//	float energyFactor = lerp(1.0f, 1.0 / 1.51, roughness);
//
//	//光源に向かうベクトルとハーフベクトルがどれだけ似ているかを内積で求める
//	float dotLH = saturate(dot(L, H));
//
//	//光源に向かうベクトルとハーフベクトル、
//	//光が平行に入射した時の拡散反射量
//	float Fd90 = energyBias + 2.0f * dotLH * dotLH * roughness;
//
//	//法線と光源に向かうベクトルを利用して拡散反射率を求める
//	float dotNL = saturate(dot(N, L));
//	float FL = (1 + (Fd90 - 1) * pow(1 - dotNL, 5));
//
//	//法線と視線に向かうベクトルを利用して拡散反射率を求める
//	float dotNE = saturate(dot(N, E));
//	float FE = (1 + (Fd90 - 1) * pow(1 - dotNE, 5));
//
//	//法線と光源への方向に依存する拡散反射率と、法線と視線ベクトルに依存する拡散反射率を乗算して
//	//最終的な拡散反射率を求めている
//	//PIで除算しているのは正規化を行うため
//	float diffuseFromFresnel = (FL * FE * energyFactor);
//	//-----------------------------------------//
//
//
//	//正規化Lambert拡散反射を求める
//	float3 lambertDiffuse = light_direction.color.xyz * NdotL / PI;
//
//	//最終的な拡散反射光を計算
//	float3 diffuse = albedo_color.xyz * diffuseFromFresnel * lambertDiffuse;
//
//
//
//	//-----< Cook-Torranceモデルを利用した鏡面反射率を計算 >-----//
//	float microfacet = 0.76f;
//
//	// 金属度を垂直入射の時のフレネル反射率として扱う
//	// 金属度が高いほどフレネル反射は大きくなる
//	float f0 = metallic;
//
//	// D項をベックマン分布を用いて計算する
//	float D = Beckmann(microfacet, NdotH);
//
//	// F項をSchlick近似を用いて計算する
//	float F = SpcFresnel(f0, VdotH);
//
//	// G項を求める
//	float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));
//
//	// m項を求める
//	float m = PI * NdotV * NdotH;
//
//	// ここまで求めた、値を利用して、Cook-Torranceモデルの鏡面反射を求める
//	float3 specular = max(F * D * G / m, 0.0);
//	//-------------------------------------------------------//
//
//
//	//金属度が高ければ、鏡面反射はスペキュラカラー、低ければ白
//	//スペキュラカラーの強さを鏡面反射率として使う
//	specular *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);
//
//
//	
//	float3 light = 0;
//
//	//滑らかさを使って、拡散反射光と鏡面反射光を合成
//   //滑らかさが高ければ、拡散反射は弱くなる
//	light += diffuse * (1.0f - smooth) + specular;
//
//	//環境光による底上げ
//	float3 ambientLight = float3(1,1,1);
//	light += ambientLight * albedo_color.xyz;
//
//	float4 color = alpha;
//	color.xyz = light * light_direction.color.w;
//
//#if 0
//	//逆ガンマ補正(Inverse gamma process)
//	const float GAMMA = 2.2;
//	color.rgb = pow(color.rgb, 1.0f / GAMMA);
//#endif
//
//
//	return color;
//
//	//float lig = 0;
//	////シンプルなディズニーベースの拡散反射を実装する
//	//// フレネル反射を考慮した拡散反射を計算
//	//float diffuseFromFresnel = CalcDiffuseFromFresnel(N, L, E);
//
//	//// 正規化Lambert拡散反射を求める
//	//float NdotL = saturate(dot(N, L));
//	//float3 lambertDiffuse = light_direction.color * NdotL / PI;
//
//	//// 最終的な拡散反射光を計算する
//	//float3 diffuse = albedo_color * diffuseFromFresnel * lambertDiffuse;
//
//	////Cook-Torranceモデルを利用した鏡面反射率を計算する
//	//// Cook-Torranceモデルの鏡面反射率を計算する
//	//float3 spec = CookTorranceSpecular(-L, E, N, smooth) * light_direction.color;
//
//	//// 金属度が高ければ、鏡面反射はスペキュラカラー、低ければ白
//	//// スペキュラカラーの強さを鏡面反射率として扱う
//	//spec *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);
//
// //   //滑らかさを使って、拡散反射光と鏡面反射光を合成する
// //   // 滑らかさが高ければ、拡散反射は弱くなる
//	//lig +=albedo_color;
// //   lig += diffuse * (1.0f - smooth) + spec;
//	//
//
//	//// 環境光による底上げ
//
//	//float4 finalColor = 1.0f;
//	//finalColor.xyz = lig*light_direction.direction.w;
//
//	//return finalColor;
//}

float4 main(VS_OUT pin) : SV_TARGET
{
	// 法線を計算
     //float3 N = GetNormal(pin.normal.xyz, pin.tangent.xyz, pin.binormal, pin.texcoord);
     float3 N = normalize(pin.normal.xyz);
     float3 T = float3(1.0001, 0, 0);
     float3 B = normalize(cross(N, T));
     T = normalize(cross(B, N));
     float4 normal = normal_map.Sample(sampler_states[LINEAR], pin.texcoord);
     normal = (normal * 2.0) - 1.0;
     normal.w = 0;
     N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

	float4 albedo_color = diffuse_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float alpha = albedo_color.w;
	// スペキュラカラーはアルベドカラーと同じにする
	float3 spec_color = albedo_color;

	// 金属度
	float metallic = metallic_map.Sample(sampler_states[POINT], pin.texcoord).r;
    metallic *= metallic_param;
	// 滑らかさ
	float smooth = metallic_map.Sample(sampler_states[POINT], pin.texcoord).a;
    smooth *= smooth_param;
	// 視線に向かって伸びるベクトルを計算する
	float3 toEye = normalize(camera_constants.position.xyz - pin.position.xyz);

    float3 lig = 0;
    for (int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        // シンプルなディズニーベースの拡散反射を実装する。
        // フレネル反射を考慮した拡散反射を計算
        float diffuseFromFresnel = CalcDiffuseFromFresnel(
            N, -light_direction.direction[ligNo], toEye);

        // 正規化Lambert拡散反射を求める
        float NdotL = saturate(dot(N, -light_direction.direction[ligNo]));
        float3 lambertDiffuse = light_direction.color[ligNo] * NdotL / PI;

        // 最終的な拡散反射光を計算する
        float3 L = normalize(-light_direction.direction.xyz);
        float3 diffuse = albedo_color * diffuseFromFresnel * lambertDiffuse;
        diffuse = albedo_color.rgb * max(0, dot(N, L) * 0.5 + 0.5);
        
        // Cook-Torranceモデルを利用した鏡面反射率を計算する
        // Cook-Torranceモデルの鏡面反射率を計算する
        float3 spec = CookTorranceSpecular(
            -light_direction.direction[ligNo], toEye, N, smooth)
            * light_direction.color[ligNo];

        // 金属度が高ければ、鏡面反射はスペキュラカラー、低ければ白
        // スペキュラカラーの強さを鏡面反射率として扱う
        spec *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);

        // 滑らかさを使って、拡散反射光と鏡面反射光を合成する
        // 滑らかさが高ければ、拡散反射は弱くなる
        lig += diffuse * (1.0f - smooth) + spec;
    }

    // 環境光による底上げ
    float ambientLight = float3(1, 1, 1);
    lig += ambientLight * albedo_color;

    //float4 emis = emissive_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 finalColor = 1.0f;
    finalColor.xyz = lig;

    //finalColor= finalColor + emis * float4(albedo_color.r, albedo_color.g, albedo_color.b, albedo_color.w)*1.6;
    return finalColor;
}