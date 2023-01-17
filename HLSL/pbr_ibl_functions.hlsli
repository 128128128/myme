#include "material_info.hlsli"

TextureCube diffuse_iem : register(t33);
TextureCube specular_pmrem : register(t34);

Texture2D lut_ggx : register(t35);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
#define POINT_CLAMP 2
#define LINEAR_CLAMP 2
#define ANISOTROPIC_CLAMP 2
SamplerState sampler_states[3] : register(s0);

static const float gamma = 2.2;
static const float PI = 3.141592653;

float4 sample_specular_pmrem(float3 v, float roughness)
{
	uint width, height, number_of_levels;
	specular_pmrem.GetDimensions(0, width, height, number_of_levels);

	float lod = roughness * float(number_of_levels - 1);

	return  specular_pmrem.SampleLevel(sampler_states[LINEAR_CLAMP], v, lod);
}

// specularWeight is introduced with KHR_materials_specular
//BRDFについては、lambertian拡散ローブとCook-Torrance microfacet model をスペキュラーローブに使用するという一般的なアプローチをとる
//Image Based Lighting with Multiple Scattering//多重散乱法を利用したIBL
float3 ibl_radiance_lambertian(float3 N, float3 V, float roughness, float3 diffuse_color, float3 f0, float specular_weight)
{
	float NoV = clamp(dot(N, V), 0.0, 1.0);

	float2 brdf_sample_point = clamp(float2(NoV, roughness), 0.0, 1.0);


	//brdf...双方向反射率分布関数と呼ばれ、ある特定の角度から光を入射した時の反射光の角度分布特性を表す
	//lut texture...color filter の用に利用する
	float2 f_ab = lut_ggx.Sample(sampler_states[LINEAR_CLAMP], brdf_sample_point).rg;
	//Projection of the surrounding environment reflected by the normal
	//法線で反射した周囲環境を投影
	float3 irradiance = diffuse_iem.Sample(sampler_states[LINEAR_CLAMP], N).rgb;
	//return float4(f_ab,0, 1);

	// see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
	// Roughness dependent fresnel, from Fdez-Aguera //Roughness 依存のフラネル
	float3 fr = max(1.0 - roughness, f0) - f0;
	float3 k_s = f0 + fr * pow(1.0 - NoV, 5.0);
	float3 fss_ess = specular_weight * k_s * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)

	// Multiple scattering, from Fdez-Aguera
	float ems = (1.0 - (f_ab.x + f_ab.y));
	float3 f_avg = specular_weight * (f0 + (1.0 - f0) / 21.0);
	float3 fms_ems = ems * fss_ess * f_avg / (1.0 - f_avg * ems);
	float3 k_d = diffuse_color * (1.0 - fss_ess + fms_ems); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

	return (fms_ems + k_d) * irradiance;
}


float3 ibl_radiance_ggx(float3 N, float3 V, float roughness, float3 f0, float specular_weight,float3 light_color)
{
	float NoV = clamp(dot(N, V), 0.0, 1.0);

	float2 brdf_sample_point = clamp(float2(NoV, roughness), 0.0, 1.0);
	float2 f_ab = lut_ggx.Sample(sampler_states[LINEAR_CLAMP], brdf_sample_point).rg;

	float3 R = normalize(reflect(-V, N));
	float3 specular_light = light_color;// sample_specular_pmrem(R, roughness).rgb;

	// see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
	// Roughness dependent fresnel, from Fdez-Aguera
	float3 fr = max(1.0 - roughness, f0) - f0;
	float3 k_s = f0 + fr * pow(1.0 - NoV, 5.0);
	float3 fss_ess = k_s * f_ab.x + f_ab.y;

	return specular_weight * specular_light * fss_ess;
}



// Fresnel
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// https://github.com/wdas/brdf/tree/master/src/brdfs
// https://google.github.io/filament/Filament.md.html
// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
float3 f_schlick(float3 f0, float3 f90, float VoH)
{
	return f0 + (f90 - f0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}

// Smith Joint GGX
// マイクロファセット同士が光の反射経路を遮蔽することにより、失われてしまう光の反射成分（減衰）を考慮する関数
// Note: Vis = G / (4 * NoL * NoV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
//NoL..dot normal vec* light vec
//NoV...dot normal vec*view vec
float v_ggx(float NoL, float NoV, float alpha_roughness)
{
	//convert to material roughness by squaring the perceptual roughness.
	//知覚的粗さを二乗して物質的粗さに変換する
	float alpha_roughness_sq = alpha_roughness * alpha_roughness;

	float ggxv = NoL * sqrt(NoV * NoV * (1.0 - alpha_roughness_sq) + alpha_roughness_sq);
	float ggxl = NoV * sqrt(NoL * NoL * (1.0 - alpha_roughness_sq) + alpha_roughness_sq);

	float ggx = ggxv + ggxl;
	return (ggx > 0.0) ? 0.5 / ggx : 0.0;
}
// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// //物体表面のミクロレベルの各微小平面の法線がどれくらい指定の方向を向いているか、という法線の分布を表す関数
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
//GGX(Trowbridge-Retiz)//real time rendering p296
float d_ggx(float NoH/*ライトベクトルと視線ベクトルを足して正規化したベクトルと法線の内積*/, float alpha_roughness)
{
	float alpha_roughness_sq = alpha_roughness * alpha_roughness;
	float f = (NoH * NoH) * (alpha_roughness_sq - 1.0) + 1.0;
	return alpha_roughness_sq / (PI * f * f);
}

//https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 brdf_lambertian(float3 f0, float3 f90, float3 diffuse_color, float specular_weight, float VoH)
{
	// see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
	return (1.0 - specular_weight * f_schlick(f0, f90, VoH)) * (diffuse_color / PI);
}

//  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
//VoH...dot half vec*view vec
//NoL..dot normal vec* light vec
//NoV...dot normal vec*view vec
//NoH...dot normal vec*half vec
float3 brdf_specular_ggx(float3 f0, float3 f90, float alpha_roughness, float specular_weight, float VoH, float NoL, float NoV, float NoH)
{
	float3 F = f_schlick(f0, f90, VoH);//フラネル
	float Vis = v_ggx(NoL, NoV, alpha_roughness);//幾何学的減衰
	float D = d_ggx(NoH, alpha_roughness);//マイクロファセット法線分布関数

	return specular_weight * F * Vis * D;
}

//physically based rendering
/////L... -light direction 
////V...position->camera position(in world)
////N...normalize normal
///P....world position
float4 physically_based_rendering(material_info material_info,float3 L, float3 V, float3 N, float3 P,float3 light_color)
{
	float3 f_specular = 0.0;
	float3 f_diffuse = 0.0;
	float3 f_emissive = 0.0;
	float3 f_clearcoat = 0.0;
	float3 f_sheen = 0.0;
	float3 f_transmission = 0.0;

	float albedo_sheen_scaling = 1.0;
	// Calculate lighting contribution from image based lighting source (IBL)
	//if (image_based_lighting > 0)//bool ibl
	{
		//lambert
		f_diffuse += ibl_radiance_lambertian(N, V, material_info.perceptual_roughness, material_info.c_diff, material_info.f0, material_info.specular_weight);
		//specular
		f_specular += ibl_radiance_ggx(N, V, material_info.perceptual_roughness, material_info.f0, material_info.specular_weight,light_color);

	}
	
	float3 R = reflect(-L, N);
	float3 H = normalize(V + L);

	float NoL = max(0.0, dot(N, L));//dot normal Vec*light Vec
	float NoH = max(0.0, dot(N, H));//dot normal Vec*half Vec
	float HoV = max(0.0, dot(H, V));//dot half Vec*eye Vec
	float NoV = max(0.0, dot(N, V));//dot normal Vec*eye Vec

	float3 Li = material_info.pure_white;
	if (NoL > 0.0 || NoV > 0.0)
	{
		// Calculation of analytical light//分析光の算出
		// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
		//lambert
		f_diffuse += Li * NoL * brdf_lambertian(material_info.f0, material_info.f90, material_info.c_diff, material_info.specular_weight, HoV);

		//specular
		f_specular += Li * NoL * brdf_specular_ggx(material_info.f0, material_info.f90, material_info.alpha_roughness, material_info.specular_weight, HoV, NoL, NoV, NoH);

	}
		//return float4(f_specular, 1);


	// Apply ambient occlusion to all lighting that is not punctual
	f_diffuse = lerp(f_diffuse, f_diffuse * material_info.occlusion_factor, material_info.occlusion_strength);
	f_specular = lerp(f_specular, f_specular * material_info.occlusion_factor, material_info.occlusion_strength);
	f_sheen = lerp(f_sheen, f_sheen * material_info.occlusion_factor, material_info.occlusion_strength);
	f_clearcoat = lerp(f_clearcoat, f_clearcoat * material_info.occlusion_factor, material_info.occlusion_strength);

	// Apply emissive factor
	f_emissive = material_info.emissive_factor;
#if 0
	f_emissive *= emissive_intensity;
#endif

	float3 color = 0.0;

	// Layer blending
	float clearcoat_factor = 0.0;
	float3 clearcoat_fresnel = 0.0;

	float3 F = f_schlick(material_info.f0, material_info.f90, HoV);//フラネル
	color = f_emissive + f_diffuse*(1-F) + f_specular;
	color = f_sheen + color * albedo_sheen_scaling;
	color = color * (1.0 - clearcoat_factor * clearcoat_fresnel) + f_clearcoat;
	return float4(max(0, color), material_info.basecolor.a);

}