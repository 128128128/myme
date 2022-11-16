#include "geometric_substance.hlsli"
#include "common.hlsli"

#define KHR_texture_transform
#define KHR_materials_sheen
#define KHR_materials_clearcoat
#define KHR_materials_specular

#define KHR_materials_transmission
#define KHR_materials_volume

//#define KHR_materials_unlit
//#define KHR_materials_pbrSpecularGlossiness

#include "material.hlsli"
#include "physically_based_rendering.hlsli"

#if 0
// If you enable earlydepthstencil, 'clip', 'discard' and 'Alpha to coverage' won't work!
[earlydepthstencil]
#endif
float4 main(VS_OUT pin, bool is_front_face : SV_IsFrontFace) : SV_TARGET
{
	const float3 P = pin.w_position.xyz;

	const material_constants material = materials[primitive_data.material];
	material_info material_info = fetch_material_info(material, pin.texcoord);
	
#if 0
	clip(material_info.basecolor.a - 0.25);
#endif
	
	if (material.alpha_mode == ALPHAMODE_OPAQUE)
	{
		material_info.basecolor.a = 1.0;
	}

#ifdef KHR_materials_unlit	
	clip(material_info.basecolor.a - material.alpha_cutoff);
	return material_info.basecolor;
#endif	
	
	float3 V = normalize(scene_data.eye_position.xyz - pin.w_position.xyz);
	float3 L = normalize(-scene_data.light_direction.xyz);

	float3 N = normalize(pin.w_normal.xyz);
	float3 T = normalize(pin.w_tangent.xyz);
	float sigma = pin.w_tangent.w;
	T = normalize(T - N * dot(N, T));
	float3 B = normalize(cross(N, T) * sigma);
#if 1
	// For a back-facing surface, the tangential basis vectors are negated.
	if (is_front_face == false)
	{
		T = -T;
		B = -B;
		N = -N;
	}
#endif
	
	const int normal_texture = material.normal_texture.index;
	if (normal_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(pin.texcoord, material.normal_texture.khr_texture_transform);
		float4 sampled = material_textures[NORMAL_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		float3 normal_factor = sampled.xyz;
		normal_factor = (normal_factor * 2.0) - 1.0;
		normal_factor = normalize(normal_factor * float3(material.normal_texture.scale, material.normal_texture.scale, 1.0));
		N = normalize((normal_factor.x * T) + (normal_factor.y * B) + (normal_factor.z * N));
	}

#ifdef KHR_materials_clearcoat
	material_info.clearcoat_normal = normalize(pin.w_normal.xyz);
	const int clearcoat_normal_texture = material.khr_materials_clearcoat.clearcoat_normal_texture.index;
	if (clearcoat_normal_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(pin.texcoord, material.khr_materials_clearcoat.clearcoat_normal_texture.khr_texture_transform);
		float4 sampled = material_textures[CLEARCOAT_NORMAL_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		float3 normal_factor = sampled.xyz;
		normal_factor = (normal_factor * 2.0) - 1.0;
		normal_factor = normalize(normal_factor * float3(material.khr_materials_clearcoat.clearcoat_normal_texture.scale, material.khr_materials_clearcoat.clearcoat_normal_texture.scale, 1.0));
		float3 N = normalize(material_info.clearcoat_normal);
		material_info.clearcoat_normal = normalize((normal_factor.x * T) + (normal_factor.y * B) + (normal_factor.z * N));
	}
 #endif
  
	return physically_based_rendering(material_info, L, V, N, P);
}


