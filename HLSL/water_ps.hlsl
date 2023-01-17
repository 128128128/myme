#include "static_mesh.hlsli"
#include "scene_constants.hlsli"
#include "render_constants.hlsli"

Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);

//sky texture
Texture2D sky_color:register(t8);
Texture2D seaTexture:register(t9);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

// MRT information
struct PS_OUT {
	float4 Color : SV_TARGET0;
	float4 Depth : SV_TARGET1;
	float4 Normal : SV_TARGET2;
	float4 Position : SV_TARGET3;
	float4 RM : SV_TARGET4;
};

// sea
#define ITER_GEOMETRY  3
#define ITER_FRAGMENT  5
#define SEA_HEIGHT  0.6
#define SEA_CHOPPY  4.0
#define  SEA_SPEED  0.8
#define SEA_FREQ  0.16
#define SEA_BASE  float3(0.1, 0.19, 0.22)
#define SEA_WATER_COLOR  float3(0.8, 0.9, 0.6)
#define octave_m  float2x2(1.6, 1.2, -1.2, 1.6)

#define NUM_STEPS 8

#define PI  3.141592
#define EPSILON  1e-3

// Get random value.
float hash(float2 p)
{
    float h = dot(p, float2(127.1, 311.7));
    return frac(sin(h) * 43758.5453123);
}

// Get Noise.//value noise
float noise(float2 p)
{
    float2 i = floor(p);
    float2 f = frac(p);

    // u = -2.0f^3 + 3.0f^2
    float2 u = f * f * (3.0 - 2.0 * f);

    // Get each grid vertices.//hash�֐�����A�^����ꂽ���W�̏㉺���E�̒l���擾���Ă���
    // +---+---+
    // | a | b |
    // +---+---+
    // | c | d |
    // +---+---+
    float a = hash(i + float2(0.0, 0.0));
    float b = hash(i + float2(1.0, 0.0));
    float c = hash(i + float2(0.0, 1.0));
    float d = hash(i + float2(1.0, 1.0));

    //���������ɂ���ĕ��
    // Interpolate grid parameters with x and y.//a,b�����c,d���������iu.x�j�ɕ�Ԃ��A���̌��ʂ�����ɏc�����iu.y�j�ɕ��
    float result = lerp(lerp(a, b, u.x),
        lerp(c, d, u.x), u.y);


    // Normalized to '-1 - 1'.
    return (2.0 * result) - 1.0;
}

// Get sea wave octave.
float sea_octave(float2 uv, float choppy)
{
    //�኱�m�C�Y�����Z//���ׂȃm�C�Y��ǉ�
    uv += noise(uv);
    //�C�̔g
    float2 wv = 1.0 - abs(sin(uv));
    float2 swv = abs(cos(uv));
    wv = lerp(wv, swv, wv);
    return pow(1.0 - pow(wv.x * wv.y, 0.65), choppy);
}

// p is ray position.
float map(float3 p)
{
    float sea_time = (1.0 + something.iTime * SEA_SPEED);

    float freq = SEA_FREQ; // => 0.16
    float amp = SEA_HEIGHT; // => 0.6
    float choppy = SEA_CHOPPY; // => 4.0

    // XZ plane.
    float2 uv = p.xz;

    float d, h = 0.0;

    // ITER_GEOMETRY => 3
    //�����Łu�t���N�^���u���E���^���v�ɂ��m�C�Y�̌v�Z���s���Ă���
    //// �P���ɁAiTime�i���Ԍo�߁j�ɂ����UV�l������ɂ��炵�ăA�j���[�V���������Ă���
    for (int i = 0; i < ITER_GEOMETRY; i++)
    {
        //// SEA_TIME�̃v���X���ƃ}�C�i�X���𑫂��ĐU����傫�����Ă���
        d = sea_octave((uv + sea_time) * freq, choppy);
        d += sea_octave((uv - sea_time) * freq, choppy);
        h += d * amp;

        //// uv�l����]�����Ă��镗�B������Ȃ����ƕ��R�ȊC�ɂȂ�
        uv = mul(uv,octave_m);

        //// fbm�֐��Ƃ��āA�U����0.2�{�A���g����2.0�{���Ď��̌v�Z���s��
        freq *= 2.0;
        amp *= 0.2;

        //// choppy...�u�g�p�v
        // ���������������ƊC���u���ƂȂ����v�Ȃ�
        choppy = lerp(choppy, 1.0, 0.2);
    }

    return p.y - h;
}

// p is ray position.
// This function calculate detail map with more iteration count.
float map_detailed(float3 p)
{
    float sea_time = (1.0 + something.iTime * SEA_SPEED);


    float freq = SEA_FREQ;
    float amp = SEA_HEIGHT;
    float choppy = SEA_CHOPPY;

    float2 uv = p.xz;

    float d, h = 0.0;

    // ITER_FRAGMENT = 5
    for (int i = 0; i < ITER_FRAGMENT; i++)
    {
        d = sea_octave((uv + sea_time) * freq, choppy);
        d += sea_octave((uv - sea_time) * freq, choppy);
        h += d * amp;
        uv = mul(uv,octave_m);
        freq *= 2.0;
        amp *= 0.2;
        choppy = lerp(choppy, 1.0, 0.2);
    }

    return p.y - h;
}


// Get Height Map
float heightMapTracing(float3 ori, float3 dir, out float3 p)
{
    float tm = 0.0;

    float tx = 1000.0;

    // Calculate 1000m far distance map.
    float hx = map(ori + dir * tx);

    // if hx over 0.0 is that ray on the sky. right?
    if (hx > 0.0)
    {
        p = float3(0.,0.,0.);
        return tx;
    }

    //�J�n���_�ł̃��C�̈ʒu�ɉ�����Height Map�̋���
    float hm = map(ori + dir * tm);

    float tmid = 0.0;

    // NUM_STEPS = 8
    for (int i = 0; i < NUM_STEPS; i++)
    {
        // Calculate weight for 'tm' and 'tx' mixing.
        //����f�̒l�����Ɍv�Z�����l�Ń��C�̈ʒu���X�V
        float f = hm / (hm - hx);

        tmid = lerp(tm, tx, f);
        //update ray position
        //���C�̈ʒu���X�V
        p.x = ori.x + dir.z * tmid;
        p.y = ori.y + dir.y * tmid;
        p.z = ori.z + dir.z * tmid;

        float hmid = map(p);

        //�œK��
        // �T���v�����O�ʒu�̍������}�C�i�X�����̏ꍇ��`hx`, `tx`���X�V����
        if (hmid < 0.0)
        {
            tx = tmid;
            hx = hmid;
        }
        // �����łȂ��ꍇ�́A`hm`, `tm`���X�V����
        else
        {
            tm = tmid;
            hm = hmid;
        }
    }

    return tmid;
}

// tracing
float3 get_normal(float3 p, float eps)
{
    float3 n;
    n.y = map_detailed(p);
    n.x = map_detailed(float3(p.x + eps, p.y, p.z)) - n.y;
    n.z = map_detailed(float3(p.x, p.y, p.z + eps)) - n.y;
    n.y = eps;
    return normalize(n);
}

// lighting
float diffuse(float3 n, float3 l, float p)
{
    return pow(dot(n, l) * 0.4 + 0.6, p);
}

float specular(float3 n, float3 l, float3 e, float s)
{
    float nrm = (s + 8.0) / (PI * 8.0);
    return pow(max(dot(reflect(e, n), l), 0.0), s) * nrm;
}

// p = ray position
// n = surface normal
// l = light
// eye = eye
// dist = ray marching distance
float3 get_sea_color(float3 p, float3 n, float3 l, float3 eye, float3 dist,float2 uv)
{
    //�t���l���ifresnel �j�̎������߂�
    //Schlick���ɂ���ē����ꂽ�ߎ���
    float fresnel = clamp(1.0 - dot(n, -eye), 0.0, 1.0);
    fresnel = pow(fresnel, 3.0) * 0.65;

    //���ː�̋�̐F���v�Z����//���˂������C�Ɋւ���
    float3 reflected = float3(0.2, 0.2, 1.0); //sky_color.Sample(anisotropic_sampler_state, uv).xyz;
    //���ː�̊C�̐F���v�Z����//���܂������C�ɂ���
    float3 refracted = SEA_BASE + diffuse(n, l, 80.0) * SEA_WATER_COLOR * 0.12;

    //����
    float3 color = lerp(refracted, reflected, fresnel);

    float atten = max(1.0 - dot(dist, dist) * 0.001, 0.0);
    color += SEA_WATER_COLOR * (p.y - SEA_HEIGHT) * 0.18 * atten;

    //specular���ˌv�Z
    float3 s = specular(n, l, eye, 60.0);
    color += float3(s);

    return color;
}

//static const float2 wave1 = { 0.005,0.001 };
//static const float2 wave2 = { -0.005,0.016 };
PS_OUT main(VS_OUT pin)
{
	PS_OUT ret;
	
	float4 color = sky_color.Sample(anisotropic_sampler_state, pin.texcoord);

    //{
    //    // ray 
    //    float3 ori = float3(camera_constants.position.xyz);
    //    float3 dir = normalize(float3(camera_constants.direction.xyz));
    //    float3 light = float3(light_direction.direction.xyz);
    //    // tracing
    //    float3 p;
    //    heightMapTracing(ori, dir, p);
    //    float3 dist = p - ori;
    //    float3 n = get_normal(p, dot(dist, dist) * EPSILON);
    //    float3 sea = get_sea_color(p, n, light, dir, dist, pin.texcoord);
    //}


	float alpha = 1.0;
	float3 N = normalize(pin.normal.xyz);
	float3 T = float3(1.0001, 0, 0);
	float3 B = normalize(cross(N, T));
	T = normalize(cross(B, N));

	float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	normal.w = 0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
	N = N * 0.5 + 0.5;//0~1

	ret.Color = float4(color.rgb, alpha);
	ret.Normal = float4(N,1.0);
	ret.Position = pin.position;
	float depth = length(pin.position - camera_constants.position);
	ret.Depth = float4(depth, 0, 0, 1);
	ret.RM = float4(0.0, 0.0, 0.0, 1.0);
	return ret;

    //{
    //    // �@���擾(UV �X�N���[��)
    //    float3 N1 = seaTexture.Sample(linear_sampler_state,
    //        pin.texcoord + wave1 * something.iTime).xyz;
    //    N1 = N1 * 2.0f - 1.0f;
    //    // �@���擾(UV �X�N���[��)
    //    float3 N2 = seaTexture.Sample(linear_sampler_state,
    //        pin.texcoord + wave2 * something.iTime).xyz;
    //    N2 = N2 * 2.0f - 1.0f;
    //    float3 normal = normalize(
    //        //lerp( float3(0,0,1), N1 + N2, 0.1 )
    //        N1 + N2 + float3(0, 0, 10)
    //    );
    //    // �ϊ��s�񐶐�
    //    float3 Tangent = normalize(pin.tangent); // �ڐ�(�E)
    //    float3 Binormal = -normalize(pin.binormal); // �]�@��(��)
    //    float3 Normal = normalize(pin.normal); // �@��(����)
    //    float3x3 m = { Tangent, Binormal, Normal };
    //    // �@����ϊ�
    //    normal = mul(normal, m); // �@�� * �p���s��
    //    normal = normalize(normal);
    //    //���ʂ̃J���[
    //    float4 Color = float4(1, 1, 1, 1.0);
    //    ret.Color = Color;
    //    float dist = length(pin.position.xyz - camera_constants.position.xyz);
    //    ret.Depth = float4(dist, dist, dist, 1);
    //    ret.Normal = float4(normal * 0.5 + 0.5, 1);
    //    ret.Position = float4(pin.position.xyz, 1);
    //    //0713 R+M
    //    float roughness = 0;// RoughnessTexture.Sample(diffuseMapSamplerState, pin.texcoord).r;
    //    float metallic = 1;// MetallicTexture.Sample(diffuseMapSamplerState, pin.texcoord).r;
    //    ret.RM = float4(roughness, metallic, 0, 1);
    //    return ret;
    
}
