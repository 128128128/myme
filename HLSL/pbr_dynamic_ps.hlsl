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

static const int NUM_DIRECTIONAL_LIGHT = 1; // �f�B���N�V�������C�g�̖{��

float3 GetNormal(float3 normal, float3 tangent, float3 biNormal, float2 uv)
{
	float3 binSpaceNormal = normal_map.SampleLevel(sampler_states[LINEAR], uv, 0.0f).xyz;
	binSpaceNormal = (binSpaceNormal * 2.0f) - 1.0f;

	float3 newNormal = tangent * binSpaceNormal.x + biNormal * binSpaceNormal.y + normal * binSpaceNormal.z;

	return newNormal;
}

//float4 main(VS_OUT pin) : SV_TARGET
//{
//	// �@�����v�Z
//     float3 N = GetNormal(pin.normal, pin.tangent, pin.binormal, pin.texcoord);
//
//	float4 albedo_color = diffuse_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//
//	float alpha = albedo_color.a;
//
//	// �X�y�L�����J���[�̓A���x�h�J���[�Ɠ����ɂ���
//	float3 spec_color = albedo_color;
//
//	// �����x
//	float metallic = metallic_map.Sample(sampler_states[POINT], pin.texcoord).r;
//
//	// ���炩��
//	float smooth = metallic_map.Sample(sampler_states[POINT], pin.texcoord).a;
//
//	float3 L = normalize(-light_direction.direction.xyz);
//	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
//
//	//�����Ɍ������x�N�g���Ǝ����Ɍ������x�N�g���̃n�[�t�x�N�g�������߂�
//	float3 H = normalize(L + E);
//	// �e��x�N�g�����ǂꂭ�炢���Ă��邩����ς𗘗p���ċ��߂�
//	float NdotH = saturate(dot(N, H));
//	float VdotH = saturate(dot(E, H));
//	float NdotL = saturate(dot(N, L));
//	float NdotV = saturate(dot(N, E));
//
//	float PI = 3.1415926f;
//
//
//	//-----< �t���l�����˂��l�������g�U���ˌ� >-----//
//	//�e����0.5�ŌŒ�
//	float roughness = smooth;
//
//	float energyBias = lerp(0.0f, 0.5f, roughness);
//	float energyFactor = lerp(1.0f, 1.0 / 1.51, roughness);
//
//	//�����Ɍ������x�N�g���ƃn�[�t�x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
//	float dotLH = saturate(dot(L, H));
//
//	//�����Ɍ������x�N�g���ƃn�[�t�x�N�g���A
//	//�������s�ɓ��˂������̊g�U���˗�
//	float Fd90 = energyBias + 2.0f * dotLH * dotLH * roughness;
//
//	//�@���ƌ����Ɍ������x�N�g���𗘗p���Ċg�U���˗������߂�
//	float dotNL = saturate(dot(N, L));
//	float FL = (1 + (Fd90 - 1) * pow(1 - dotNL, 5));
//
//	//�@���Ǝ����Ɍ������x�N�g���𗘗p���Ċg�U���˗������߂�
//	float dotNE = saturate(dot(N, E));
//	float FE = (1 + (Fd90 - 1) * pow(1 - dotNE, 5));
//
//	//�@���ƌ����ւ̕����Ɉˑ�����g�U���˗��ƁA�@���Ǝ����x�N�g���Ɉˑ�����g�U���˗�����Z����
//	//�ŏI�I�Ȋg�U���˗������߂Ă���
//	//PI�ŏ��Z���Ă���̂͐��K�����s������
//	float diffuseFromFresnel = (FL * FE * energyFactor);
//	//-----------------------------------------//
//
//
//	//���K��Lambert�g�U���˂����߂�
//	float3 lambertDiffuse = light_direction.color.xyz * NdotL / PI;
//
//	//�ŏI�I�Ȋg�U���ˌ����v�Z
//	float3 diffuse = albedo_color.xyz * diffuseFromFresnel * lambertDiffuse;
//
//
//
//	//-----< Cook-Torrance���f���𗘗p�������ʔ��˗����v�Z >-----//
//	float microfacet = 0.76f;
//
//	// �����x�𐂒����˂̎��̃t���l�����˗��Ƃ��Ĉ���
//	// �����x�������قǃt���l�����˂͑傫���Ȃ�
//	float f0 = metallic;
//
//	// D�����x�b�N�}�����z��p���Čv�Z����
//	float D = Beckmann(microfacet, NdotH);
//
//	// F����Schlick�ߎ���p���Čv�Z����
//	float F = SpcFresnel(f0, VdotH);
//
//	// G�������߂�
//	float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));
//
//	// m�������߂�
//	float m = PI * NdotV * NdotH;
//
//	// �����܂ŋ��߂��A�l�𗘗p���āACook-Torrance���f���̋��ʔ��˂����߂�
//	float3 specular = max(F * D * G / m, 0.0);
//	//-------------------------------------------------------//
//
//
//	//�����x��������΁A���ʔ��˂̓X�y�L�����J���[�A�Ⴏ��Δ�
//	//�X�y�L�����J���[�̋��������ʔ��˗��Ƃ��Ďg��
//	specular *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);
//
//
//	
//	float3 light = 0;
//
//	//���炩�����g���āA�g�U���ˌ��Ƌ��ʔ��ˌ�������
//   //���炩����������΁A�g�U���˂͎キ�Ȃ�
//	light += diffuse * (1.0f - smooth) + specular;
//
//	//�����ɂ���グ
//	float3 ambientLight = float3(1,1,1);
//	light += ambientLight * albedo_color.xyz;
//
//	float4 color = alpha;
//	color.xyz = light * light_direction.color.w;
//
//#if 0
//	//�t�K���}�␳(Inverse gamma process)
//	const float GAMMA = 2.2;
//	color.rgb = pow(color.rgb, 1.0f / GAMMA);
//#endif
//
//
//	return color;
//
//	//float lig = 0;
//	////�V���v���ȃf�B�Y�j�[�x�[�X�̊g�U���˂���������
//	//// �t���l�����˂��l�������g�U���˂��v�Z
//	//float diffuseFromFresnel = CalcDiffuseFromFresnel(N, L, E);
//
//	//// ���K��Lambert�g�U���˂����߂�
//	//float NdotL = saturate(dot(N, L));
//	//float3 lambertDiffuse = light_direction.color * NdotL / PI;
//
//	//// �ŏI�I�Ȋg�U���ˌ����v�Z����
//	//float3 diffuse = albedo_color * diffuseFromFresnel * lambertDiffuse;
//
//	////Cook-Torrance���f���𗘗p�������ʔ��˗����v�Z����
//	//// Cook-Torrance���f���̋��ʔ��˗����v�Z����
//	//float3 spec = CookTorranceSpecular(-L, E, N, smooth) * light_direction.color;
//
//	//// �����x��������΁A���ʔ��˂̓X�y�L�����J���[�A�Ⴏ��Δ�
//	//// �X�y�L�����J���[�̋��������ʔ��˗��Ƃ��Ĉ���
//	//spec *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);
//
// //   //���炩�����g���āA�g�U���ˌ��Ƌ��ʔ��ˌ�����������
// //   // ���炩����������΁A�g�U���˂͎キ�Ȃ�
//	//lig +=albedo_color;
// //   lig += diffuse * (1.0f - smooth) + spec;
//	//
//
//	//// �����ɂ���グ
//
//	//float4 finalColor = 1.0f;
//	//finalColor.xyz = lig*light_direction.direction.w;
//
//	//return finalColor;
//}

float4 main(VS_OUT pin) : SV_TARGET
{
	// �@�����v�Z
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
	// �X�y�L�����J���[�̓A���x�h�J���[�Ɠ����ɂ���
	float3 spec_color = albedo_color;

	// �����x
	float metallic = metallic_map.Sample(sampler_states[POINT], pin.texcoord).r;
    metallic *= metallic_param;
	// ���炩��
	float smooth = metallic_map.Sample(sampler_states[POINT], pin.texcoord).a;
    smooth *= smooth_param;
	// �����Ɍ������ĐL�т�x�N�g�����v�Z����
	float3 toEye = normalize(camera_constants.position.xyz - pin.position.xyz);

    float3 lig = 0;
    for (int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        // �V���v���ȃf�B�Y�j�[�x�[�X�̊g�U���˂���������B
        // �t���l�����˂��l�������g�U���˂��v�Z
        float diffuseFromFresnel = CalcDiffuseFromFresnel(
            N, -light_direction.direction[ligNo], toEye);

        // ���K��Lambert�g�U���˂����߂�
        float NdotL = saturate(dot(N, -light_direction.direction[ligNo]));
        float3 lambertDiffuse = light_direction.color[ligNo] * NdotL / PI;

        // �ŏI�I�Ȋg�U���ˌ����v�Z����
        float3 L = normalize(-light_direction.direction.xyz);
        float3 diffuse = albedo_color * diffuseFromFresnel * lambertDiffuse;
        diffuse = albedo_color.rgb * max(0, dot(N, L) * 0.5 + 0.5);
        
        // Cook-Torrance���f���𗘗p�������ʔ��˗����v�Z����
        // Cook-Torrance���f���̋��ʔ��˗����v�Z����
        float3 spec = CookTorranceSpecular(
            -light_direction.direction[ligNo], toEye, N, smooth)
            * light_direction.color[ligNo];

        // �����x��������΁A���ʔ��˂̓X�y�L�����J���[�A�Ⴏ��Δ�
        // �X�y�L�����J���[�̋��������ʔ��˗��Ƃ��Ĉ���
        spec *= lerp(float3(1.0f, 1.0f, 1.0f), spec_color, metallic);

        // ���炩�����g���āA�g�U���ˌ��Ƌ��ʔ��ˌ�����������
        // ���炩����������΁A�g�U���˂͎キ�Ȃ�
        lig += diffuse * (1.0f - smooth) + spec;
    }

    // �����ɂ���グ
    float ambientLight = float3(1, 1, 1);
    lig += ambientLight * albedo_color;

    //float4 emis = emissive_map.Sample(sampler_states[ANISOTROPIC], pin.texcoord);

    float4 finalColor = 1.0f;
    finalColor.xyz = lig;

    //finalColor= finalColor + emis * float4(albedo_color.r, albedo_color.g, albedo_color.b, albedo_color.w)*1.6;
    return finalColor;
}