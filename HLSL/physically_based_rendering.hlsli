#ifndef __PHYSICALLY_BASED_RENDERING__
#define __PHYSICALLY_BASED_RENDERING__

#include "bidirectional_reflectance_distribution_function.hlsli"
#include "ibl.hlsli"

// This fragment shader defines a reference implementation for Physically Based Shading of
// a microfacet surface material defined by a glTF model.
//
// References:
// [1] Real Shading in Unreal Engine 4
//     http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// [2] Physically Based Shading at Disney
//     http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf
// [3] README.md - Environment Maps
//     https://github.com/KhronosGroup/glTF-WebGL-PBR/#environment-maps
// [4] "An Inexpensive BRDF Model for Physically based Rendering" by Christophe Schlick
//     https://www.cs.virginia.edu/~jdl/bib/appearance/analytic%20models/schlick94b.pdf
// [5] "KHR_materials_clearcoat"
//     https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_clearcoat

float4 physically_based_rendering(material_info material_info, float3 L, float3 V, float3 N, float3 P)
{
	float3 f_specular = 0.0;
	float3 f_diffuse = 0.0;
	float3 f_emissive = 0.0;
	float3 f_clearcoat = 0.0;
	float3 f_sheen = 0.0;
	float3 f_transmission = 0.0;

	float albedo_sheen_scaling = 1.0;
	// Calculate lighting contribution from image based lighting source (IBL)
	if (image_based_lighting > 0)
	{
		f_diffuse += ibl_radiance_lambertian(N, V, material_info.perceptual_roughness, material_info.c_diff, material_info.f0, material_info.specular_weight);
		f_specular += ibl_radiance_ggx(N, V, material_info.perceptual_roughness, material_info.f0, material_info.specular_weight);

	}

		float3 R = reflect(-L, N);
		float3 H = normalize(V + L);

		float NoL = max(0.0, dot(N, L));
		float NoH = max(0.0, dot(N, H));
		float HoV = max(0.0, dot(H, V));
		float NoV = max(0.0, dot(N, V));

		float3 Li = pure_white;
		if (NoL > 0.0 || NoV > 0.0)
		{
			// Calculation of analytical light
			// https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
			f_diffuse += Li * NoL * brdf_lambertian(material_info.f0, material_info.f90, material_info.c_diff, material_info.specular_weight, HoV);
			f_specular += Li * NoL * brdf_specular_ggx(material_info.f0, material_info.f90, material_info.alpha_roughness, material_info.specular_weight, HoV, NoL, NoV, NoH);

		}


		// Apply ambient occlusion to all lighting that is not punctual
		f_diffuse = lerp(f_diffuse, f_diffuse * material_info.occlusion_factor, material_info.occlusion_strength);
		f_specular = lerp(f_specular, f_specular * material_info.occlusion_factor, material_info.occlusion_strength);
		f_sheen = lerp(f_sheen, f_sheen * material_info.occlusion_factor, material_info.occlusion_strength);
		f_clearcoat = lerp(f_clearcoat, f_clearcoat * material_info.occlusion_factor, material_info.occlusion_strength);

		// Apply emissive factor
		f_emissive = material_info.emissive_factor;
#if 1
		f_emissive *= emissive_intensity;
#endif

		float3 color = 0.0;

		// Layer blending
		float clearcoat_factor = 0.0;
		float3 clearcoat_fresnel = 0.0;


		color = f_emissive + f_diffuse + f_specular;
		color = f_sheen + color * albedo_sheen_scaling;
		color = color * (1.0 - clearcoat_factor * clearcoat_fresnel) + f_clearcoat;
		return float4(max(0, color), material_info.basecolor.a);
	
}
#endif // __PHYSICALLY_BASED_RENDERING__