#include "static_mesh.hlsli"
#include "scene_constants.hlsli"
#include "render_constants.hlsli"

Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);

//sky texture
Texture2D sky_color:register(t8);

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

PS_OUT main(VS_OUT pin)
{
	PS_OUT ret;
	
	float4 color = sky_color.Sample(anisotropic_sampler_state, pin.texcoord);
	
	float alpha = 1.0;
	float3 N = normalize(pin.normal.xyz);
	float3 T = float3(1.0001, 0, 0);
	float3 B = normalize(cross(N, T));
	T = normalize(cross(B, N));

	float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	normal.w = 0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

	float3 L = normalize(-light_direction.direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L));

	float3 finalcolor = diffuse;

	ret.Color = float4(finalcolor, alpha);
	ret.Normal = float4(N, 0.0);
	ret.Position = pin.position;
	float dist = length(pin.position - camera_constants.position);
	ret.Depth = float4(dist, 0, 0, 1);
	ret.RM = color;
	return ret;
}

//PS_OUT main(VS_OUT pin) {
//	i.viewDir = normalize(i.viewDir);
//
//	// combine two scrolling bumpmaps into one
//	half3 bump1 = UnpackNormal(tex2D(_BumpMap, i.bumpuv0)).rgb;
//	half3 bump2 = UnpackNormal(tex2D(_BumpMap, i.bumpuv1)).rgb;
//	half3 bump = (bump1 + bump2) * 0.5;
//
//	// fresnel factor
//	half fresnelFac = dot(i.viewDir, bump);
//
//	// perturb reflection/refraction UVs by bumpmap, and lookup colors
//
//#if HAS_REFLECTION
//	float4 uv1 = i.ref; uv1.xy += bump * _ReflDistort;
//	half4 refl = tex2Dproj(_ReflectionTex, UNITY_PROJ_COORD(uv1));
//#endif
//#if HAS_REFRACTION
//	float4 uv2 = i.ref; uv2.xy -= bump * _RefrDistort;
//	half4 refr = tex2Dproj(_RefractionTex, UNITY_PROJ_COORD(uv2)) * _RefrColor;
//#endif
//
//	// final color is between refracted and reflected based on fresnel
//	half4 color;
//
//#if defined(WATER_REFRACTIVE)
//	half fresnel = UNITY_SAMPLE_1CHANNEL(_Fresnel, float2(fresnelFac, fresnelFac));
//	color = lerp(refr, refl, fresnel);
//#endif
//
//#if defined(WATER_REFLECTIVE)
//	half4 water = tex2D(_ReflectiveColor, float2(fresnelFac, fresnelFac));
//	color.rgb = lerp(water.rgb, refl.rgb, water.a);
//	color.a = refl.a * water.a;
//#endif
//
//#if defined(WATER_SIMPLE)
//	half4 water = tex2D(_ReflectiveColor, float2(fresnelFac, fresnelFac));
//	color.rgb = lerp(water.rgb, _HorizonColor.rgb, water.a);
//	color.a = _HorizonColor.a;
//#endif
//
//	UNITY_APPLY_FOG(i.fogCoord, color);
//	return color;
//}

