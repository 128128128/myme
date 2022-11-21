#include "scene_constants.hlsli"
#include "differred_light.hlsli"

Texture2D albedo_map : register(t0);
Texture2D normal_map : register(t1);
Texture2D position_map : register(t2);
Texture2D metal_smooth_map : register(t3);
Texture2D emissive_map : register(t4);
Texture2D env_map : register(t15);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
float4 main(PS_IN pin) : SV_TARGET
{
    float4 normal = normal_map.Sample(sampler_states[LINEAR], pin.texcoord);
    float3 emissive = emissive_map.Sample(sampler_states[LINEAR], pin.texcoord).rgb;
    //state of elevationnormal's Y
    float3 N = normalize(normal.xyz * 2.0 - 1.0);

    //Gaze Reflection
    float4 P = position_map.Sample(sampler_states[LINEAR], pin.texcoord);
    float3 E = P.xyz - camera_constants.position.xyz;
    float3 R = reflect(normalize(E), N);
     //float3 R = reflect(N, normalize(E));
    float3 dir = R;
    dir.y = 0; //Height invalid
    dir = normalize(dir);
    float2 uv;
    uv.x = dir.x;
    //Eliminate angle distortion
    //Šp“x‚Ì‚Ğ‚¸‚İ‚ğ–³‚­‚· //ˆ—‘¬“x‚Æ‘Š’k acos‚ªd‚¢
    //-1<--->+1 ‚ª0<--->1.0
    ///uv.x = acos(uv.x) / 3.141593;
    //0.0<--->1.0 ‚ğ-1<--->+1
    uv.x = uv.x * 2.0 - 1.0;

    uv.y = R.y;

    uv.y = -uv.y;
    uv = uv * 0.5 + 0.5;

    //East-West Coordination : “Œ¼’²®
    uv.x *= 0.5;
    //north-south adjustment : “ì–k’²®
   if (R.z > 0)
    {
       uv.x = 1 - uv.x;
   }

    //Roughness=1-smoothness
    float Roughness = 1 - metal_smooth_map.Sample(sampler_states[LINEAR], pin.texcoord).g;

    //R		0	0.1	   0.25     0.4	  1.0
    //N		0   0.45   0.72     0.81  1.0      //³‹K‰»ó‘Ô
    //Lv	0	5	   8	    9 	  11
    //1-R	1	0.9	   0.75		0.6   0.0
    //1-	1   0.55   0.28     0.19  0.0
    float level = pow(Roughness, 5.0);
    //level *= 11;
    //Blur using mipmap
    float3 light = env_map.SampleLevel(sampler_states[LINEAR], uv, level);
    
    //emissive
   //light += emissive;

   light= (normal.w <= 0) ? float3(1, 1, 1) : light + emissive;
   float4 c=env_map.Sample(sampler_states[LINEAR], pin.texcoord);
    //return float4(light.rgb, 1.0);
    return float4(c.rgb, 1.0);
}