#include "static_mesh.hlsli"
#include "scene_constants.hlsli"
#include "water_fall.hlsli"

Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);
Texture2D shader_map : register(t4);
Texture2D noise_map_1 : register(t5);
Texture2D noise_map_2 : register(t6);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

//float4 main(VS_OUT pin) : SV_TARGET0
//{
//	float4 color = diffuse_map.Sample(anisotropic_sampler_state, pin.texcoord);
//	
//
//	float4 temp_cast_0 = (1.0).xxxx;
//	float2 appendResult15 = float2(1, offset);
//	float2 uv_TexCoord12 = pin.texcoord * appendResult15 + float2(0, 0);
//	
//	float2 panner7 = (uv_TexCoord12 + (light_direction.iTime * -speed) * float2(0, 1));
//	float4 shader_map_color = shader_map.Sample(anisotropic_sampler_state, panner7);
//	
//	float4 lerpResult9 = lerp(color, temp_cast_0, shader_map_color.r);
//	float4 shader_map_23 = shader_map.Sample(anisotropic_sampler_state, pin.texcoord);
//
//	float4 clampResult24 = clamp(lerpResult9 + shader_map_23.g, float4(0, 0, 0, 0), float4(1, 1, 1, 0));
//	float4 final_color;
//    final_color.rgb = clampResult24.rgb;
//    final_color.a = 1;
//
//	return final_color;
//}


// https://www.shadertoy.com/view/XdS3RW
float3 screen_blend(float3 s, float3 d) {
    return s + d - s * d;
}

// https://catlikecoding.com/unity/tutorials/flow/texture-distortion/
float2 flow(float2 uv, float2 flowmap, float phase, float t, out float weight) {
    float progress = frac(t + phase);
    float2 displacement = flowmap * progress;
    weight = 1. - abs(progress * 2. - 1.);
    return uv + displacement;
}

// https://thebookofshaders.com/13/
float fbm(Texture2D texture_2d, float2 p) {
    // Initial values
    float value = 0.;
    float amplitude = .5;
    float frequency = 1.;

    float col_r = 0;
    // Loop of octaves
    for (int i = 0; i < 5; i++) {
        p.x = p.x * frequency;
        p.y = p.y * frequency;
        col_r = texture_2d.Sample(anisotropic_sampler_state, p).r;
        value += amplitude * col_r;
        frequency *= 2.;
        amplitude *= .5;
    }
    return value;
}

float4 main(VS_OUT pin) : SV_TARGET0
{
    float2 uv=pin.texcoord;

    float2 flowuv = uv * .03 + float2(0., light_direction.iTime * -0.05);//speed
    float noise = fbm(noise_map_1, flowuv);
    float2 flowmap = float2(0., smoothstep(0.2, 1., noise)) * .6;
    float weightA, weightB,weightC,weightD;
    float t = light_direction.iTime * .8;
    float2 uvA = flow(flowuv, flowmap, 0.0, t, weightA);
    float2 uvB = flow(flowuv, flowmap, 0.5, t, weightB);
    float2 uvC = flow(flowuv, flowmap, 0.0, t, weightC);
    float2 uvD = flow(flowuv, flowmap, 0.5, t, weightD);
    float flowA = fbm(noise_map_1, uvA) * weightA;
    float flowB = fbm(noise_map_1, uvB) * weightB;
    float flowC = fbm(noise_map_1, uvC) * weightC;
    float flowD = fbm(noise_map_1, uvD) * weightD;
    float flow = (flowA + flowB+flowC+flowD);

    float4 blue = float4(0.6, 0.6, 0.9,0.2);
    float4 waterfall = (1. - flow) * blue + smoothstep(0., 1., flow);
  
    float4 final_color = waterfall;//float4(waterfall, 1.0);
    return final_color;
}


//#define PI 3.14159265359
//
//// alternate the direction of rotation along a checker pattern
//#define CHECKERED

//float2x2 rot(float angle) {
//    return float2x2(cos(angle), -sin(angle),
//        sin(angle), cos(angle));
//}
//float checkersign(float2 uv) {
//#ifdef CHECKERED
//    uv = floor(uv);
//    return sign(fmod(uv.x + uv.y, 2.) - .5);
//#else
//    return 1.;
//#endif
//}
//float3 mod289(float3 x) {
//    return x - floor(x * (1.0 / 289.0)) * 289.0;
//}
//float4 mod289(float4 x) {
//    return x - floor(x * (1.0 / 289.0)) * 289.0;
//}
//float4 permute(float4 x) {
//    return mod289(((x * 34.0) + 1.0) * x);
//}
//float4 taylorInvSqrt(float4 r)
//{
//    return 1.79284291400159 - 0.85373472095314 * r;
//}
//// https://github.com/ashima/webgl-noise/blob/master/src/noise3Dgrad.glsl
//// modified to allow for rotation 
//float snoise(float3 v, out float3 gradient, float time)
//{
//    const float2  C = float2(1.0 / 6.0, 1.0 / 3.0);
//    const float4  D = float4(0.0, 0.5, 1.0, 2.0);
//
//    // First corner
//    float3 i = floor(v + dot(v, C.yyy));
//    float3 x0 = v - i + dot(i, C.xxx);
//
//    // Other corners
//    float3 g = step(x0.yzx, x0.xyz);
//    float3 l = 1.0 - g;
//    float3 i1 = min(g.xyz, l.zxy);
//    float3 i2 = max(g.xyz, l.zxy);
//
//    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
//    //   x1 = x0 - i1  + 1.0 * C.xxx;
//    //   x2 = x0 - i2  + 2.0 * C.xxx;
//    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
//    float3 x1 = x0 - i1 + C.xxx;
//    float3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
//    float3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y
//
//  // Permutations
//    i = mod289(i);
//    float4 p = permute(permute(permute(
//        i.z + float4(0.0, i1.z, i2.z, 1.0))
//        + i.y + float4(0.0, i1.y, i2.y, 1.0))
//        + i.x + float4(0.0, i1.x, i2.x, 1.0));
//
//    // Gradients: 7x7 points over a square, mapped onto an octahedron.
//    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
//    float n_ = 0.142857142857; // 1.0/7.0
//    float3  ns = n_ * D.wyz - D.xzx;
//
//    float4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)
//
//    float4 x_ = floor(j * ns.z);
//    float4 y_ = floor(j - 7.0 * x_);    // mod(j,N)
//
//    float4 x = x_ * ns.x + ns.yyyy;
//    float4 y = y_ * ns.x + ns.yyyy;
//    float4 h = 1.0 - abs(x) - abs(y);
//
//    float4 b0 = float4(x.xy, y.xy);
//    float4 b1 = float4(x.zw, y.zw);
//
//    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
//    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
//    float4 s0 = floor(b0) * 2.0 + 1.0;
//    float4 s1 = floor(b1) * 2.0 + 1.0;
//    float4 sh = -step(h, float4(0.0,0.0,0.0,0.0));
//
//    float4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
//    float4 a1 = b1.xzyw + s1.xzyw * sh.zzww;
//
//    float3 p0 = float3(a0.xy, h.x);
//    float3 p1 = float3(a0.zw, h.y);
//    float3 p2 = float3(a1.xy, h.z);
//    float3 p3 = float3(a1.zw, h.w);
//
//    //Normalise gradients
//    float4 norm = taylorInvSqrt(float4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
//    p0 *= norm.x;
//    p1 *= norm.y;
//    p2 *= norm.z;
//    p3 *= norm.w;
//
//    // add rotation
//    x0.xy =mul(x0, rot(time * checkersign(a0.xy)));
//    x1.xy =mul(x1.xy, rot(time * checkersign(a0.zw)));
//    x2.xy =mul(x2.xy, rot(time * checkersign(a1.xy)));
//    x3.xy =mul(x3.xy, rot(time * checkersign(a1.zw)));
//
//    // Mix final noise value
//    float4 m = max(0.6 - float4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
//    float4 m2 = m * m;
//    float4 m4 = m2 * m2;
//    float4 pdotx = float4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3));
//
//    // Determine noise gradient
//    float4 temp = m2 * m * pdotx;
//    gradient = -8.0 * (temp.x * x0 + temp.y * x1 + temp.z * x2 + temp.w * x3);
//    gradient += m4.x * p0 + m4.y * p1 + m4.z * p2 + m4.w * p3;
//    gradient *= 42.0;
//
//    return 42.0 * dot(m4, pdotx);
//}
//float fbm(float3 p, inout float3 gradient) {
//    // Initial values
//    float value = 0.;
//    float amplitude = .3;
//    float frequency = 1.;
//    float rotation = 1.5;
//    float3 grad;
//    // Loop of octaves
//    for (int i = 0; i < 4; i++) {
//
//        value += amplitude * snoise(frequency * p - gradient, grad, light_direction.iTime * rotation);
//        grad.z = 0.;
//        gradient += amplitude * grad * .3;
//        frequency *= 2.;
//        amplitude *= .5;
//        rotation *= 2.;
//    }
//    return value;
//}
//float4 main(VS_OUT pin):SV_TARGET0
//{
//    
//    float2 uv = pin.texcoord;
// 
//    float3 p = float3(uv, 0.);
//    //p.x *= pow(p.y, .3);
//    //p.y = pow(p.y, .5);
//
//    float3 gradient = float3(0.,0.,0.);
//    float noise = fbm(p + float3(0., light_direction.iTime * .6, 0.), gradient);
//    noise = noise * .5 + .5;
//
//    float3 f;
//    f.x = noise * 0.6* 2.;
//    f.y = noise * 0.6* 2.;
//    f.z = noise * 0.9* 2.;
//    float3 col = float3(f);
//  
//   return float4(col, 1.0);
//}