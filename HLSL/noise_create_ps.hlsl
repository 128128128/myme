#include "noise_create.hlsli"

float hash(float2 p) 
{
	p = 50.0 * frac(p * 0.3183099 + float2(0.71, 0.113));
	return -1.0 + 2.0 * frac(p.x * p.y * (p.x + p.y));
}

float noise(in float2 p)
{
	float2 i = floor(p);
	float2 f = frac(p);

	float2 u = f * f * (3.0 - 2.0 * f);

	return lerp(lerp(hash(i + float2(0.0, 0.0)),
		hash(i + float2(1.0, 0.0)), u.x),
		lerp(hash(i + float2(0.0, 1.0)),
			hash(i + float2(1.0, 1.0)), u.x), u.y);
}

float r(float n)
{
    return frac(cos(n * 89.42) * 343.42);
}
float2 r(float2 n)
{
    return float2(r(n.x * 23.62 - 300.0 + n.y * 34.35), r(n.x * 45.13 + 256.0 + n.y * 38.89));
}
float worley(float2 n, float s)
{
    float dis = 2.0;
    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float2 p = floor(n / s) + float2(x, y);
            float d = length(r(p) + float2(x, y) - frac(n / s));
            if (dis > d)
            {
                dis = d;
            }
        }
    }
    return 1.0 - dis;

}

#define MOD3 float3(.1031,.11369,.13787)

float3 hash33(float3 p3)
{
    p3 = frac(p3 * MOD3);
    p3 += dot(p3, p3.yxz + 19.19);
    return -1.0 + 2.0 * frac(float3((p3.x + p3.y) * p3.z, (p3.x + p3.z) * p3.y, (p3.y + p3.z) * p3.x));
}
float perlin_noise(float3 p)
{
    float3 pi = floor(p);
    float3 pf = p - pi;

    float3 w = pf * pf * (3.0 - 2.0 * pf);

    return 	lerp(
        lerp(
            lerp(dot(pf - float3(0, 0, 0), hash33(pi + float3(0, 0, 0))),
                dot(pf - float3(1, 0, 0), hash33(pi + float3(1, 0, 0))),
                w.x),
            lerp(dot(pf - float3(0, 0, 1), hash33(pi + float3(0, 0, 1))),
                dot(pf - float3(1, 0, 1), hash33(pi + float3(1, 0, 1))),
                w.x),
            w.z),
        lerp(
            lerp(dot(pf - float3(0, 1, 0), hash33(pi + float3(0, 1, 0))),
                dot(pf - float3(1, 1, 0), hash33(pi + float3(1, 1, 0))),
                w.x),
            lerp(dot(pf - float3(0, 1, 1), hash33(pi + float3(0, 1, 1))),
                dot(pf - float3(1, 1, 1), hash33(pi + float3(1, 1, 1))),
                w.x),
            w.z),
        w.y);
}


// -----------------------------------------------

float4 main(VS_OUT pin) : SV_TARGET
{
	float2 iResolution = float2(1280.0,720.0);

	float2 p = pin.position.xy / iResolution.xy;

	float2 uv = p * float2(iResolution.x / iResolution.y, 1.0);

	float f = 0.0;


	//fbm - fractal noise (octaves)
	if(cb_kind_of_noise ==0)
	{
		uv *= 8.0;
		float amplitude = 1.0;
        float2 m0 = cb_mat0;
        float2 m1 = cb_mat1;
		float2x2 m = float2x2(m0, m1);
		amplitude *= cb_amplitude;
		for (int i = 0; i < cb_octaves; i++) {
			f += amplitude * noise(uv);
		    uv = mul(m, uv);
		    amplitude *= cb_amplitude;
		}
	}
	// value noise	
	else if (cb_kind_of_noise == 1)
	{
		f = noise(cb_param * uv);
	}

	f = 0.5 + 0.5 * f;

    if(cb_kind_of_noise ==2)
    {
        f = (1.0 + perlin_noise(float3(pin.position.xy / iResolution.xy, 0.0) * 8.0))
            * (1.0 + (worley(pin.position.xy, cb_param) +
                0.5 * worley(2.0 * pin.position.xy, cb_param) +
                0.25 * worley(4.0 * pin.position.xy, cb_param)));
        return float4(f / 4.0, f / 4.0, f / 4.0, 1.0);
    }

	return float4(f, f, f, 1.0);
}
