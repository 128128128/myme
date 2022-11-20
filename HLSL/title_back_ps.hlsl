#include "title_back.hlsli"

#define PI 3.14159265359
#define TPI 6.28318530718
#define HPI 1.57079632679

float4 main(VS_OUT pin) :SV_TARGET
{
    float2 iResolution = float2(1280,720);
  float2 uv = float2(pin.texcoord.x / iResolution.x, pin.texcoord.y / iResolution.y);

    float2 p = uv - float2(.87,.5);
    float time = itime * 1.5;

    float angle = -(time - sin(time + PI) * cos(time)) - time * .95;
    float2x2 rot = float2x2(cos(angle),sin(angle),-sin(angle),cos(angle));
    p = mul(rot,p);

    float3 col = float3(0.,0.,0.);
    float L = length(p);
    float f = 0.;

    f = smoothstep(L - .005, L, .35);
    f -= smoothstep(L,L + 0.005, .27);
    //f = step(sin(L * 200. + iTime * p.x)*.5+.5,.25); // uncomment for a headache

    float t = fmod(time,TPI) - PI;
    float t1 = -PI;
    float t2 = sin(t) * (PI - .25);

    float a = atan2(p.y,p.x);
    f = f * step(a,t2) * (1. - step(a,t1));


    col = lerp(col,float3(cos(time),cos(time + TPI / 3.),cos(time + 2. * TPI / 3.)),f);

    return float4(col,1.0);
}
