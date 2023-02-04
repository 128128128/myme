#include "playshader.hlsli"
#include "scene_constants.hlsli"

struct PS_OUT {
    float4 Color : SV_TARGET0;
    float4 Depth : SV_TARGET1;
    float4 Normal : SV_TARGET2;
    float4 Position : SV_TARGET3;
    float4 RM : SV_TARGET4;
};

//texture for noise...ノイズ生成用テクスチャ
Texture2D noise_map : register(t0);

//sampler states
SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

#define SCALE 2.0
//#define PI radians(180.0)
#define TAU (PI*2.0)
#define CL(x,a,b) smoothstep(0.0,1.0,(2.0/3.0)*(x-a)/(b-a)+(1.0/6.0))*(b-a)+a // https://www.shadertoy.com/view/Ws3Xzr

const int NUM_STEPS = 8;
const float EPSILON = 1e-3;
#define EPSILON_NRM (0.1 / iResolution.x)
#define AA

//simple color wave
/*float4 main(VS_OUT pin) : SV_TARGET
{
    {
       const float2 dis = float2(pin.position.x / 1280,pin.position.y / 720);
       return  float4(dis, abs(sin(light_direction.iTime)), 1.0);
   }
}*/

//sprial points
/*float4 main(VS_OUT pin) : SV_TARGET
{
   {
   float2 iResolution = float2(1280,720);
   float2 m = float2((mouse.xy - 0.5 * iResolution.xy) * 2.0 / iResolution.y);
   float st = radians(-31.0); // start time
   float t = (mouse.z > 0.0) ? atan2(m.x, -m.y) : st + (light_direction.iTime * TAU) / 3600.0;
   float n = (cos(t) > 0.0) ? sin(t) : 1.0 / sin(t);
   float z = pow(500.0, n); // autozoom
   z = clamp(z, CL(z, 1e-16, 1e-15), CL(z, 1e+17, 1e+18)); // clamp to prevent black screen
   float2 uv = (pin.position.xy - 0.5 * iResolution.xy) / iResolution.y * SCALE * z;
   float ls = (light_direction.iTime * TAU) / 5.0; // light animation speed
   float a = atan2(uv.x, -uv.y); // screen arc
   float i = a / TAU; // spiral increment 0.5 per 180°
   float r = pow(length(uv), 0.5 / n) - i; // archimedean at 0.5
   float cr = ceil(r); // round up radius
   float wr = cr + i; // winding ratio
   float vd = (cr * TAU + a) / (n * 2.0); // visual denominator
   float vd2 = vd * 2.0;
   float3 col = float(cos(wr * vd + ls)); // blend it
   col *= pow(sin(frac(r) * PI), floor(abs(n * 2.0)) + 5.0); // smooth edges
   col *= sin(vd2 * wr + PI / 2.0 + ls * 2.0); // this looks nice
   col *= 0.2 + abs(cos(vd2)); // dark spirals
   float3 g = lerp(float3(0,0,0), float3(1,1,1), pow(length(uv) / z, -1.0 / n)); // dark gradient
   col = min(col, g); // blend gradient with spiral
   float3 rgb = float3(cos(vd2) + 1.0, abs(sin(t)), cos(PI + vd2) + 1.0);
   col += (col * 2.0) - (rgb * 0.5); // add color
   if (mouse.z > 0.0) // on mouse click
   {
       uv = (pin.position.xy - 0.5 * iResolution.xy) / iResolution.y;
       //col = lerp(col, float3(1,1,1), (uv * 2.0) - m, 44.0, n); // show value
   }
   return  float4(col, 1.0);
   }
}*/


/**** TWEAK *****************************************************************/
#define COVERAGE		0.4   //有効範囲//雲の分厚さ
#define THICKNESS		15.  //雲の厚さ
#define ABSORPTION		1.030725  //同化
#define WIND			float3(0, 0, -u_time * .2)   //風

#define FBM_FREQ		2.76434
#define NOISE_VALUE
//#define NOISE_WORLEY
//#define NOISE_PERLIN

//#define SIMULATE_LIGHT //cloud shadow simulate
#define FAKE_LIGHT

#define STEPS			25
/******************************************************************************/
#define PI 3.14159265359

struct ray_t {
    float3 origin;
    float3 direction;
};
#define BIAS 1e-4 // small offset to avoid self-intersections

struct sphere_t {
    float3 origin;
    float radius;
    int material;
};

struct plane_t {
    float3 direction;
    float distance;
    int material;
};

struct hit_t {
    float t;
    int material_id;
    float3 normal;
    float3 origin;
};
#define max_dist 1e8
static const hit_t  no_hit = {
    float(max_dist + 1e1), // 'infinite' distance
        -1, // material id
        float3(0., 0., 0.), // normal
        float3(0., 0., 0.) // origin
};

// ----------------------------------------------------------------------------
// Various 3D utilities functions//各種3Dユーティリティ機能
// ----------------------------------------------------------------------------

#define E 0.000001 

double my_sqrt(double a)
{
    a = a < 0 ? -a : a;
    double x = a / 2;
    while (1) {
        double e = x * x - a;
        double t = e < 0 ? -e : e;
        if (t < E) return x;
        x -= e / (x * 2);
    }
}

ray_t get_primary_ray(
    const in float3 cam_local_point,
    inout float3 cam_origin,
    inout float3 cam_look_at)
{
    //normalize
  // float3 fwd = normalize(cam_look_at - cam_origin);
    float3 fwd = cam_look_at - cam_origin;
    my_sqrt(fwd);

    float3 up = float3(0, 1, 0);
    float3 right = cross(up, fwd);
    up = cross(fwd, right);

    ray_t r = {
        cam_origin,
        normalize(fwd + up * cam_local_point.y + right * cam_local_point.x)
    };
    return r;
}

float2x2 rotate_2d(
    const in float angle_degrees
) {
    float angle = radians(angle_degrees);
    float _sin = sin(angle);
    float _cos = cos(angle);
    return float2x2(_cos, -_sin, _sin, _cos);
}

float3x3 rotate_around_z(
    const in float angle_degrees
) {
    float angle = radians(angle_degrees);
    float _sin = sin(angle);
    float _cos = cos(angle);
    return float3x3(_cos, -_sin, 0, _sin, _cos, 0, 0, 0, 1);
}

float3x3 rotate_around_y(
    const in float angle_degrees
) {
    float angle = radians(angle_degrees);
    float _sin = sin(angle);
    float _cos = cos(angle);
    return float3x3(_cos, 0, _sin, 0, 1, 0, -_sin, 0, _cos);
}

float3x3 rotate_around_x(
    const in float angle_degrees
) {
    float angle = radians(angle_degrees);
    float _sin = sin(angle);
    float _cos = cos(angle);
    return float3x3(1, 0, 0, 0, _cos, -_sin, 0, _sin, _cos);
}

float3 corect_gamma(
    const in float3 color,
    const in float gamma
) {
    float p = 1.0 / gamma;
    return float3(pow(color.r, p), pow(color.g, p), pow(color.b, p));
}

#ifdef __cplusplus
vec3 faceforward(
    _in(vec3) N,
    _in(vec3) I,
    _in(vec3) Nref
) {
    return dot(Nref, I) < 0 ? N : -N;
}
#endif

//create check board pattern
float checkboard_pattern(
    const in float2 pos,
    const in float scale
) {
    float2 pattern = floor(pos * scale);
    return fmod(pattern.x + pattern.y, 2.0);
}

float band(
    const in float start,
    const in float peak,
    const in float end,
    const in float t
) {
    return
        smoothstep(start, peak, t) *
        (1. - smoothstep(peak, end, t));
}
// ----------------------------------------------------------------------------
// Analytical surface-ray intersection routines     //計算した表面光線交差ルーチン
// ----------------------------------------------------------------------------

// geometrical solution
// info: http://www.scratchapixel.com/old/lessons/3d-basic-lessons/lesson-7-intersecting-simple-shapes/ray-sphere-intersection/
void intersect_sphere(
    const in ray_t ray,
    const in sphere_t sphere,
    inout hit_t hit
) {
    float3 rc = sphere.origin - ray.origin;
    float radius2 = sphere.radius * sphere.radius;
    float tca = dot(rc, ray.direction);
    //	if (tca < 0.) return;

    float d2 = dot(rc, rc) - tca * tca;
    if (d2 > radius2)
        return;

    float thc = sqrt(radius2 - d2);
    float t0 = tca - thc;
    float t1 = tca + thc;

    if (t0 < 0.) t0 = t1;
    if (t0 > hit.t)
        return;

    float3 impact = ray.origin + ray.direction * t0;

    hit.t = t0;
    hit.material_id = sphere.material;
    hit.origin = impact;
    hit.normal = (impact - sphere.origin) / sphere.radius;
}

// Plane is defined by normal N and distance to origin P0 (which is on the plane itself)
// a plane eq is: (P - P0) dot N = 0
// which means that any line on the plane is perpendicular to the plane normal
// a ray eq: P = O + t*D
// substitution and solving for t gives:
// t = ((P0 - O) dot N) / (N dot D)
void intersect_plane(
    const in ray_t ray,
    const in plane_t p,
    inout hit_t hit
) {
    float denom = dot(p.direction, ray.direction);
    if (denom < 1e-6) return;

    float3 P0 = float3(p.distance, p.distance, p.distance);
    float t = dot(P0 - ray.origin, p.direction) / denom;
    if (t < 0. || t > hit.t) return;

    hit.t = t;
    hit.material_id = p.material;
    hit.origin = ray.origin + ray.direction * t;
    hit.normal = faceforward(p.direction, ray.direction, p.direction);
}

// ----------------------------------------------------------------------------
// Noise function by iq from https://www.shadertoy.com/view/4sfGzS
// ----------------------------------------------------------------------------

float hash(
    const in float n
) {
    return frac(sin(n) * 753.5453123);
}

float noise_iq(
    const in float3 x
) {
    float3 p = floor(x);
    float3 f = frac(x);
    f = f * f * (3.0 - 2.0 * f);

#if 1
    float n = p.x + p.y * 157.0 + 113.0 * p.z;
    return lerp(lerp(lerp(hash(n + 0.0), hash(n + 1.0), f.x),
        lerp(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
        lerp(lerp(hash(n + 113.0), hash(n + 114.0), f.x),
            lerp(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
#else
    float2 uv = (p.xy + float2(37.0, 17.0) * p.z) + f.xy;
    float2 rg = noise_map.Sample(linear_sampler_state, (uv + 0.5) / 256.0, 0);
    return lerp(rg.x, rg.y, f.z);
#endif
}
// ----------------------------------------------------------------------------
// Noise function by iq from https://www.shadertoy.com/view/ldl3Dl
// ----------------------------------------------------------------------------

float3 hash_w(
    const in float3 x
) {
#if 1
    float3 xx = float3(dot(x, float3(127.1, 311.7, 74.7)),
        dot(x, float3(269.5, 183.3, 246.1)),
        dot(x, float3(113.5, 271.9, 124.6)));

    return frac(sin(xx) * 43758.5453123);
#else
    //use texture
    float2 v = (x.xy + float2(3.0, 1.0) * x.z + 0.5) / 64;
    float4 tex = noise_map.Sample(point_sampler_state, v, -100);
    return tex.xyz;
#endif
}

// returns closest, second closest, and cell id
float3 noise_w(
    const in float3 x
) {
    float3 p = floor(x);
    float3 f = frac(x);

    float id = 0.0;
    float2 res = float2(100.0, 100.0);
    [loop]
    for (int k = -1; k <= 1; k++)
        for (int j = -1; j <= 1; j++)
            for (int i = -1; i <= 1; i++)
            {
                float3 b = float3(float(i), float(j), float(k));
                float3 r = float3(b)-f + hash_w(p + b);
                float d = dot(r, r);

                if (d < res.x)
                {
                    id = dot(p + b, float3(1.0, 57.0, 113.0));
                    res = float2(d, res.x);
                }
                else if (d < res.y)
                {
                    res.y = d;
                }
            }

    return float3(sqrt(res), abs(id));
}

float3 mod289(float3 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 mod289(float4 x)
{
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

float4 permute(float4 x)
{
    return mod289(((x * 34.0) + 1.0) * x);
}

float4 taylorInv_sqrt(float4 r)
{
    return 1.79284291400159 - 0.85373472095314 * r;
}

float3 fade(float3 t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

// Classic Perlin noise
float cnoise(float3 P)
{
    float3 Pi0 = floor(P); // Integer part for indexing
    float3 Pi1 = Pi0 + float3(1, 1, 1); // Integer part + 1
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    float3 Pf0 = frac(P); // Fractional part for interpolation
    float3 Pf1 = Pf0 - float3(1, 1, 1); // Fractional part - 1.0
    float4 ix = float4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    float4 iy = float4(Pi0.yy, Pi1.yy);
    float4 iz0 = Pi0.zzzz;
    float4 iz1 = Pi1.zzzz;

    float4 ixy = permute(permute(ix) + iy);
    float4 ixy0 = permute(ixy + iz0);
    float4 ixy1 = permute(ixy + iz1);

    float4 gx0 = ixy0 * (1.0 / 7.0);
    float4 gy0 = frac(floor(gx0) * (1.0 / 7.0)) - 0.5;
    gx0 = frac(gx0);
    float4 gz0 = float4(.5, .5, .5, .5) - abs(gx0) - abs(gy0);
    float4 sz0 = step(gz0, float4(0, 0, 0, 0));
    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

    float4 gx1 = ixy1 * (1.0 / 7.0);
    float4 gy1 = frac(floor(gx1) * (1.0 / 7.0)) - 0.5;
    gx1 = frac(gx1);
    float4 gz1 = float4(.5, .5, .5, .5) - abs(gx1) - abs(gy1);
    float4 sz1 = step(gz1, float4(0, 0, 0, 0));
    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
    gy1 -= sz1 * (step(0.0, gy1) - 0.5);

    float3 g000 = float3(gx0.x, gy0.x, gz0.x);
    float3 g100 = float3(gx0.y, gy0.y, gz0.y);
    float3 g010 = float3(gx0.z, gy0.z, gz0.z);
    float3 g110 = float3(gx0.w, gy0.w, gz0.w);
    float3 g001 = float3(gx1.x, gy1.x, gz1.x);
    float3 g101 = float3(gx1.y, gy1.y, gz1.y);
    float3 g011 = float3(gx1.z, gy1.z, gz1.z);
    float3 g111 = float3(gx1.w, gy1.w, gz1.w);

    float4 norm0 = taylorInv_sqrt(float4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    float4 norm1 = taylorInv_sqrt(float4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;

    float n000 = dot(g000, Pf0);
    float n100 = dot(g100, float3(Pf1.x, Pf0.yz));
    float n010 = dot(g010, float3(Pf0.x, Pf1.y, Pf0.z));
    float n110 = dot(g110, float3(Pf1.xy, Pf0.z));
    float n001 = dot(g001, float3(Pf0.xy, Pf1.z));
    float n101 = dot(g101, float3(Pf1.x, Pf0.y, Pf1.z));
    float n011 = dot(g011, float3(Pf0.x, Pf1.yz));
    float n111 = dot(g111, Pf1);

    float3 fade_xyz = fade(Pf0);
    float4 n_z = lerp(float4(n000, n100, n010, n110), float4(n001, n101, n011, n111), fade_xyz.z);
    float2 n_yz = lerp(n_z.xy, n_z.zw, fade_xyz.y);
    float n_xyz = lerp(n_yz.x, n_yz.y, fade_xyz.x);
    return 2.2 * n_xyz;
}

// Classic Perlin noise, periodic variant
float pnoise(float3 P, float3 rep)
{
    float3 Pi0 = fmod(floor(P), rep); // Integer part, modulo period
    float3 Pi1 = fmod(Pi0 + float3(1, 1, 1), rep); // Integer part + 1, mod period
    Pi0 = mod289(Pi0);
    Pi1 = mod289(Pi1);
    float3 Pf0 = frac(P); // Fractional part for interpolation
    float3 Pf1 = Pf0 - float3(1, 1, 1); // Fractional part - 1.0
    float4 ix = float4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
    float4 iy = float4(Pi0.yy, Pi1.yy);
    float4 iz0 = Pi0.zzzz;
    float4 iz1 = Pi1.zzzz;

    float4 ixy = permute(permute(ix) + iy);
    float4 ixy0 = permute(ixy + iz0);
    float4 ixy1 = permute(ixy + iz1);

    float4 gx0 = ixy0 * (1.0 / 7.0);
    float4 gy0 = frac(floor(gx0) * (1.0 / 7.0)) - 0.5;
    gx0 = frac(gx0);
    float4 gz0 = float4(.5, .5, .5, .5) - abs(gx0) - abs(gy0);
    float4 sz0 = step(gz0, float4(0, 0, 0, 0));
    gx0 -= sz0 * (step(0.0, gx0) - 0.5);
    gy0 -= sz0 * (step(0.0, gy0) - 0.5);

    float4 gx1 = ixy1 * (1.0 / 7.0);
    float4 gy1 = frac(floor(gx1) * (1.0 / 7.0)) - 0.5;
    gx1 = frac(gx1);
    float4 gz1 = float4(.5, .5, .5, .5) - abs(gx1) - abs(gy1);
    float4 sz1 = step(gz1, float4(0, 0, 0, 0));
    gx1 -= sz1 * (step(0.0, gx1) - 0.5);
    gy1 -= sz1 * (step(0.0, gy1) - 0.5);

    float3 g000 = float3(gx0.x, gy0.x, gz0.x);
    float3 g100 = float3(gx0.y, gy0.y, gz0.y);
    float3 g010 = float3(gx0.z, gy0.z, gz0.z);
    float3 g110 = float3(gx0.w, gy0.w, gz0.w);
    float3 g001 = float3(gx1.x, gy1.x, gz1.x);
    float3 g101 = float3(gx1.y, gy1.y, gz1.y);
    float3 g011 = float3(gx1.z, gy1.z, gz1.z);
    float3 g111 = float3(gx1.w, gy1.w, gz1.w);

    float4 norm0 = taylorInv_sqrt(float4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
    g000 *= norm0.x;
    g010 *= norm0.y;
    g100 *= norm0.z;
    g110 *= norm0.w;
    float4 norm1 = taylorInv_sqrt(float4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
    g001 *= norm1.x;
    g011 *= norm1.y;
    g101 *= norm1.z;
    g111 *= norm1.w;

    float n000 = dot(g000, Pf0);
    float n100 = dot(g100, float3(Pf1.x, Pf0.yz));
    float n010 = dot(g010, float3(Pf0.x, Pf1.y, Pf0.z));
    float n110 = dot(g110, float3(Pf1.xy, Pf0.z));
    float n001 = dot(g001, float3(Pf0.xy, Pf1.z));
    float n101 = dot(g101, float3(Pf1.x, Pf0.y, Pf1.z));
    float n011 = dot(g011, float3(Pf0.x, Pf1.yz));
    float n111 = dot(g111, Pf1);

    float3 fade_xyz = fade(Pf0);
    float4 n_z = lerp(float4(n000, n100, n010, n110), float4(n001, n101, n011, n111), fade_xyz.z);
    float2 n_yz = lerp(n_z.xy, n_z.zw, fade_xyz.y);
    float n_xyz = lerp(n_yz.x, n_yz.y, fade_xyz.x);
    return 2.2 * n_xyz;
}

#ifdef NOISE_VALUE
#define noise(x) noise_iq(x)
#endif
#ifdef NOISE_WORLEY
#define noise(x) (1. - noise_w(x).r)
//#define noise(x) abs( noise_iq(x / 8.) - (1. - (noise_w(x * 2.).r)))
#endif
#ifdef NOISE_PERLIN
#define noise(x) abs(cnoise(x))
#endif
// ----------------------------------------------------------------------------
// Fractal Brownian Motion//非整数ブラウン運動
// //https://ja.wikipedia.org/wiki/%E9%9D%9E%E6%95%B4%E6%95%B0%E3%83%96%E3%83%A9%E3%82%A6%E3%83%B3%E9%81%8B%E5%8B%95
// https://jp.mathworks.com/help/wavelet/gs/one-dimensional-fractional-brownian-motion-synthesis.html
// //https://thebookofshaders.com/13/?lan=jp
// ----------------------------------------------------------------------------

float fbm(
    const in float3 pos,
    const in float lacunarity//周波数を一定の割合で増加させる値
) {
    float3 p = pos;
    float
        t = 0.51749673 * noise(p); p *= lacunarity;
    t += 0.25584929 * noise(p); p *= lacunarity;
    t += 0.12527603 * noise(p); p *= lacunarity;
    t += 0.06255931 * noise(p);

    return t;
}

float get_noise(const in float3 x)// get noise
{
    return fbm(x, FBM_FREQ);
}

static const float3 sun_color = float3(1., .7, .55);
static const float3 moon_color = float3(.9, .9, .9);

//atmosphere..大気圏
static const sphere_t atmosphere = {
float3(0, -450, 0), 500., 0
};
static const sphere_t atmosphere_2 = {
atmosphere.origin, atmosphere.radius + 50., 0
};
static const plane_t ground = {
float3(0., -1., 0.), 0., 1
};

float star_rand(float2 co)
{
    return frac(sin(dot(co.xy, float2(12.9898, 78.233))) * 43758.5453);
}

//sky color...空の描画
float3 render_sky_color(
    const in ray_t eye,float2 texcoord
) {
    float3 rd = eye.direction;
   

    //blue sky
    if(sky_state<=0)
    {
        //sun speed setting
        float3 sun_dir = normalize(float3(0, abs(sin(something.iTime * .05)), -1));
        float sun_amount = max(dot(rd, sun_dir), 0.0);
        //sky gradation//top color & under color
        float3  sky_color = lerp(top_sky_color.rgb, under_sky_color.rgb, 1.0 - rd.y);
        sky_color = sky_color + sun_color * min(pow(sun_amount, 1500.0) * 5.0, 1.0);
        sky_color = sky_color + sun_color * min(pow(sun_amount, 10.0) * .6, 1.0);

        return sky_color;


    }

    //star night
    else if(sky_state>=0&&sky_state<=1)
    {
        float3 moon_dir = normalize(float3(0, abs(sin(something.iTime * .05)), -1));
        float moon_amount = max(dot(rd, moon_dir), 0.0);

        float size = 30.0;
        float prob = 0.95;

        float2 pos = floor(1.0 / size * texcoord.xy);

        float color = 0.0;
        float star_value = star_rand(pos);
        float2 iResolution = float2(1280, 720);

        if (star_value > prob)
        {
            float2 center = size * pos + float2(size, size) * 0.5;

            float t = 0.9 + 0.2 * sin(something.iTime + (star_value - prob) / (1.0 - prob) * 45.0);

            color = 1.0 - distance(texcoord.xy, center) / (0.5 * size);
            color = color * t / (abs(texcoord.y - center.y)) * t / (abs(texcoord.x - center.x));
        }
        else if (star_rand(texcoord.xy / iResolution.xy) > 0.996)
        {
            float r = star_rand(texcoord.xy);
            color = r * (sin(something.iTime * (r * 5.0) + 720.0 * r) + 0.75);
        }

        float3  sky_color = float3(.1, .1, .4);
        sky_color += color;
        sky_color = sky_color + moon_color * min(pow(moon_amount, 500.0) * 5.0, 0.7);

        return sky_color;
    }
    //sunset sky
    else
    {
        //sun speed setting
        float3 sun_dir = normalize(float3(0, 0, -1));
        float sun_amount = max(dot(rd, sun_dir), 0.0);
        //sky gradation//top color & under color
        float3  sky_color = lerp(float3(.09, .09, .4), float3(1., .5, .0), 1.0 - rd.y);
        sky_color = sky_color + sun_color * min(pow(sun_amount, 1500.0) * 5.0, 1.0);
        sky_color = sky_color + sun_color * min(pow(sun_amount, 10.0) * .6, 1.0);

        return sky_color;
    }
}

//density...密度計算
float density(
    const in float3 pos,
    const in float3 offset,
    const in float t
) {
    // signal
    float3 p = pos * .0212242 + offset;
    float dens = get_noise(p);

    float cov = 1. - coverage;
    //dens = band (.1, .3, .6, dens);
    //dens *= step(cov, dens);
    //dens -= cov;
    dens *= smoothstep(cov, cov + .05, dens);

    return clamp(dens, 0., 1.);
}

float light(
    const in float3 origin
) {
    const int steps = 1;
    float march_step = 1.;

    float3 pos = origin;

    float3 dir_step = normalize(float3(0, abs(sin(something.iTime * .000001)), -1)) * march_step;
    float T = 1.; // transmitance

    for (int i = 0; i < steps; i++) {
        float dens = density(pos, float3(0, 0, -something.iTime * .2), 0.);

        float T_i = exp(-ABSORPTION * dens * march_step);
        T *= T_i;
        //if (T < .01) break;

        pos += dir_step;
    }

    return T;
}

//renderer cloud 雲描画
float4 render_clouds(
    const in ray_t eye
) {
    hit_t hit = no_hit;
    intersect_sphere(eye, atmosphere/*大気圏*/, hit);
    //hit_t hit_2 = no_hit;
    //intersect_sphere(eye, atmosphere_2, hit_2);

    const float thickness = THICKNESS; // length(hit_2.origin - hit.origin);//thickness...濃度
    //const float r = 1. - ((atmosphere_2.radius - atmosphere.radius) / thickness);
    const int steps = STEPS; // +int(32. * r);
    float march_step = thickness / float(steps);

    float3 dir_step = eye.direction / eye.direction.y * march_step;
    float3 pos = //eye.origin + eye.direction * 100.; 
        hit.origin;

    float T = 1.0; // transmitance..透過率 //rainy cloud...about 0.6...something..? 感覚(個人的に)

    if (sky_state >= 2)
        T = 0.6;

    float3 C = float3(0, 0, 0); // color
    float alpha = 0.;

    for (int i = 0; i < steps; i++) {
        float h = float(i) / float(steps);
        //cloud speed & density setting
        float dens = density(pos, float3(0, 0, -something.iTime * cloud_speed), h);

        float T_i = exp(-ABSORPTION * dens * march_step);
        T *= T_i;
        if (T < .01) break;

        C += T *
#ifdef SIMULATE_LIGHT
            light(pos) *
#endif
#ifdef FAKE_LIGHT
            (exp(h) / 1.75) *
#endif
            dens * march_step;
        alpha += (1. - T_i) * (1. - alpha);

        pos += dir_step;
        if (length(pos) > 1e3) break;
    }

    return float4(C, alpha);
}

///---------------
///rain
///---------------
float rain_random(in float2 uv)
{
    return frac(sin(dot(uv.xy,
        float2(12.9898, 78.233))) *
        43758.5453123);
}

float rain_noise(in float2 uv)
{
    float2 i = floor(uv);
    float2 f = frac(uv);
    f = f * f * (3. - 2. * f);

    float lb = rain_random(i + float2(0., 0.));
    float rb = rain_random(i + float2(1., 0.));
    float lt = rain_random(i + float2(0., 1.));
    float rt = rain_random(i + float2(1., 1.));

    return lerp(lerp(lb, rb, f.x),
        lerp(lt, rt, f.x), f.y);
}

float rain(float2 uv)
{
    float travelTime = (something.iTime * 0.2) + 0.1;

    float2 tiling = float2(1., .01);
    float2 offset = float2(travelTime * 0.5 + uv.x * 0.2, travelTime * 0.2);

    float2 st = uv * tiling + offset;

    float rain = 0.1;
    float f = rain_noise(st * 200.5) * rain_noise(st * 125.5);
    f = clamp(pow(abs(f), 15.0) * 1.5 * (rain * rain * 125.0), 0.0, 0.25);
    return f;
}

PS_OUT main(VS_OUT pin)
{
    PS_OUT ret;


    //screen size
    float2 iResolution = float2(1280, 720);
    //x:aspect ratio y:1
    float2 aspect_ratio = float2(iResolution.x / iResolution.y, 1);

    //field of view..視野角
    float fov = tan(radians(45.0));
    //position in ndc space
    float2 point_ndc = pin.position.xy / iResolution.xy;
    //0.0~1.0
    point_ndc.y = 1. - point_ndc.y;

    float3 point_cam = float3((2.0 * point_ndc - 1.0) * aspect_ratio * fov, -1.0);//point camera


    //float n = saturate(get_noise(point_cam));
    //float4 color = float4(float3(n, n, n), 1);
    //return;


    float3 col = float3(0, 0, 0);

    //float3x3 rot = rotate_around_x(abs(sin(light_direction.iTime / 2.)) * 45.);
    //sun_dir = mul(rot, sun_dir);

    float3 eye = float3(0, 1., 0);
    float3 look_at = float3(0, 1.6, -1);
    ray_t eye_ray = get_primary_ray(point_cam, eye, look_at);

    if (mouse.w > 0) {
        eye_ray.direction.yz = mul(rotate_2d(+mouse.y * .13), eye_ray.direction.yz);
        eye_ray.direction.xz = mul(rotate_2d(-mouse.x * .39), eye_ray.direction.xz);
   }
    //hit_t hit = no_hit;
    //white & black ground insert//for test
    //intersect_plane(eye_ray, ground, hit);

    ///--------if use check borad material-----///
    /*if (hit.material_id == 1) {
        float cb = checkboard_pattern(hit.origin.xz, .5);
        col = lerp(float3(.6, .6, .6), float3(.75, .75, .75), cb);
    }
    else {*/

        float3 sky = render_sky_color(eye_ray,pin.texcoord);
        float4 cld = render_clouds(eye_ray);
        //cld.rgb...cloud.rbg, cld.a...alpha
    if(sky_state<=0)//blue
        col = lerp(sky, cld.rgb / (0.000001 + cld.a),cld.a);
    else if(sky_state<=1)//night
        col = lerp(sky, cld.rgb / (0.000001 + cld.a),0.3);///cld.alpha is 0.3 in star night maybe
    else
        col = lerp(sky, cld.rgb / (0.000001 + cld.a), cld.a);


        ///--------if use check borad material-----///
        /* intersect_sphere(eye_ray, atmosphere, hit);
        float3 d = hit.normal;
        float u = .5 + atan2(d.z, d.x) / (2. * PI);
        float v = .5 - asin(d.y) / PI;
        float cb = checkboard_pattern(float2(u, v), 50.);
        col = float3(cb, cb, cb);*/
        //}


        if (boolrain>.0) {
            float rainy = rain(point_ndc);

            col += float3(1.0, 1.0, 1.0) * rainy;
        }
        
        ret.Color = float4(col, 1.0);
        ret.Depth = float4(0.0, 0.0, 0.0, 0.0);
        ret.RM = float4(0.0, 0.0, 1.0, 1.0);
        ret.Position = float4(0.0, 0.0, 0.0, 0.0);
        ret.Normal = float4(0.0, 0.0, 0.0, 0.0);
        return ret;
}