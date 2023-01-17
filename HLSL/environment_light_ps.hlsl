//#include "scene_constants.hlsli"
//#include "differred_light.hlsli"
//
//Texture2D albedo_map : register(t0);
//Texture2D normal_map : register(t1);
//Texture2D position_map : register(t2);
//Texture2D metal_roughness_map : register(t3);
//Texture2D emissive_map : register(t4);
//Texture2D env_map : register(t15);
//
//#define POINT 0
//#define LINEAR 1
//#define ANISOTROPIC 2
//SamplerState sampler_states[3] : register(s0);
//float4 main(PS_IN pin) : SV_TARGET
//{
//    float4 normal = normal_map.Sample(sampler_states[LINEAR], pin.texcoord);
//    float3 emissive = emissive_map.Sample(sampler_states[LINEAR], pin.texcoord).rgb;
//    //state of elevation��normal's Y
//    float3 N = normalize(normal.xyz * 2.0 - 1.0);
//
//    //Gaze Reflection
//    float4 P = position_map.Sample(sampler_states[LINEAR], pin.texcoord);
//    float3 E = P.xyz - camera_constants.position.xyz;
//    float3 R = reflect(normalize(E), N);
//     //float3 R = reflect(N, normalize(E));
//    float3 dir = R;
//    dir.y = 0; //Height invalid
//    dir = normalize(dir);
//    float2 uv;
//    uv.x = dir.x;
//    //Eliminate angle distortion
//    //�p�x�̂Ђ��݂𖳂��� //�������x�Ƒ��k acos���d��
//    //-1<--->+1 ��0<--->1.0
//    ///uv.x = acos(uv.x) / 3.141593;
//    //0.0<--->1.0 ��-1<--->+1
//    uv.x = uv.x * 2.0 - 1.0;
//
//    uv.y = R.y;
//
//    uv.y = -uv.y;
//    uv = uv * 0.5 + 0.5;
//
//    //East-West Coordination : ��������
//    uv.x *= 0.5;
//    //north-south adjustment : ��k����
//   if (R.z > 0)
//    {
//       uv.x = 1 - uv.x;
//   }
//
//    //Roughness=1-smoothness
//    float Roughness = metal_roughness_map.Sample(sampler_states[LINEAR], pin.texcoord).a;
//
//    //R		0	0.1	   0.25     0.4	  1.0
//    //N		0   0.45   0.72     0.81  1.0      //���K�����
//    //Lv	0	5	   8	    9 	  11
//    //1-R	1	0.9	   0.75		0.6   0.0
//    //1-	1   0.55   0.28     0.19  0.0
//    float level = pow(Roughness, 5.0);
//    //level *= 11;
//    //Blur using mipmap
//    float3 light = env_map.SampleLevel(sampler_states[LINEAR], uv, level);
//    
//    //emissive
//   //light += emissive;
//
//   light= (normal.w <= 0) ? float3(1, 1, 1) : light + emissive;
//   float4 c=env_map.Sample(sampler_states[LINEAR], pin.texcoord);
//    return float4(light.rgb, 1.0);
//    return float4(c.rgb, 1.0);
//}

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

SamplerState ClampSampler : register(s1);

//SSR
// �f�肱�މ�ʃe�N�X�`����UV��Ԃ�
// P : ���W�@R:�����̔���
float2 ssr(float3 P, float3 R)
{
    // reflect vec //���˃x�N�g��(�����Ɩ@���Ŕ���)
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
        // check out of screen ��ʊO����
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
    return -1; // non reflect �f�荞�ݖ�
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
    // get normal
      float4 tex = normal_texture.Sample(decal_sampler, pin.texcoord);
      float3 orgN = tex.xyz * 2.0 - 1.0;
      float3 N = normalize(orgN);

      // �����̔���
      float4 P = position_texture.Sample(decal_sampler, pin.texcoord);
      float3 E = P.xyz - camera_constants.position.xyz;//����
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
      float roughness = RM.b;
      //r 0.0  0.3   0.5  1.0 
      //l 0.0  8.0  10.0  11.0
      float w = 1.0 - pow(1.0 - roughness, 5.0);
      float level = w * 2.0;
      float4 env = env_texture.SampleLevel(ClampSampler, uv, level);
      //�P���ȃg�[���}�b�s���O
      env.rgb = env.rgb / (100 + env.rgb);

      //SSR �f�荞�݂�UV���󂯎��
      float2 l_ssr = ssr(P, R);
      if (l_ssr.x >= 0) {
          float4 texR = albedo_texture.Sample(
              decal_sampler, l_ssr);
          // blend based roughness�u�����h
          float rate = roughness;
          rate = rate * 0.6 + 0.0; //0.4---1.0
          env =  lerp(texR, env, roughness);
      }

      return env;
}


