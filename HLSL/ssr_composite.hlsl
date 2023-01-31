#include "pbr_common.hlsli"

cbuffer posteffect_buffer : register(b9)
{
    float bool_noise;
    float3 pad;
};
//texture
Texture2D albedo_texture : register(t0);
Texture2D normal_texture : register(t1);
Texture2D position_texture : register(t2);
Texture2D rm_texture : register(t3);
Texture2D depth_texture : register(t4);
Texture2D color_texture : register(t5);
Texture2D reflection_map : register(t6);
Texture2D composite_map:register(t7);

Texture2D env_texture : register(t15);

SamplerState ClampSampler : register(s1);

float3 bloom(float2 uv)
{
    float threshold = 0.5; //threshold(これ以下は消す)
    float ratio = (1.0 / (1.0 - threshold));

    float3 ret = 0; //start from darkness
    int num = 7; //Blur frequency
    for (int i = 0; i < num; i++) {
        float uv_x = bool_noise * 0.003 * (i + 1);
        float uv_y = bool_noise * 0.002 * (i + 1);
        float rate = (1.0 - (i * (1.0 / num)));
        // right
        float4 tex = reflection_map.Sample(ClampSampler, uv + float2(uv_x, 0));
        // Leave above threshold 閾値以上を残す
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;

        // left
        tex = reflection_map.Sample(ClampSampler, uv + float2(-uv_x, 0));
        //  Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;

        // top
        tex = reflection_map.Sample(ClampSampler, uv + float2(0, -uv_y));
        //  Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;
        // bottom
        tex = reflection_map.Sample(ClampSampler, uv + float2(0, uv_y));
        //  Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;

        // right-upper
        tex = reflection_map.Sample(ClampSampler, uv + float2(uv_x, -uv_y) * 0.7);
        //  Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;
        // left-upper
        tex = reflection_map.Sample(ClampSampler, uv + float2(-uv_x, -uv_y) * 0.7);
        // Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;
        // lower right
        tex = reflection_map.Sample(ClampSampler, uv + float2(uv_x, uv_y) * 0.7);
        // Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;
        // lower left
        tex = reflection_map.Sample(ClampSampler, uv + float2(-uv_x, uv_y) * 0.7);
        // Leave above threshold
        tex.rgb = (tex.rgb - threshold) * ratio;
        tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
        ret += tex.rgb;
    }
    // central pixel
    float4 center = reflection_map.Sample(ClampSampler, uv);
    // Leave above threshold
    float4 tex = center;
    tex.rgb = (tex.rgb - threshold) * ratio;
    tex.rgb = max(0, tex.rgb); //0以下を0に
    ret += tex.rgb;

    ret /= num * 8 + 1; //normalize 足した回数で割る(正規化)
    ret *= bool_noise;
    // simple Bloom = 元のピクセル + まぶしい光
    ret = center.rgb + ret;
    return ret;
}

//blur
float4 blur(float2 texcoord)
{
    uint mip_level = 0, width, height, number_of_levels;
    reflection_map.GetDimensions(mip_level, width, height, number_of_levels);
    const float aspect_ratio = width / height;

    //http://rastergrid.com/blog/2010/09/efficient-gaussian-blur-with-linear-sampling/
    const float offset[3] = { 0.0, 1.3846153846, 3.2307692308 };
    const float weight[3] = { 0.2270270270, 0.3162162162, 0.0702702703 };

    float4 fragment_color = reflection_map.Sample(ClampSampler, texcoord) * weight[0];
 
    for (int i = 1; i < 3; i++)
    {
        fragment_color += reflection_map.Sample(ClampSampler, texcoord + float2(0.0, offset[i] / height)) * weight[i];
        fragment_color += reflection_map.Sample(ClampSampler, texcoord - float2(0.0, offset[i] / height)) * weight[i];
        fragment_color += reflection_map.Sample(ClampSampler, texcoord + float2(offset[i] / width, 0.0)) * weight[i];
        fragment_color += reflection_map.Sample(ClampSampler, texcoord - float2(offset[i] / width, 0.0)) * weight[i];
    }

    return fragment_color;
}

float noise(float2 seed)
{
	return frac(sin(dot(seed.xy, float2(12.9898, 78.233))) * 43758.5453);
}
//SSR
// 映りこむ画面テクスチャのUVを返す
// P : 座標　R:視線の反射
float3 ssr(float3 P, float3 R, float2 uv)
{
    float step = 0.3;
    // reflect vec //反射ベクトル(視線と法線で反射)
    R = normalize(R);
    // ちょっとずつ進める
    R *= step; //0.1 = 一回で進める距離
   // float3 step = 2.0 / 100 * R;
    float maxThickness = 0.2;
    float ray = 0;
    [loop] //Loopのまま
    for (int i = 0; i < 10; i++) {
        if (bool_noise > 0.0)
            ray = (i + noise(uv + something.iTime.x));
        else
            ray = 1.0;

        P += R * ray; //進める
        // 何かにぶつかった？
        // Pまでの距離と
        float DistP = length(P - camera_constants.position);
        // レンダリング結果の距離で比較
        // Pのスクリーン座標 → UV → Depthから距離取得
        float4 screenP = mul(float4(P, 1.0), camera_constants.view_projection);
        // スクリーン座標(-1 <--> 1) → UV(0<-->1)
        float2 Puv = (screenP.xy / screenP.w) * 0.5 + 0.5;
        Puv.y = 1.0 - Puv.y; //Y反転
        // check out of screen 画面外判定
        if (Puv.x < 0) return -1;
        if (Puv.y < 0) return -1;
        if (Puv.x > 1) return -1;
        if (Puv.y > 1) return -1;

        // Depthから距離取得
        float DistS = depth_texture.Sample(
            decal_sampler, Puv).x;

        float maxLength = 2.0;
        // 距離比較(maxThickness以上近い場合はNG)
        if (DistP > DistS && DistP - DistS < maxThickness) {
            float a = 0.2 * pow(min(1.0, (maxLength / 2) / length(ray)), 2.0);
            // 交差 = 映り込みポイント
            return float3(Puv, a);
            /* float sign = -1.0;
             for (int m = 1; m <= 4; ++m) {
                 P += sign * pow(0.5, m) * step;
                 screenP = mul(camera_constants.view_projection, float4(P, 1.0));
                 Puv = screenP.xy / screenP.w * 0.5 + 0.5;
                 DistP = length(P - camera_constants.position);
                 DistS = depth_texture.Sample(
                     decal_sampler, Puv).x;
                 sign = DistP - DistS > 0 ? -1 : 1;
                 return Puv;
             }*/
        }
    }
    return -1; // non reflect 映り込み無
}


float4 applySSR(half3 normal, float3 viewPosition, half2 screenUV)
{
    float4 color = albedo_texture.Sample(decal_sampler, screenUV);

    float3 position = viewPosition;
    float2 uvNow;
    float2 uvScale = (screenUV - 0.5f) * float2(2.0f, -2.0f);

    float3 reflectVec = normalize(reflect(normalize(viewPosition), normal));// 反射ベクトル
    const int iteration = 200;  // 繰り返し数
    const int maxLength = 10;// 反射最大距離
    float3 delta = reflectVec * maxLength / iteration;// １回で進む距離
    [loop]
    for (int i = 0; i < iteration; i++)
    {
        position += delta;
        // 現在の座標を射影変換してUV値を求める
        float4 projectPosition = mul(camera_constants.projection, float4(position, 1.0));
        uvNow = projectPosition.xy / projectPosition.w * 0.5f + 0.5f;
        uvNow.y = 1.0f - uvNow.y;
        // zバッファの値を取得したらまた逆変換
        float4 pos;
        pos.z = depth_texture.Sample(decal_sampler, uvNow).r;
        pos.xy = uvScale;
        pos.w = 1.0f;
        pos = mul(camera_constants.inverse_view_projection, pos);
        float z = pos.z / pos.w;
        // Z値を比較
        [branch]
        if (position.z < z && position.z + radians(15) > z)
        {
            // 交差したので色をブレンドする
            return lerp(color, albedo_texture.Sample(decal_sampler, uvNow), radians(15));
        }
    }
    return color;
}

float4 main(PS_IN pin) : SV_Target0
{
    // get normal
      /*float4 tex = normal_texture.Sample(decal_sampler, pin.texcoord);
      float3 orgN = tex.xyz * 2.0 - 1.0;*/
      //float3 N = normalize(orgN);

      // 視線の反射
      //float4 P = position_texture.Sample(decal_sampler, pin.texcoord);
      //float3 E = P.xyz - camera_constants.position.xyz;//視線
      //float3 R = reflect(normalize(E), N);

      //return float4(R,1);
      //
      // Rを上下と方向に分解
      //float2 v;
      //v.y = R.y; //Yはそのまま
      //float3 work = R;
      //work.y = 0; //高さ無視
      //work = normalize(work); //方角のみ
      //v.x = work.x;
      //ひずみを直す
      //acos(-1)=180° acos(0)=90° acos(1)=0 
      //v.x = acos(v.x);
      //v.x /= 3.14159266; //-1=1.0 0=0.5 1=0
      //v.x = (-v.x) * 2.0 + 1.0;

      //float2 uv = v;
      //uv.y = -uv.y;
      //uv = uv * 0.5 + 0.5;
      //uv.x *= 0.5;
      //// 南向き
      //if (R.z > 0) {
      //    uv.x = 1.0 - uv.x;
      //}
      //Roughnessを考えてみる
      //float4 RM = rm_texture.Sample(decal_sampler, pin.texcoord);
      //float roughness = RM.b;
      //r 0.0  0.3   0.5  1.0 
      //l 0.0  8.0  10.0  11.0
     // float w = 1.0 - pow(1.0 - roughness, 5.0);
     // float level = w * 2.0;
      //float4 env = env_texture.SampleLevel(ClampSampler, uv, level);
      //単純なトーンマッピング
      //env.rgb = env.rgb / (100 + env.rgb);


      float4 composite = composite_map.Sample(decal_sampler, pin.texcoord);
      float4 ref = reflection_map.Sample(decal_sampler, pin.texcoord);
      ////SSR 映り込みのUVを受け取る
      //float3 l_ssr = ssr(P, R,pin.texcoord);
      //if (l_ssr.x >= 0) {
      //    //float4 texR = albedo_texture.Sample(
      //    //   decal_sampler, l_ssr);
      //    //float a = l_ssr.z;

      //    //composite += (composite* (1 - a) + texR * a) * 0.2;
      //    float4 texR = albedo_texture.Sample(
      //        decal_sampler, l_ssr);
      //    // blend based roughnessブレンド
      //    float rate = roughness;
      //    //rate = rate * 0.6 + 0.0; //0.4---1.0
      //   //env = lerp(texR, env, rate);
      //    ref = lerp(texR, 0, rate);
      //   
      //}

   
      composite = composite + blur(pin.texcoord)*0.3;/*bloom(pin.texcoord);*/
      return composite;
}

