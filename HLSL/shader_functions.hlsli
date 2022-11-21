static const float PI = 3.1415926f;         // π

//------lim light------
//N...normal
//E...camera dir Vec(normalize)
//L...light dir(normalize)
//C...light color
//limpower...limlight power
float3 Calc_lim_light(float3 N,float3 E,float3 L,float3 C,float limpower=3.0f)
{
    float rim = 1.0f - saturate(dot(N, -E));
    return C * pow(rim, limpower) * saturate(dot(L, -E));
}
//Calculate the Beckmann distribution
// Beckmann分布を計算する
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}

//Calculate Fresnel, using Schlick approximation
// フレネルを計算。Schlick近似を使用
float SpcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}


/// Calculate specular reflections for Cook-Torrance model : Cook-Torranceモデルの鏡面反射を計算
/// <"L">Vector toward light source : 光源に向かうベクトル
/// <"V">Vector toward the viewpoint : 視点に向かうベクトル
/// <"N">normal : 法線ベクトル
/// <"metallic">金属度
float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;

    //Metallicity is treated as Fresnel reflectance at perpendicular incidence
    //The higher the metallicity, the larger the Fresnel reflection.
    // 金属度を垂直入射の時のフレネル反射率として扱う
    // 金属度が高いほどフレネル反射は大きくなる
    float f0 = metallic;

    //Find the half vector of the vector toward the light and the vector toward the line of sight
    // ライトに向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L + V);

    //Use inner product to determine how similar various vectors are
    // 各種ベクトルがどれくらい似ているかを内積を利用して求める
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));

    //Calculate the D term using the Beckman distribution
    // D項をベックマン分布を用いて計算する
    float D = Beckmann(microfacet, NdotH);

    //Compute the F term using the Schlick approximation
    // F項をSchlick近似を用いて計算する
    float F = SpcFresnel(f0, VdotH);

    //Find the G term
    // G項を求める
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));

    // Find the m term
    float m = PI * NdotV * NdotH;

    //Using the values obtained up to this point, we obtain the specular reflection of the Cook-Torrance model
    // ここまで求めた、値を利用して、Cook-Torranceモデルの鏡面反射を求める
    return max(F * D * G / m, 0.0);
}


/// フレネル反射を考慮した拡散反射を計算
/// この関数はフレネル反射を考慮した拡散反射率を計算します
/// フレネル反射は、光が物体の表面で反射する現象のとこで、鏡面反射の強さになります
/// 一方拡散反射は、光が物体の内部に入って、内部錯乱を起こして、拡散して反射してきた光のこと
/// つまりフレネル反射が弱いときには、拡散反射が大きくなり、フレネル反射が強いときは、拡散反射が小さくなります
/// <param name="N">normal</param>
/// <param name="L">to light source。opposite light direction(光の方向と逆向きのベクトル) </param>
/// <param name="V">to eye vector(視線に向かうベクトル)
//float CalcDiffuseFromFresnel(float3 N, float3 L, float3 E)
//{
//    //フレネル反射を考慮した拡散反射光を求める
//
//    // 法線と光源に向かうベクトルがどれだけ似ているかを内積で求める
//    float dotNL = saturate(dot(N, L));
//
//    // 法線と視線に向かうベクトルがどれだけ似ているかを内積で求める
//    float dotNV = saturate(dot(N, E));
//
//    // 法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
//    // 乗算して最終的な拡散反射率を求めている。PIで除算しているのは正規化を行うため
//    return (dotNL * dotNV);
//}
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    //Serious implementation of Disney-based diffuse reflection with Fresnel reflection.
    // Find the half vector of vectors toward the light source and toward the line of sight
    // ディズニーベースのフレネル反射による拡散反射を真面目に実装する。
    // 光源に向かうベクトルと視線に向かうベクトルのハーフベクトルを求める
    float3 H = normalize(L + V);

    //Roughness is fixed at 0.5.
    // 粗さは0.5で固定。
    float roughness = 0.5f;

    float energy_bias = lerp(0.0f, 0.5f, roughness);
    float energy_factor = lerp(1.0, 1.0 / 1.51, roughness);

    //Find the inner product of how similar the vector toward the light source and the half vector are.
    // 光源に向かうベクトルとハーフベクトルがどれだけ似ているかを内積で求める
    float dotLH = saturate(dot(L, H));

    //The vector toward the light source and the half vector, the
    // the amount of diffuse reflection when the light is incident parallel to the light source
    // 光源に向かうベクトルとハーフベクトル、
    // 光が平行に入射したときの拡散反射量を求めている
    float Fd90 = energy_bias + 2.0 * dotLH * dotLH * roughness;

    //Find the diffuse reflectance using the normal and the vector w toward the light source.
    // 法線と光源に向かうベクトルwを利用して拡散反射率を求める
    float dotNL = saturate(dot(N, L));
    float FL = (1 + (Fd90 - 1) * pow(1 - dotNL, 5));

    //Calculate diffuse reflectance using normals and vectors toward the viewpoint
    // 法線と視点に向かうベクトルを利用して拡散反射率を求める
    float dotNV = saturate(dot(N, V));
    float FV = (1 + (Fd90 - 1) * pow(1 - dotNV, 5));

    //The diffuse reflectance, which depends on the normal and the direction to the light source, and the diffuse reflectance, which depends on the normal and the viewpoint vector, are
    //The final diffuse reflectance is obtained by multiplying the diffuse reflectance that depends on the normal and the direction to the light source by the diffuse reflectance that depends on the normal and the viewpoint vector.
    //Translated with www.DeepL.com / Translator(free version)
    // 法線と光源への方向に依存する拡散反射率と、法線と視点ベクトルに依存する拡散反射率を
    // 乗算して最終的な拡散反射率を求めている。PIで除算しているのは正規化を行うため
    return (FL * FV * energy_factor);
}
