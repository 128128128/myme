#ifndef __IMAGE_BASED_LIGHTING__
#define __IMAGE_BASED_LIGHTING__

static const float gamma = 2.2;
static const float inv_gamma = 1.0 / 2.2;

static const float PI = 3.141592653;
static const float epsilon = 0.00001;

float max3(float3 v)
{
	return max(max(v.x, v.y), v.z);
}

#define POINT_WRAP 0
#define LINEAR_WRAP 1
#define ANISOTROPIC_WRAP 2
#define POINT_CLAMP 3
#define LINEAR_CLAMP 4
#define ANISOTROPIC_CLAMP 5
#define POINT_BORDER_OPAQUE_BLACK 6
#define LINEAR_BORDER_OPAQUE_BLACK 7
#define POINT_BORDER_OPAQUE_WHITE 8
#define LINEAR_BORDER_OPAQUE_WHITE 9
#define COMPARISON_DEPTH 10
SamplerState sampler_states[10] : register(s0);

#define USE_LATITUDE_LONGITUDE_MAP
#ifdef USE_LATITUDE_LONGITUDE_MAP	
Texture2D skybox : register(t32);
#else
TextureCube skybox : register(t32);
#endif

TextureCube diffuse_iem : register(t33);
TextureCube specular_pmrem : register(t34);
TextureCube sheen_pmrem : register(t35);

Texture2D lut_ggx : register(t36);
Texture2D lut_sheen_e : register(t37);
Texture2D lut_charlie : register(t38);

Texture2D transmission_framebuffer : register(t39);

float4 sample_lut_ggx(float2 brdf_sample_point)
{
	return lut_ggx.Sample(sampler_states[LINEAR_CLAMP], brdf_sample_point);
}
float albedo_sheen_scaling_lut(float NoV, float sheen_roughness_factor)
{
	return lut_sheen_e.Sample(sampler_states[LINEAR_CLAMP], float2(NoV, sheen_roughness_factor)).r;
}
float4 sample_lut_charlie(float2 brdf_sample_point)
{
	return lut_charlie.Sample(sampler_states[LINEAR_CLAMP], brdf_sample_point);
}
#if 1
float4 sample_skybox(float3 v, float roughness)
{
	uint width, height, number_of_levels;
	skybox.GetDimensions(0, width, height, number_of_levels);
	
	float lod = roughness * float(number_of_levels - 1);
	
#ifdef USE_LATITUDE_LONGITUDE_MAP	
	v = normalize(v);
    // Blinn/Newell Latitude Mapping
	float2 sample_point;
	sample_point.x = (atan2(v.z, v.x) + PI) / (PI * 2.0);
	sample_point.y = 1.0 - ((asin(v.y) + PI * 0.5) / PI);
	return skybox.SampleLevel(sampler_states[LINEAR_CLAMP], sample_point, lod);
#else
	return skybox.SampleLevel(sampler_states[LINEAR_CLAMP], v, lod);
#endif
}
#endif
float4 sample_diffuse_iem(float3 v)
{
	return diffuse_iem.Sample(sampler_states[LINEAR_CLAMP], v);
}
float4 sample_specular_pmrem(float3 v, float roughness)
{
	uint width, height, number_of_levels;
	specular_pmrem.GetDimensions(0, width, height, number_of_levels);
	
	float lod = roughness * float(number_of_levels - 1);
	
	return specular_pmrem.SampleLevel(sampler_states[LINEAR_CLAMP], v, lod);
}
float4 sample_sheen_pmrem(float3 v, float roughness)
{
	uint width, height, number_of_levels;
	specular_pmrem.GetDimensions(0, width, height, number_of_levels);
	
	float lod = roughness * float(number_of_levels - 1);

	return specular_pmrem.SampleLevel(sampler_states[LINEAR_CLAMP], v, lod); // TODO
}
float4 sample_transmission(float2 texcoord, float roughness, float ior)
{
	uint width, height, number_of_levels;
	transmission_framebuffer.GetDimensions(0, width, height, number_of_levels);
	
	// Scale roughness with IOR so that an IOR of 1.0 results in no microfacet refraction and
    // an IOR of 1.5 results in the default amount of microfacet refraction.
	//IOR を使って粗さをスケーリングし、IOR が 1.0 の場合はマイクロファセット屈折が発生せず
	//IORが1.5の場合は、デフォルトのマイクロファセット屈折量になります。
#if 0
	float lod = log2(width) * roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
#else
	float lod = number_of_levels * roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);
#endif
	
	return transmission_framebuffer.SampleLevel(sampler_states[LINEAR_CLAMP], texcoord, lod);
}
#endif // __IMAGE_BASED_LIGHTING__