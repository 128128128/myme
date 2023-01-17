//#include "common.hlsli"
#include "differred_light.hlsli"
#include "scene_constants.hlsli"

#include "pbr_ibl_functions.hlsli"

//	textures
Texture2D albedo_map : register(t0);
Texture2D normal_map : register(t1);
Texture2D position_map : register(t2);
Texture2D RM_map : register(t3);
Texture2D light_map : register(t5);

Texture2D envTexture : register(t15);

//#define POINT 0
//#define LINEAR 1
//#define ANISOTROPIC 2
//SamplerState sampler_states[3] : register(s0);

//float4 main(PS_IN input)
//	: SV_TARGET0
//{
//	float4 color = albedo_map.Sample(DecalSampler, input.texcoord);
//	float4 light = light_map.Sample(DecalSampler, input.texcoord);
//
//	//Metallic
//	float metallic = RM_texture.Sample(
//		DecalSampler, input.texcoord).g;
//	//metallic = 0.5;
//	//color.rgb = float3(0, 0, 0);
//	//light.rgb *= 3;
//	float3 m1 = color.rgb * light.rgb;
//	float3 m0 = color.rgb * 0.4;
//	m0 += light.rgb * 0.25;
//	color.rgb = lerp(m0, m1, metallic);
//	return color;
//
//}

//float4 main(PS_IN pin) : SV_TARGET
//{
//    float4 tex = albedo_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//    float4 color = tex;
//    float4 light = light_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//  
//    //metallic
//    float metallic = RM_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord).r;
//
//    float3 m1 = color.rgb * light.rgb;
//    float3 m0 = color.rgb * 0.4;
//    m0 += light.rgb * 0.05;
//    color.rgb = lerp(m0, m1, metallic);
//
//    //Fresnel reflection
//    //gaze
//    float4 P = position_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//    float3 E = P.xyz - camera_constants.position.xyz;
//    E = normalize(E);
//    //normal
//    float4 nor = normal_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//    if (nor.w <= 0.0)
//        return tex;
//    float3 N = nor.xyz;
//   
//    float NoLight = 1.0 - length(N);
//    NoLight = step(0.9, NoLight);
//    N = N * 2.0 - 1.0; //0<-->1 -1<-->+1
//    N = normalize(N);
//    //Line of sight and normal orientation with inner product : 内積で視線と法線の向き合い具合
//    float d = dot(-E, N);
//    float fresnel = saturate(1.0 - d) * 0.6;
//    fresnel = pow(fresnel, 6.0);
//    color.rgb = lerp(color.rgb,
//                light.rgb, fresnel);
//
//    return color;
//
//}

material_info fetch_material_info(float2 texcoord)
{
    material_info material_info;
#if 1
    material_info.basecolor = 1.0;
    material_info.ior = 1.5;//屈折率
    material_info.perceptual_roughness = 1.0;
    material_info.f0 = 0.04;//全反射カラー
    material_info.alpha_roughness = 1.0;//知覚的粗さ
    material_info.c_diff = 1.0;
    material_info.f90 = 1.0;
    material_info.metallic = 1.0;
    material_info.sheen_roughness_factor = 0.0;//シアノラフネス係数
    material_info.sheen_color_factor = 0.0;
    material_info.clearcoat_f0 = 0.04; //GLTF の仕様に従って一様な 4 % と見なされる
    material_info.clearcoat_f90 = 1.0;
    material_info.clearcoat_factor = 0.0;
    material_info.clearcoat_normal = 0.0;
    material_info.clearcoat_roughness = 1.0;
    material_info.specular_weight = 1.0;
    material_info.transmission_factor = 0.0;
    material_info.thickness_factor = 0.0;
    material_info.attenuation_color = 1.0;
    material_info.attenuation_distance = 0.0;
    material_info.occlusion_factor = 1.0;// occlusion_factor;
    material_info.occlusion_strength = 1.0;// occlusion_strength;
#endif



    float4 sampled = albedo_map.Sample(sampler_states[ANISOTROPIC], texcoord);
    //gamma correction
    sampled.rgb = pow(sampled.rgb, gamma);
    //base color = diffuse texture
    material_info.basecolor = sampled;

    float r_factor = 0;
    float m_factor = 0;
    float pure_white = 0;

    sampled = RM_map.Sample(sampler_states[LINEAR], texcoord);
    //roughness , metallic can be controled
    r_factor =  sampled.b;
    m_factor = sampled.r;
    pure_white = sampled.g;

    //rm add to material info
    material_info.metallic = m_factor;
    material_info.perceptual_roughness = r_factor;
    material_info.pure_white = pure_white;

    // Achromatic f0 based on IOR(屈折率).
    material_info.c_diff = lerp(material_info.basecolor.rgb, 0.0, material_info.metallic);
    //// full reflectance color (n incidence angle)// 全反射カラー（n入射角）
    //平面に垂直方向から光が入射した際の鏡面反射率//鏡面反射率を表す材質特性
    material_info.f0 = lerp(material_info.f0, material_info.basecolor.rgb, material_info.metallic);


    //float occlusion_factor = 1.0;
    //const int occlusion_texture = material.occlusion_texture.index;
    //if (occlusion_texture > -1)
    //{
    //	float2 transformed_texcoord = transform_texcoord(texcoord, material.occlusion_texture.khr_texture_transform);
    //	float4 sampled = material_textures[OCCLUSION_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
    //	occlusion_factor *= sampled.r;
    //}
    //material_info.occlusion_factor = occlusion_factor;
    //material_info.occlusion_strength = material.occlusion_texture.strength;

    //emissive values
    float e_factor = 0;
    //sampled = emissive_map.Sample(sampler_states[ANISOTROPIC], texcoord);
    //sampled.rgb = pow(sampled.rgb, gamma);
    //e_factor = emissive_factor * sampled.rgb;//can control
    //register
    material_info.emissive_factor = e_factor;

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness.
    //知覚的粗さを二乗して物質的粗さに変換する
    material_info.alpha_roughness = material_info.perceptual_roughness * material_info.perceptual_roughness;

    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th edition on page 325.
    material_info.f90 = 1.0;//reflectance color at grazing angle//かすむ寸前の角度でのカラー


    return material_info;
}
float4 main(PS_IN pin) : SV_TARGET
{
    material_info material_info = fetch_material_info(pin.texcoord);

   const float3 P = position_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

   material_info.basecolor.a = 1.0;//base color alpha

   float3 V = normalize(camera_constants.position.xyz - P);

   float4 light = light_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
 
   float3 L = normalize(-light_direction.direction.xyz);

   float3 N = normal_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord).rgb;

   return  physically_based_rendering(material_info, L, V, N, P,light.rgb);

}

//SamplerState sceneSampler : register(s0);
//float4 main(PS_IN pin) : SV_TARGET
//{
//	float4 tex = albedo_map.Sample(sceneSampler, pin.texcoord);
//	float4 light = light_map.Sample(sceneSampler, pin.texcoord);
//
//	float4 N = normal_map.Sample(sceneSampler, pin.texcoord);
//	float dist = length(N.xyz * 2.0 - 1.0);
//	float n = step(0.5, dist);
//
//	float4 color = tex * lerp(1.0, light, n);
//	//color = light;
//	return color;
//}