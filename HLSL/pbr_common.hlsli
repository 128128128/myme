#include "differred_light.hlsli"
#include "scene_constants.hlsli"

static const float PI = 3.1415926f;         // π
static const int NUM_DIRECTIONAL_LIGHT = 1; // number of dir light

static const int NUM_SHADOW_MAP = 3;        // number of shadow map
Texture2D shadow_map:register(t14);

SamplerState decal_sampler : register(s0);


// Beckmann distribution
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}

// Calculate Fresnel (use Schlick approximate)
float SpcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}


/// <summary>
/// the specular reflection of the cook-trans model
/// </summary>
/// <param name="L">vector to light source</param>
/// <param name="V">vector to point of view</param>
/// <param name="N">normal vector</param>
/// <param name="metallic">metallic</param>
float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;

    // Treat metallicity as Fresnel reflectance at perpendicular incidence
    // The higher the metallicity, the greater the Fresnel reflection.
    float f0 = metallic;

    // Find the half vector of the vector toward the light and the vector toward the line of sight
    float3 H = normalize(L + V);

    //  Use inner product to determine how similar various vectors are
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));

    // Calculate the D term using the Beckman distribution
    float D = Beckmann(microfacet, NdotH);

    //  Compute the F term using the Schlick approximation
    float F = SpcFresnel(f0, VdotH);

    // Calculate the G term
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));

    // Calculate the m term
    float m = PI * NdotV * NdotH;

    // Find the specular reflection of the cook-trans model
    return max(F * D * G / m, 0.0);
}


/// <summary>
///Calculate diffuse reflections taking Fresnel reflections into account
/// </summary>
/// <remark>
/// This function calculates the diffuse reflectance taking into account Fresnel reflections.
/// Fresnel reflection is a phenomenon in which light is reflected off the surface of an object, with the intensity of the specular reflection.
/// Diffuse reflection, on the other hand, is light that has entered the interior of an object and caused internal confusion, resulting in diffuse reflections.
/// In other words, when the Fresnel reflection is weak, the diffuse reflection is large, and when the Fresnel reflection is strong, the diffuse reflection is small
///
////// (JP)この関数はフレネル反射を考慮した拡散反射率を計算
/// フレネル反射は、光が物体の表面で反射する現象のとこで、鏡面反射の強さになる
/// 一方拡散反射は、光が物体の内部に入って、内部錯乱を起こして、拡散して反射してきた光のこと
/// つまりフレネル反射が弱いときには、拡散反射が大きくなり、フレネル反射が強いときは、拡散反射が小さくなる
///
/// </remark>
/// <param name="N">normal</param>
/// <param name="L">the vector to light point (in the opposite direction of light)</param>
/// <param name="V">the vector to eye point </param>
/// <param name="roughness">roughness 0 to 1 range ...range: 範囲</param>
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    // Find the half vector between the vector toward the light source and the vector toward the line of sight
    float3 H = normalize(L + V);

    // Roughness fixed at 0.5... fixed at:固定する
    float roughness = 0.5f;

    float energyBias = lerp(0.0f, 0.5f, roughness);
    float energyFactor = lerp(1.0, 1.0 / 1.51, roughness);

    // Find the inner product of how similar the vector toward the light source and the half vector are
    float dotLH = saturate(dot(L, H));

    // The vector toward the light source and the half vector
    // The amount of diffuse reflection when light is incident in parallel is determined
    float Fd90 = energyBias + 2.0 * dotLH * dotLH * roughness;

    // Find the diffuse reflectance using the normal and the vector w toward the light source.
    float dotNL = saturate(dot(N, L));
    float FL = (1 + (Fd90 - 1) * pow(1 - dotNL, 5));

    // Calculate diffuse reflectance using normals and vectors toward the viewpoint
    float dotNV = saturate(dot(N, V));
    float FV = (1 + (Fd90 - 1) * pow(1 - dotNV, 5));

    // The final diffuse reflectance is obtained by multiplying the diffuse reflectance that depends on the normal and the direction to the light source by the diffuse reflectance that depends on the normal and the viewpoint vector The division by PI is to perform normalization
    //(JP) (法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
    // 乗算して最終的な拡散反射率を求めている。PIで除算しているのは正規化を行うため)
    return (FL * FV * energyFactor);
}

/*shadow rate*/
float calc_shadow_rate(int ligNo, float3 world_pos)
{
    float shadow = 0.0f;
    for (int cascadeIndex = 0; cascadeIndex < NUM_SHADOW_MAP; cascadeIndex++)
    {
        float4 posInLVP = mul(something.mlvp[ligNo][cascadeIndex], float4(world_pos, 1.0f));
        float2 shadowMapUV = posInLVP.xy / posInLVP.w;
        float zInLVP = posInLVP.z / posInLVP.w;
        shadowMapUV *= float2(0.5f, -0.5f);
        shadowMapUV += 0.5f;
        // シャドウマップUVが範囲内か判定
        if (shadowMapUV.x >= 0.0f && shadowMapUV.x <= 1.0f
            && shadowMapUV.y >= 0.0f && shadowMapUV.y <= 1.0f)
        {
            // シャドウマップから値をサンプリング
            float2 shadowValue = shadow_map.Sample(decal_sampler, shadowMapUV).xy;/* cb_shadow_map[ligNo][cascadeIndex].Sample(decal_sampler, shadowMapUV).xy;*/

            // まずこのピクセルが遮蔽されているか調べる
            if (zInLVP >= shadowValue.r + 0.001f)
            {
                shadow = 1.0f;
            }
            break;
        }
    }
    return shadow;
}
