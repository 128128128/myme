#ifndef __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__
#define __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__

#include "image_based_lighting.hlsli"

// Fresnel
// http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
// https://github.com/wdas/brdf/tree/master/src/brdfs
// https://google.github.io/filament/Filament.md.html
// The following equation models the Fresnel reflectance term of the spec equation (aka F())
// Implementation of fresnel from [4], Equation 15
float3 f_schlick(float3 f0, float3 f90, float VoH)
{
	return f0 + (f90 - f0) * pow(clamp(1.0 - VoH, 0.0, 1.0), 5.0);
}
// Smith Joint GGX
// Note: Vis = G / (4 * NoL * NoV)
// see Eric Heitz. 2014. Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs. Journal of Computer Graphics Techniques, 3
// see Real-Time Rendering. Page 331 to 336.
// see https://google.github.io/filament/Filament.md.html#materialsystem/specularbrdf/geometricshadowing(specularg)
float v_ggx(float NoL, float NoV, float alpha_roughness)
{
	float alpha_roughness_sq = alpha_roughness * alpha_roughness;

	float ggxv = NoL * sqrt(NoV * NoV * (1.0 - alpha_roughness_sq) + alpha_roughness_sq);
	float ggxl = NoV * sqrt(NoL * NoL * (1.0 - alpha_roughness_sq) + alpha_roughness_sq);

	float ggx = ggxv + ggxl;
	return (ggx > 0.0) ? 0.5 / ggx : 0.0;
}
// The following equation(s) model the distribution of microfacet normals across the area being drawn (aka D())
// Implementation from "Average Irregularity Representation of a Roughened Surface for Ray Reflection" by T. S. Trowbridge, and K. P. Reitz
// Follows the distribution function recommended in the SIGGRAPH 2013 course notes from EPIC Games [1], Equation 3.
float d_ggx(float NoH, float alpha_roughness)
{
	float alpha_roughness_sq = alpha_roughness * alpha_roughness;
	float f = (NoH * NoH) * (alpha_roughness_sq - 1.0) + 1.0;
	return alpha_roughness_sq / (PI * f * f);
}

//  https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 brdf_specular_ggx(float3 f0, float3 f90, float alpha_roughness, float specular_weight, float VoH, float NoL, float NoV, float NoH)
{
	float3 F = f_schlick(f0, f90, VoH);
	float Vis = v_ggx(NoL, NoV, alpha_roughness);
	float D = d_ggx(NoH, alpha_roughness);

	return specular_weight * F * Vis * D;
}

//https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#acknowledgments AppendixB
float3 brdf_lambertian(float3 f0, float3 f90, float3 diffuse_color, float specular_weight, float VoH)
{
    // see https://seblagarde.wordpress.com/2012/01/08/pi-or-not-to-pi-in-game-lighting-equation/
	return (1.0 - specular_weight * f_schlick(f0, f90, VoH)) * (diffuse_color / PI);
}

// Estevez and Kulla http://www.aconty.com/pdf/s2017_pbs_imageworks_sheen.pdf
float d_charlie(float sheen_roughness, float NoH)
{
	sheen_roughness = max(sheen_roughness, 0.000001); //clamp (0,1]
	float alpha = sheen_roughness * sheen_roughness;
	float inv_alpha = 1.0 / alpha;
	float cos2h = NoH * NoH;
	float sin2h = 1.0 - cos2h;
	return (2.0 + inv_alpha) * pow(sin2h, inv_alpha * 0.5) / (2.0 * PI);
}
float lambda_sheen_numeric_helper(float x, float alpha)
{
	float one_minus_alpha_sq = (1.0 - alpha) * (1.0 - alpha);
	float a = lerp(21.5473, 25.3245, one_minus_alpha_sq);
	float b = lerp(3.82987, 3.32435, one_minus_alpha_sq);
	float c = lerp(0.19823, 0.16801, one_minus_alpha_sq);
	float d = lerp(-1.97760, -1.27393, one_minus_alpha_sq);
	float e = lerp(-4.32054, -4.85967, one_minus_alpha_sq);
	return a / (1.0 + b * pow(x, c)) + d * x + e;
}
float lambda_sheen(float cos_theta, float alpha)
{
	if (abs(cos_theta) < 0.5)
	{
		return exp(lambda_sheen_numeric_helper(cos_theta, alpha));
	}
	else
	{
		return exp(2.0 * lambda_sheen_numeric_helper(0.5, alpha) - lambda_sheen_numeric_helper(1.0 - cos_theta, alpha));
	}
}
float v_sheen(float NoL, float NoV, float sheen_roughness)
{
	sheen_roughness = max(sheen_roughness, 0.000001); //clamp (0,1]
	float alpha = sheen_roughness * sheen_roughness;

	return clamp(1.0 / ((1.0 + lambda_sheen(NoV, alpha) + lambda_sheen(NoL, alpha)) * (4.0 * NoV * NoL)), 0.0, 1.0);
}
float3 brdf_specular_sheen(float3 sheen_color, float sheen_roughness, float NoL, float NoV, float NoH)
{
	float sheen_distribution = d_charlie(sheen_roughness, NoH);
	float sheen_visibility = v_sheen(NoL, NoV, sheen_roughness);
	return sheen_color * sheen_distribution * sheen_visibility;
}

float3 ibl_radiance_ggx(float3 N, float3 V, float roughness, float3 f0, float specular_weight)
{
	float NoV = clamp(dot(N, V), 0.0, 1.0);
	
	float2 brdf_sample_point = clamp(float2(NoV, roughness), 0.0, 1.0);
	float2 f_ab = sample_lut_ggx(brdf_sample_point).rg;

	float3 R = normalize(reflect(-V, N));
	float3 specular_light = sample_specular_pmrem(R, roughness).rgb;

    // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
    // Roughness dependent fresnel, from Fdez-Aguera
	float3 fr = max(1.0 - roughness, f0) - f0;
	float3 k_s = f0 + fr * pow(1.0 - NoV, 5.0);
	float3 fss_ess = k_s * f_ab.x + f_ab.y;

	return specular_weight * specular_light * fss_ess;
}
// specularWeight is introduced with KHR_materials_specular
float3 ibl_radiance_lambertian(float3 N, float3 V, float roughness, float3 diffuse_color, float3 f0, float specular_weight)
{
	float NoV = clamp(dot(N, V), 0.0, 1.0);
	
	float2 brdf_sample_point = clamp(float2(NoV, roughness), 0.0, 1.0);
	float2 f_ab = sample_lut_ggx(brdf_sample_point).rg;

	float3 irradiance = sample_diffuse_iem(N).rgb;
	
    // see https://bruop.github.io/ibl/#single_scattering_results at Single Scattering Results
    // Roughness dependent fresnel, from Fdez-Aguera
	float3 fr = max(1.0 - roughness, f0) - f0;
	float3 k_s = f0 + fr * pow(1.0 - NoV, 5.0);
	float3 fss_ess = specular_weight * k_s * f_ab.x + f_ab.y; // <--- GGX / specular light contribution (scale it down if the specularWeight is low)

    // Multiple scattering, from Fdez-Aguera
	float ems = (1.0 - (f_ab.x + f_ab.y));
	float3 f_avg = specular_weight * (f0 + (1.0 - f0) / 21.0);
	float3 fms_ems = ems * fss_ess * f_avg / (1.0 - f_avg * ems);
	float3 k_d = diffuse_color * (1.0 - fss_ess + fms_ems); // we use +FmsEms as indicated by the formula in the blog post (might be a typo in the implementation)

	return (fms_ems + k_d) * irradiance;
}
float3 ibl_radiance_charlie(float3 N, float3 V, float sheen_roughness, float3 sheen_color)
{
	float NoV = clamp(dot(N, V), 0.0, 1.0);

	float2 brdf_sample_point = clamp(float2(NoV, sheen_roughness), 0.0, 1.0);
	float brdf = sample_lut_charlie(brdf_sample_point).b;
	
	float3 R = normalize(reflect(-V, N));
	float3 sheen_sample = sample_sheen_pmrem(R, sheen_roughness).rgb; // TODO

	float3 sheen_light = sheen_sample.rgb;
	return sheen_light * sheen_color * brdf;
}

float3 volume_transmission_ray(float3 N, float3 V, float thickness, float ior, column_major float4x4 model_matrix)
{
    // Direction of refracted light.
	float3 refraction_vector = refract(-V, normalize(N), 1.0 / ior);

    // Compute rotation-independant scaling of the model matrix.
	float3 model_scale;
#if 0
	model_scale.x = length(float3(model_matrix[0].xyz));
	model_scale.y = length(float3(model_matrix[1].xyz));
	model_scale.z = length(float3(model_matrix[2].xyz));
#else
	model_scale.x = length(mul(float4(1, 0, 0, 0), model_matrix));
	model_scale.y = length(mul(float4(0, 1, 0, 0), model_matrix));
	model_scale.z = length(mul(float4(0, 0, 1, 0), model_matrix));
#endif

    // The thickness is specified in local space.
	return normalize(refraction_vector) * thickness * model_scale;
}
// Compute attenuated light as it travels through a volume.
float3 apply_volume_attenuation(float3 radiance, float transmission_distance, float3 attenuation_color, float attenuation_distance)
{
	if (attenuation_distance == 0.0)
	{
        // Attenuation distance is +infinity (which we indicate by zero), i.e. the transmitted color is not attenuated at all.
		return radiance;
	}
	else
	{
        // Compute light attenuation using Beer's law.
		float3 attenuation_coefficient = -log(attenuation_color) / attenuation_distance;
		float3 transmittance = exp(-attenuation_coefficient * transmission_distance); // Beer's law
		return transmittance * radiance;
	}
}
float3 ibl_volume_refraction(float3 N, float3 V, float perceptual_roughness, float3 basecolor, float3 f0, float3 f90,
    float3 P, column_major float4x4 world_transform, column_major float4x4 view_projection_transform, float ior, float thickness, float3 attenuation_color, float attenuation_distance)
{
	float3 transmission_ray = volume_transmission_ray(N, V, thickness, ior, world_transform);
	float3 refracted_ray_exit = P + transmission_ray;

    // Project refracted vector on the framebuffer, while mapping to normalized device coordinates.
	float4 ndc = mul(float4(refracted_ray_exit, 1.0), view_projection_transform);
	ndc = ndc / ndc.w;
	
	float2 refraction_coords;
	refraction_coords.x = 0.5 + 0.5 * ndc.x;
	refraction_coords.y = 0.5 - 0.5 * ndc.y;

    // Sample framebuffer to get pixel the refracted ray hits.
	float3 transmitted_light = sample_transmission(refraction_coords, perceptual_roughness, ior).rgb;
	float3 attenuated_color = apply_volume_attenuation(transmitted_light, length(transmission_ray), attenuation_color, attenuation_distance);

    // Sample GGX LUT to get the specular component.
	float NoV = clamp(dot(N, V), 0.0, 1.0);
	float2 brdf_sample_point = clamp(float2(NoV, perceptual_roughness), 0.0, 1.0);
	float2 brdf = sample_lut_ggx(brdf_sample_point).rg;
	float3 specular_color = f0 * brdf.x + f90 * brdf.y;

	return (1.0 - specular_color) * attenuated_color * basecolor;
}

float3 punctual_radiance_transmission(float3 N, float3 V, float3 L, float alpha_roughness,
    float3 f0, float3 f90, float3 basecolor, float ior)
{
	float transmission_rougness = alpha_roughness * clamp(ior * 2.0 - 2.0, 0.0, 1.0);

	N = normalize(N); // Outward direction of surface point
	V = normalize(V); // Direction from surface point to view
	L = normalize(L);
	float3 l_mirror = normalize(L + 2.0 * N * dot(-L, N)); // Mirror light reflection vector on surface
	float3 H = normalize(l_mirror + V); // Halfway vector between transmission light vector and v

	float D = d_ggx(clamp(dot(N, H), 0.0, 1.0), transmission_rougness);
	float3 F = f_schlick(f0, f90, clamp(dot(V, H), 0.0, 1.0));
	float Vis = v_ggx(clamp(dot(N, l_mirror), 0.0, 1.0), clamp(dot(N, V), 0.0, 1.0), transmission_rougness);

    // Transmission BTDF
	return (1.0 - F) * basecolor * D * Vis;
}
#endif // __BIDIRECTIONAL_REFLECTANCE_DISTRIBUTION_FUNCTION_HLSL__