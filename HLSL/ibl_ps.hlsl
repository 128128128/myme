#include "pbr_dynamic_mesh.hlsli"
#include "physically_based_rendering.hlsli"
#include "scene_constants.hlsli"
//#include "shader_functions.hlsli"

static const int NUM_DIRECTIONAL_LIGHT = 1; 

float3 GetNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
    float3 binSpaceNormal = normal_map.SampleLevel(sampler_states[LINEAR_WRAP], uv, 0.0f).xyz;
    binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

    float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

    return newNormal;
}

material_info fetch_material_info(float2 texcoord)
{
    material_info material_info;
#if 1
    material_info.basecolor = 1.0;
    material_info.ior = 1.5;
    material_info.perceptual_roughness = 1.0;
    material_info.f0 = 0.04;
    material_info.alpha_roughness = 1.0;
    material_info.c_diff = 1.0;
    material_info.f90 = 1.0;
    material_info.metallic = 1.0;
    material_info.sheen_roughness_factor = 0.0;
    material_info.sheen_color_factor = 0.0;
    material_info.clearcoat_f0 = 0.04;
    material_info.clearcoat_f90 = 1.0;
    material_info.clearcoat_factor = 0.0;
    material_info.clearcoat_normal = 0.0;
    material_info.clearcoat_roughness = 1.0;
    material_info.specular_weight = 1.0;
    material_info.transmission_factor = 0.0;
    material_info.thickness_factor = 0.0;
    material_info.attenuation_color = 1.0;
    material_info.attenuation_distance = 0.0;
    material_info.occlusion_factor = occlusion_factor;
    material_info.occlusion_strength = occlusion_strength;
#endif



    float4 sampled = diffuse_map.Sample(sampler_states[ANISOTROPIC_WRAP], texcoord);
    sampled.rgb = pow(sampled.rgb, gamma);

    material_info.basecolor = sampled;

    float r_factor = 0;
    float m_factor = 0;

    sampled = metallic_map.Sample(sampler_states[LINEAR_WRAP], texcoord);
    r_factor = roughness_factor * sampled.a;// g;
    m_factor = metallic_factor * sampled.r;// b;

    material_info.metallic = m_factor;
    material_info.perceptual_roughness = r_factor;

    // Achromatic f0 based on IOR.
    material_info.c_diff = lerp(material_info.basecolor.rgb, 0.0, material_info.metallic);
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

    float e_factor=0;
    sampled = emissive_map.Sample(sampler_states[ANISOTROPIC_WRAP], texcoord);
    sampled.rgb = pow(sampled.rgb, gamma);
    e_factor=emissive_factor * sampled.rgb;

    material_info.emissive_factor = e_factor;

    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness.
    material_info.alpha_roughness = material_info.perceptual_roughness * material_info.perceptual_roughness;

    // Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
    material_info.f90 = 1.0;


    return material_info;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    material_info material_info = fetch_material_info(pin.texcoord);

    const float3 P = pin.position.xyz;

    material_info.basecolor.a = 1.0;


    float3 V = normalize(camera_constants.position.xyz - pin.position.xyz);

    float3 L = normalize(-light_direction.direction.xyz);

     float3 N = normalize(pin.normal.xyz);
     float3 T = float3(pin.tangent.xyz);
     float sigma = pin.tangent.w;
     T = normalize(T - N * dot(N, T));
     float3 B = normalize(cross(N, T) * sigma);
   
     float4 sampled = normal_map.Sample(sampler_states[LINEAR_WRAP], pin.texcoord);
     float3 normal_factor = sampled.xyz;
     normal_factor = (normal_factor * 2.0) - 1.0;
     normal_factor = normalize(normal_factor * float3(1.0, 1.0, 1.0));
     N = normalize((normal_factor.x * T) + (normal_factor.y * B) + (normal_factor.z * N));

     return physically_based_rendering(material_info, L, V, N, P);

    //float4 albedo_color = diffuse_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    //// �X�y�L�����J���[�̓A���x�h�J���[�Ɠ����ɂ���
    //float3 spec_color = albedo_color;

    //// �����x
    //float metallic = metallic_map.Sample(sampler_states[POINT], pin.texcoord).r;
    //metallic *= metallic_param;
    //// ���炩��
    //float smooth = metallic_map.Sample(sampler_states[POINT], pin.texcoord).a;
    //smooth *= smooth_param;
    //// �����Ɍ������ĐL�т�x�N�g�����v�Z����
    //float3 toEye = normalize(camera_constants.position.xyz - pin.position.xyz);

    //float3 lig = 0;
    //for (int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    //{
    //    // �V���v���ȃf�B�Y�j�[�x�[�X�̊g�U���˂���������B
    //    // �t���l�����˂��l�������g�U���˂��v�Z
    //    float diffuseFromFresnel = CalcDiffuseFromFresnel(
    //        N, -light_direction.direction[ligNo], toEye);

    //    // ���K��Lambert�g�U���˂����߂�
    //    float NdotL = saturate(dot(N, -light_direction.direction[ligNo]));
    //    float3 lambertDiffuse = light_direction.color[ligNo] * NdotL / PI;

    //    // �ŏI�I�Ȋg�U���ˌ����v�Z����
    //    float3 L = normalize(-light_direction.direction.xyz);
    //    float3 diffuse = albedo_color * diffuseFromFresnel * lambertDiffuse;
    //    diffuse = albedo_color.rgb * max(0, dot(N, L) * 0.5 + 0.5);

    //    // Cook-Torrance���f���𗘗p�������ʔ��˗����v�Z����
    //    // Cook-Torrance���f���̋��ʔ��˗����v�Z����
    //    float3 spec = CookTorranceSpecular(
    //        -light_direction.direction[ligNo], toEye, N, smooth)
    //        * light_direction.color[ligNo];

    //    // �����x��������΁A���ʔ��˂̓X�y�L�����J���[�A�Ⴏ��Δ�
    //    // �X�y�L�����J���[�̋��������ʔ��˗��Ƃ��Ĉ���
    //    spec *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);

    //    // ���炩�����g���āA�g�U���ˌ��Ƌ��ʔ��ˌ�����������
    //    // ���炩����������΁A�g�U���˂͎キ�Ȃ�
    //    lig += diffuse * (1.0f - smooth) + spec;
    //}

    //// �����ɂ���グ
    //float ambientLight = float3(1, 1, 1);
    //lig += ambientLight * albedo_color;

    ////float4 emis = emissive_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    //float4 finalColor = 1.0f;
    //finalColor.xyz = lig;

    ////finalColor= finalColor + emis * float4(albedo_color.r, albedo_color.g, albedo_color.b, albedo_color.w)*1.6;
    //return finalColor;
}