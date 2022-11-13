#include "pbr_common.hlsli"
//texture
Texture2D albedo_texture : register(t0);
Texture2D normal_texture : register(t1);
//Texture2D light_texture : register(t1);
Texture2D position_texture : register(t2);
Texture2D rm_texture : register(t3);
Texture2D depth_texture : register(t4);
Texture2D shadow_param_texture : register(t7);    // 影param
Texture2D env_texture : register(t15);

SamplerComparisonState comparison_sampler_state : register(s4);


//float4 main(PS_IN input) : SV_TARGET0
//{
//	float4 tex = NormalTexture.Sample(DecalSampler, input.texcoord);// *input.Color;
//	
//	//point light
//	float3 PLightPos = LightDir.xyz;
//	float4 P = PositionTexture.Sample(DecalSampler, input.texcoord);
//	float3 PLightDir = P - PLightPos;
//	// ライトタイプ判定(0:平行光 1:点光源)
//	float lighttype = step(0.01, LightDir.w);
//	// 方向決定(平行光:w=0.0 点光源:w>0)
//	//float3 dir = LightDir.w < 0.001 ? LightDir.xyz : PLightDir;
//	float3 dir = lerp(
//	  LightDir.xyz, PLightDir, lighttype);
//	//減衰
//	float attenuation = LightDir.w < 0.001 ?
//		1.0 :
//		1.0 - (length(PLightDir) / LightDir.w);
//	attenuation = saturate(attenuation); //0.0<--->1.0
//	// ライト計算
//	float3 L = dir; //方向
//	float3 C = LightColor.rgb; //カラー
//	C *= attenuation; // 減衰
//	float3 N = tex * 2.0 - 1.0; //法線
//	L = normalize(L);
//	N = normalize(N);
//	// 当たり具合 = -cos = -内積
//	float i = -dot(N, L);
//	i = saturate(i); // 0.0～1.0
//	tex.rgb = C * i; // ライトカラー決定
//	return tex;
//}

//float4 main(PS_IN pin) : SV_TARGET0
//{
//	float4 normal = normal_texture.Sample(decal_sampler, pin.texcoord);// *input.Color;
//	float4 albedo = albedo_texture.Sample(decal_sampler, pin.texcoord);
//
//     //world position Calculate from position texture
//    float4 world_pos;
//
//     //zpos from position texture
//    world_pos.z = position_texture.Sample(decal_sampler, pin.texcoord).r;
//    // xypos calculate from UV
//    world_pos.xy = pin.texcoord * float2(2.0f, -2.0f) - 1.0f;
//    world_pos.w = 1.0f;
//     //go back world matrix
//    world_pos = mul(camera_constants.inverse_view_projection, world_pos);
//    world_pos.xyz /= world_pos.w;
//
//    normal = (normal * 2.0f) - 1.0f;
//
//     //lambert
//    float3 lig = 0.0f;
//    float t = max(0.0f, dot(normal, light_direction.direction) * -1.0f);
//    lig = light_direction.color * t;
//
//    // specular
//    float3 to_eye = normalize(camera_constants.position.xyz - world_pos.xyz);
//    float3 r = reflect(light_direction.direction, normal);
//    t = max(0.0f, dot(to_eye, r));
//    t = pow(t, 1.0f);
//
//    float3 specColor = light_direction.color * t;
//
//    //specular strength
//    float specPower = normal_texture.Sample(decal_sampler, pin.texcoord).w;
//
//   // Multiply specular strength
//    specColor *= specPower;
//
//    lig += specColor;
//   //ambient light
//
//    float4 finalColor = albedo;
//    finalColor.xyz *= lig;
//    return finalColor;
//}


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

    for (int ligNo = 0; ligNo < NUM_DIRECTIONAL_LIGHT; ligNo++)
    {
        // 影の落ち具合を計算する。
       // float shadow = 0.0f;
       //// if (directionalLight[ligNo].castShadow == 1) {
       //     //影を生成するなら。
       //     shadow = calc_shadow_rate(ligNo, worldPos) * shadow_param.r;
       // //}
       // if (shadow > 0.9f) {
       //     //ライトの計算をしない。
       //     //影が落ちていると環境光の影響も下げる。
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
    return final_color ;
}
