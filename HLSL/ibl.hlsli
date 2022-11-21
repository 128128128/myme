//texture maps
Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);
Texture2D emissive_map : register(t5);
//pbr metallic
Texture2D metallic_map : register(t6);

cbuffer IBL_CONSTANT_BUFFER:register(b5)
{
    int image_based_lighting;//bool ibl
	float pure_white;
	float emissive_intensity;//emissive‚Ì‹­‚³
    float roughness_factor;//roughness‚Ì‹­‚³
	float metallic_factor;//metallic‚Ì‹­‚³
	float emissive_factor;
	float occlusion_factor;
	float occlusion_strength;
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

