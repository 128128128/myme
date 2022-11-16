#ifndef __MATERIAL_HLSL__
#define __MATERIAL_HLSL__

#include "common.hlsli"

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4

#define DIFFUSE_TEXTURE 5
#define SPECULAR_GLOSSINESS_TEXTURE 6

#define SHEEN_COLOR_TEXTURE 7
#define SHEEN_ROUGHNESS_TEXTURE 8

#define SPECULAR_TEXTURE 9
#define SPECULAR_COLOR_TEXTURE 10

#define CLEARCOAT_TEXTURE 11
#define CLEARCOAT_ROUGHNESS_TEXTURE 12
#define CLEARCOAT_NORMAL_TEXTURE 13

#define TRANSMISSION_TEXTURE 14
#define THICKNESS_TEXTURE 15

Texture2D<float4> material_textures[16] : register(t1);

// KHR_texture_transform
// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_texture_transform
struct khr_texture_transform
{
	float2 offset;
	float rotation;
	float2 scale;
	int texcoord;
};
// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-textureinfo
struct texture_info
{
	int index; // required.
	int texcoord; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
    
	khr_texture_transform khr_texture_transform;
};
// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material-normaltextureinfo
struct normal_texture_info
{
	int index; // required
	int texcoord; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
	float scale; // scaledNormal = normalize((<sampled normal texture value> * 2.0 - 1.0) * vec3(<normal scale>, <normal scale>, 1.0))
    
	khr_texture_transform khr_texture_transform;
};
// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material-occlusiontextureinfo
struct occlusion_texture_info
{
	int index; // required
	int texcoord; // The set index of texture's TEXCOORD attribute used for texture coordinate mapping.
	float strength; // A scalar parameter controlling the amount of occlusion applied. A value of `0.0` means no occlusion. A value of `1.0` means full occlusion. This value affects the final occlusion value as: `1.0 + strength * (<sampled occlusion texture value> - 1.0)`.

	khr_texture_transform khr_texture_transform;
};
// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material-pbrmetallicroughness
struct pbr_metallic_roughness
{
	float4 basecolor_factor; // len = 4. default [1,1,1,1]
	texture_info basecolor_texture;
	float metallic_factor; // default 1
	float roughness_factor; // default 1
	texture_info metallic_roughness_texture;
};
// KHR_materials_pbrSpecularGlossiness
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Archived/KHR_materials_pbrSpecularGlossiness/README.md
struct khr_materials_pbr_specular_glossiness
{
	float4 diffuse_factor;
	texture_info diffuse_texture;
	float3 specular_factor;
	float glossiness_factor;
	texture_info specular_glossiness_texture;
};
// KHR_materials_sheen
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_sheen/README.md
struct khr_materials_sheen
{
	float3 sheen_color_factor;
	texture_info sheen_color_texture;
	float sheen_roughness_factor;
	texture_info sheen_roughness_texture;
};
// KHR_materials_specular
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_specular/README.md
struct khr_materials_specular
{
	float specular_factor;
	texture_info specular_texture;
	float3 specular_color_factor;
	texture_info specular_color_texture;
};
// KHR_materials_clearcoat
// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_clearcoat
struct khr_materials_clearcoat
{
	float clearcoat_factor;
	texture_info clearcoat_texture;
	float clearcoat_roughness_factor;
	texture_info clearcoat_roughness_texture;
	normal_texture_info clearcoat_normal_texture;
};
// KHR_materials_transmission
// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_materials_transmission
struct khr_materials_transmission
{
	float transmission_factor;
	texture_info transmission_texture;
};
// KHR_materials_volume
// https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Khronos/KHR_materials_volume/README.md
struct khr_materials_volume
{
	float thickness_factor;
	texture_info thickness_texture;
	float attenuation_distance;
	float3 attenuation_color;
};



// "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
#define ALPHAMODE_OPAQUE 0
#define ALPHAMODE_MASK 1
#define ALPHAMODE_BLEND 2
// https://www.khronos.org/registry/glTF/specs/2.0/glTF-2.0.html#reference-material
struct material_constants
{
	float3 emissive_factor; // length 3. default [0, 0, 0]
	int alpha_mode; // "OPAQUE" : 0, "MASK" : 1, "BLEND" : 2
	float alpha_cutoff; // default 0.5
	int double_sided; // default false;

	pbr_metallic_roughness pbr_metallic_roughness;

	normal_texture_info normal_texture;
	occlusion_texture_info occlusion_texture;
	texture_info emissive_texture;
    
	// Extensions for glTF 2.0
	khr_materials_pbr_specular_glossiness khr_materials_pbr_specular_glossiness;
	khr_materials_sheen khr_materials_sheen;
	khr_materials_specular khr_materials_specular;
	khr_materials_clearcoat khr_materials_clearcoat;
	khr_materials_transmission khr_materials_transmission;
	khr_materials_volume khr_materials_volume;
};
StructuredBuffer<material_constants> materials : register(t0);

// KHR_texture_transform
// https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_texture_transform
float2 transform_texcoord(float2 texcoord, khr_texture_transform khr_texture_transform)
{
#ifdef KHR_texture_transform
	float2 transformed_texcoord = texcoord;
#if 1
	const float2 offset = khr_texture_transform.offset;
	const float rotation = khr_texture_transform.rotation;
	const float2 scale = khr_texture_transform.scale;
        
	const float cosine = cos(rotation);
	const float sine = sin(rotation);
        
	const column_major matrix<float, 3, 3> M = { scale.x * cosine, -scale.y * sine, offset.x, scale.x * sine, scale.y * cosine, offset.x, 0, 0, 1 };
	transformed_texcoord = mul(float3(texcoord.x, texcoord.y, 1), M).xy;
#else
	transformed_texcoord *= khr_texture_transform.scale;
	transformed_texcoord += khr_texture_transform.offset;
#endif
	return transformed_texcoord;
#else
	return texcoord;
#endif
}

struct material_info
{
	float4 basecolor;
	
	float ior;
	float perceptual_roughness; // roughness value, as authored by the model creator (input to shader)
	float3 f0; // full reflectance color (n incidence angle)

	float alpha_roughness; // roughness mapped to a more linear change in the roughness (proposed by [2])
	float3 c_diff;

	float3 f90; // reflectance color at grazing angle
	float metallic;

	float occlusion_factor;
	float occlusion_strength;
	
	float3 emissive_factor;
	
	float sheen_roughness_factor;
	float3 sheen_color_factor;

	float3 clearcoat_f0;
	float3 clearcoat_f90;
	float clearcoat_factor;
	float3 clearcoat_normal;
	float clearcoat_roughness;

    // KHR_materials_specular 
	float specular_weight; // product of specularFactor and specularTexture.a

	float transmission_factor;

	float thickness_factor;
	float3 attenuation_color;
	float attenuation_distance;
};

material_info fetch_material_info(material_constants material, float2 texcoord)
{
	material_info material_info;
#if 1
	material_info.basecolor = 1.0;
	material_info.ior = 1.5;
	material_info.perceptual_roughness = 1.0;
	material_info.f0 = 0.04;
	material_info.alpha_roughness = 1.0;
	material_info.c_diff = 1.0;
	material_info.f90 = 1.0;
	material_info.metallic = 1.0;
	material_info.sheen_roughness_factor = 0.0;
	material_info.sheen_color_factor = 0.0;
	material_info.clearcoat_f0 = 0.04;
	material_info.clearcoat_f90 = 1.0;
	material_info.clearcoat_factor = 0.0;
	material_info.clearcoat_normal = 0.0;
	material_info.clearcoat_roughness = 1.0;
	material_info.specular_weight = 1.0;
	material_info.transmission_factor = 0.0;
	material_info.thickness_factor = 0.0;
	material_info.attenuation_color = 1.0;
	material_info.attenuation_distance = 0.0;
#endif
	
#ifdef KHR_materials_pbrSpecularGlossiness
	float4 diffuse_factor = material.khr_materials_pbr_specular_glossiness.diffuse_factor;   
	const int diffuse_texture = material.khr_materials_pbr_specular_glossiness.diffuse_texture.index;
	if (diffuse_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_pbr_specular_glossiness.diffuse_texture.khr_texture_transform);
		float4 sampled = material_textures[DIFFUSE_TEXTURE].Sample(sampler_states[ANISOTROPIC_WRAP], transformed_texcoord);
		sampled.rgb = pow(sampled.rgb, gamma);
		diffuse_factor *= sampled;
	}
	material_info.basecolor = diffuse_factor;
#else	
	float4 basecolor_factor = material.pbr_metallic_roughness.basecolor_factor;
	const int basecolor_texture = material.pbr_metallic_roughness.basecolor_texture.index;
	if (basecolor_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.pbr_metallic_roughness.basecolor_texture.khr_texture_transform);
		float4 sampled = material_textures[BASECOLOR_TEXTURE].Sample(sampler_states[ANISOTROPIC_WRAP], transformed_texcoord);
		sampled.rgb = pow(sampled.rgb, gamma);
		basecolor_factor *= sampled;
	}
	material_info.basecolor = basecolor_factor;
#endif

#ifdef KHR_materials_pbrSpecularGlossiness
	float4 specular_glossiness_factor;
	specular_glossiness_factor.rgb = material.khr_materials_pbr_specular_glossiness.specular_factor;
	specular_glossiness_factor.a = material.khr_materials_pbr_specular_glossiness.glossiness_factor;
	const int specular_glossiness_texture = material.khr_materials_pbr_specular_glossiness.specular_glossiness_texture.index;
	if (specular_glossiness_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_pbr_specular_glossiness.specular_glossiness_texture.khr_texture_transform);
		float4 sampled = material_textures[SPECULAR_GLOSSINESS_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		specular_glossiness_factor.rgb *= sampled.rgb;
		specular_glossiness_factor.a *= sampled.a;
	}
	material_info.f0 = specular_glossiness_factor.rgb;
	material_info.perceptual_roughness = specular_glossiness_factor.a;
	material_info.perceptual_roughness = 1.0 - material_info.perceptual_roughness; // 1 - glossiness
	material_info.c_diff = material_info.basecolor.rgb * (1.0 - max(max(material_info.f0.r, material_info.f0.g), material_info.f0.b));
#else
	float roughness_factor = material.pbr_metallic_roughness.roughness_factor;
	float metallic_factor = material.pbr_metallic_roughness.metallic_factor;
	const int metallic_roughness_texture = material.pbr_metallic_roughness.metallic_roughness_texture.index;
	if (metallic_roughness_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.pbr_metallic_roughness.metallic_roughness_texture.khr_texture_transform);
		float4 sampled = material_textures[METALLIC_ROUGHNESS_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		roughness_factor *= sampled.g;
		metallic_factor *= sampled.b;
	}
	material_info.metallic = metallic_factor;
	material_info.perceptual_roughness = roughness_factor;
	
    // Achromatic f0 based on IOR.
	material_info.c_diff = lerp(material_info.basecolor.rgb, 0.0, material_info.metallic);
	material_info.f0 = lerp(material_info.f0, material_info.basecolor.rgb, material_info.metallic);
#endif

#ifdef KHR_materials_sheen
	float3 sheen_color_factor = material.khr_materials_sheen.sheen_color_factor;
	const int sheen_color_texture = material.khr_materials_sheen.sheen_color_texture.index;
	if (sheen_color_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_sheen.sheen_color_texture.khr_texture_transform);
		float4 sampled = material_textures[SHEEN_COLOR_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		sampled.rgb = pow(sampled.rgb, gamma);
		sheen_color_factor *= sampled.rgb;
	}
	material_info.sheen_color_factor = sheen_color_factor;
	
	float sheen_roughness_factor = material.khr_materials_sheen.sheen_roughness_factor;
	const int sheen_roughness_texture = material.khr_materials_sheen.sheen_roughness_texture.index;
	if (sheen_roughness_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_sheen.sheen_roughness_texture.khr_texture_transform);
		float4 sampled = material_textures[SHEEN_ROUGHNESS_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		sheen_roughness_factor *= sampled.a;
	}
	material_info.sheen_roughness_factor = sheen_roughness_factor;
#endif

	
#ifdef KHR_materials_specular
	float3 specular_color_factor = material.khr_materials_specular.specular_color_factor;
	const int specular_color_texture = material.khr_materials_specular.specular_color_texture.index;
	if (specular_color_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_specular.specular_color_texture.khr_texture_transform);
		float4 sampled = material_textures[SPECULAR_COLOR_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		specular_color_factor *= sampled.rgb;
	}
	
	float specular_factor = material.khr_materials_specular.specular_factor;
	const int specular_texture = material.khr_materials_specular.specular_texture.index;
	if (specular_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_specular.specular_texture.khr_texture_transform);
		float4 sampled = material_textures[SPECULAR_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		specular_factor *= sampled.a; // A texture that defines the strength of the specular reflection, stored in the alpha (A) channel. This will be multiplied by specularFactor.
	}
	float3 dielectric_specular_f0 = min(material_info.f0 * specular_color_factor, 1.0);
	material_info.f0 = lerp(dielectric_specular_f0, material_info.basecolor.rgb, material_info.metallic);
	material_info.specular_weight = specular_factor;
	material_info.c_diff = lerp(material_info.basecolor.rgb, 0, material_info.metallic);
#endif
	
#ifdef KHR_materials_clearcoat
	float clearcoat_factor = material.khr_materials_clearcoat.clearcoat_factor;
	const int clearcoat_texture = material.khr_materials_clearcoat.clearcoat_texture.index;
	if (clearcoat_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_clearcoat.clearcoat_texture.khr_texture_transform);
		float4 sampled = material_textures[CLEARCOAT_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		clearcoat_factor *= sampled.r;
	}
	material_info.clearcoat_factor = clearcoat_factor;
	
	float clearcoat_roughness_factor = material.khr_materials_clearcoat.clearcoat_roughness_factor;
	const int clearcoat_roughness_texture = material.khr_materials_clearcoat.clearcoat_roughness_texture.index;
	if (clearcoat_roughness_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_clearcoat.clearcoat_roughness_texture.khr_texture_transform);
		float4 sampled = material_textures[CLEARCOAT_ROUGHNESS_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		clearcoat_roughness_factor *= sampled.g;
	}
	material_info.clearcoat_roughness = clamp(clearcoat_roughness_factor, 0.0, 1.0);

	material_info.clearcoat_f0 = pow((material_info.ior - 1.0) / (material_info.ior + 1.0), 2.0);
	material_info.clearcoat_f90 = 1.0;
#endif
	
#ifdef KHR_materials_transmission
	float transmission_factor = material.khr_materials_transmission.transmission_factor;
	const int transmission_texture = material.khr_materials_transmission.transmission_texture.index;
	if (transmission_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_transmission.transmission_texture.khr_texture_transform);
		float4 sampled = material_textures[TRANSMISSION_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		transmission_factor *= sampled.g;
	}
	material_info.transmission_factor = transmission_factor;
#endif
	
#ifdef KHR_materials_volume
	float thickness_factor = material.khr_materials_volume.thickness_factor;
	const int thickness_texture = material.khr_materials_volume.thickness_texture.index;
	if (thickness_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.khr_materials_volume.thickness_texture.khr_texture_transform);
		float4 sampled = material_textures[THICKNESS_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		thickness_factor *= sampled.g;
	}
	material_info.thickness_factor = thickness_factor;
	material_info.attenuation_color = material.khr_materials_volume.attenuation_color;
	material_info.attenuation_distance = material.khr_materials_volume.attenuation_distance;
#endif
	
	float occlusion_factor = 1.0;
	const int occlusion_texture = material.occlusion_texture.index;
	if (occlusion_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.occlusion_texture.khr_texture_transform);
		float4 sampled = material_textures[OCCLUSION_TEXTURE].Sample(sampler_states[LINEAR_WRAP], transformed_texcoord);
		occlusion_factor *= sampled.r;
	}
	material_info.occlusion_factor = occlusion_factor;
	material_info.occlusion_strength = material.occlusion_texture.strength;

	float3 emissive_factor = material.emissive_factor;
	const int emissive_texture = material.emissive_texture.index;
	if (emissive_texture > -1)
	{
		float2 transformed_texcoord = transform_texcoord(texcoord, material.emissive_texture.khr_texture_transform);
		float4 sampled = material_textures[EMISSIVE_TEXTURE].Sample(sampler_states[ANISOTROPIC_WRAP], transformed_texcoord);
		sampled.rgb = pow(sampled.rgb, gamma);
		emissive_factor *= sampled.rgb;
	}
	material_info.emissive_factor = emissive_factor;
	
    // Roughness is authored as perceptual roughness; as is convention,
    // convert to material roughness by squaring the perceptual roughness.
	material_info.alpha_roughness = material_info.perceptual_roughness * material_info.perceptual_roughness;

	// Anything less than 2% is physically impossible and is instead considered to be shadowing. Compare to "Real-Time-Rendering" 4th editon on page 325.
	material_info.f90 = 1.0;
	
	
	return material_info;
}
#endif // __MATERIAL_HLSL__