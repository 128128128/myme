//����//Todo
#include "pbr_common.hlsli"
//texture
Texture2D albedo_texture : register(t0);
Texture2D normal_texture : register(t1);
//Texture2D light_texture : register(t1);
Texture2D position_texture : register(t2);
Texture2D rm_texture : register(t3);
Texture2D depth_texture : register(t4);
Texture2D shadow_param_texture : register(t7);    // �eparam
Texture2D env_texture : register(t15);

SamplerComparisonState comparison_sampler_state : register(s4);

//SSR
// �f�肱�މ�ʃe�N�X�`����UV��Ԃ�
// P : ���W�@R:�����̔���
float2 SSR(float3 P, float3 R)
{
    // ���˃x�N�g��(�����Ɩ@���Ŕ���)
    R = normalize(R);
    // ������Ƃ��i�߂�
    R *= 0.3; //0.1 = ���Ői�߂鋗��
    [loop] //Loop�̂܂�
    for (int i = 0; i < 100; i++) {
        P += R; //�i�߂�
        // �����ɂԂ������H
        // P�܂ł̋�����
        float DistP = length(P - camera_constants.position);
        // �����_�����O���ʂ̋����Ŕ�r
        // P�̃X�N���[�����W �� UV �� Depth���狗���擾
        float4 screenP = mul(float4(P, 1.0), camera_constants.view_projection);
        // �X�N���[�����W(-1 <--> 1) �� UV(0<-->1)
        float2 Puv = (screenP.xy / screenP.w) * 0.5 + 0.5;
        Puv.y = 1.0 - Puv.y; //Y���]
        // ��ʊO����
        if (Puv.x < 0) return -1;
        if (Puv.y < 0) return -1;
        if (Puv.x > 1) return -1;
        if (Puv.y > 1) return -1;

        // Depth���狗���擾
        float DistS = depth_texture.Sample(
            decal_sampler, Puv).x;

        // ������r(�Q�ȏ�߂��ꍇ��NG)
        if (DistP > DistS && DistP - 2 < DistS) {
            // ���� = �f�荞�݃|�C���g
            return Puv;
        }
    }
    return -1; // �f�荞�ݖ�
}


float4 applySSR(half3 normal, float3 viewPosition, half2 screenUV)
{
    float4 color = albedo_texture.Sample(decal_sampler, screenUV);

    float3 position = viewPosition;
    float2 uvNow;
    float2 uvScale = (screenUV - 0.5f) * float2(2.0f, -2.0f);

    float3 reflectVec = normalize(reflect(normalize(viewPosition), normal));// ���˃x�N�g��
    const int iteration = 200;  // �J��Ԃ���
    const int maxLength = 10;// ���ˍő勗��
    float3 delta = reflectVec * maxLength / iteration;// �P��Ői�ދ���
    [loop]
    for (int i = 0; i < iteration; i++)
    {
        position += delta;
        // ���݂̍��W���ˉe�ϊ�����UV�l�����߂�
        float4 projectPosition = mul(camera_constants.projection, float4(position, 1.0));
        uvNow = projectPosition.xy / projectPosition.w * 0.5f + 0.5f;
        uvNow.y = 1.0f - uvNow.y;
        // z�o�b�t�@�̒l���擾������܂��t�ϊ�
        float4 pos;
        pos.z = depth_texture.Sample(decal_sampler, uvNow).r;
        pos.xy = uvScale;
        pos.w = 1.0f;
        pos = mul(camera_constants.inverse_view_projection, pos);
        float z = pos.z / pos.w;
        // Z�l���r
        [branch]
        if (position.z < z && position.z + radians(15) > z)
        {
            // ���������̂ŐF���u�����h����
            return lerp(color, albedo_texture.Sample(decal_sampler, uvNow), radians(15));
        }
    }
    return color;
}

float4 main(PS_IN pin) : SV_Target0
{
    //Writing with G-Buffer contents
    //Sampling albedo color
    float4 albedo_color = albedo_texture.Sample(decal_sampler, pin.texcoord);
    //Sampling normal color
    float3 normal = normal_texture.Sample(decal_sampler, pin.texcoord).xyz;
    //Restore the normal from a range of 0 to 1 to a range of -1 to 1
    normal = (normal * 2.0f) - 1.0f;
    //Sampling world position
    float3 worldPos = pin.worldpos;
    // Specular color should be the same as albedo color.
    float3 specColor = albedo_color;
    // Sampling metallicity and smoothness
    float4 metallic_smooth = rm_texture.Sample(decal_sampler, pin.texcoord);

    //Parameters for shadow generation.
   // float4 shadow_param= shadow_param_texture.Sample(decal_sampler, pin.texcoord);
    // Calculate the vector extending toward the line of sight
    float3 to_eye = normalize(camera_constants.position.xyz - worldPos);

    float3 lig = 0;

    // �����̔���
    float4 P = position_texture.Sample(decal_sampler, pin.texcoord);
    float3 E = P.xyz - camera_constants.position.xyz;//����
    float3 N = normalize(normal);
    float3 R = reflect(normalize(E), N);

    // R���㉺�ƕ����ɕ���
    float2 v;
    v.y = R.y; //Y�͂��̂܂�
    float3 work = R;
    work.y = 0; //��������
    work = normalize(work); //���p�̂�
    v.x = work.x;
    //�Ђ��݂𒼂�
    //acos(-1)=180�� acos(0)=90�� acos(1)=0 
    //v.x = acos(v.x);
    //v.x /= 3.14159266; //-1=1.0 0=0.5 1=0
    //v.x = (-v.x) * 2.0 + 1.0;

    float2 uv = v;
    uv.y = -uv.y;
    uv = uv * 0.5 + 0.5;
    uv.x *= 0.5;
    // �����
    if (R.z > 0) {
        uv.x = 1.0 - uv.x;
    }
    //Roughness���l���Ă݂�
    float4 RM = rm_texture.Sample(decal_sampler, pin.texcoord);
    float roughness = 0.0;
    //r 0.0  0.3   0.5  1.0 
    //l 0.0  8.0  10.0  11.0
    float w = 1.0 - pow(1.0 - roughness, 0.0);
    float level = w * 12.0;
    float4 env = env_texture.SampleLevel(decal_sampler, uv, level);
    //tone
    env.rgb = env.rgb / (100 + env.rgb);
    //SSR �f�荞�݂�UV���󂯎��
    float2 ssr = SSR(P, R);
    if (ssr.x >= 0) {
        float4 texR = albedo_texture.Sample(
            decal_sampler, ssr);
        // �u�����h
        float rate = roughness;
        rate = rate * 0.6 + 0.1; //0.4---1.0
        env = lerp(texR, env, rate);
    }
    lig += env;

    for (int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        // �e�̗�������v�Z����B
       // float shadow = 0.0f;
       //// if (directionalLight[ligNo].castShadow == 1) {
       //     //�e�𐶐�����Ȃ�B
       //     shadow = calc_shadow_rate(ligNo, worldPos) * shadow_param.r;
       // //}
       // if (shadow > 0.9f) {
       //     //���C�g�̌v�Z�����Ȃ��B
       //     //�e�������Ă���Ɗ����̉e����������B
       //     continue;
       // }

        // Implement Disney-based diffuse reflection
        // Calculate diffuse reflections taking Fresnel reflections into account
        float diffuseFromFresnel = CalcDiffuseFromFresnel(
            normal,-light_direction.direction.xyz /*-directionalLight[ligNo].direction*/, to_eye);

        // Find the normalized Lambert diffuse reflection
        float NdotL = saturate(dot(normal, -light_direction.direction.xyz /*-directionalLight[ligNo].direction*/));
        float3 lambertDiffuse = light_direction.color.xyz/*directionalLight[ligNo].color*/ * NdotL / PI;

        // Calculate the final diffuse reflected light
        float3 diffuse = albedo_color * diffuseFromFresnel * lambertDiffuse;


        // Calculate specular reflectance using the Cook-Trans model
        // Calculate the specular reflectance of the cooktrans model
        float3 spec = CookTorranceSpecular(
            -light_direction.direction.xyz /*-directionalLight[ligNo].direction*/, to_eye, normal, metallic_smooth.w)
            * light_direction.color.xyz/*directionalLight[ligNo].color*/;

        // If metallicity is high, specular reflection is specular color, if low, white
        // Treat specular color intensity as specular reflectance
        spec *= lerp(float3(1.0f, 1.0f, 1.0f), specColor, metallic_smooth.x);

        // Use smoothness to combine diffuse and specular reflected light
        // The higher the smoothness, the weaker the diffuse reflection.
        lig += diffuse * (1.0f - metallic_smooth.w) + spec * metallic_smooth.w;


        }



    float3 ambient_light = float3(1.0, 1.0, 1.0);
    // Raise the bottom line with ambient light
    lig += ambient_light * albedo_color;


    float4 final_color = 1.0f;
    final_color.xyz = lig;
    return final_color;
}


