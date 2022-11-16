#include "terrain.hlsli"
#include "scene_constants.hlsli"
#include "render_constants.hlsli"

Texture2D diffuse_map1 : register(t0);//moss_albedo
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);
//Texture2D diffuse_map2 : register(t5);//MuddyGround_Albedo
//Texture2D normal_map1 : register(t6);//moss_normal
//Texture2D normal_detail_map1 : register(t7);//Moss_detail_Normal
//Texture2D normal_map2 : register(t8); //MuddyGround_normal
//Texture2D metasmooth_map1 : register(t9);//Moss_MetallicSmooth
//Texture2D metasmooth_map2 : register(t10);//MuddyGround_MetallicSmooth
//Texture2D stones_diffuse_map : register(t11);//SmallStones_Albedo
//Texture2D stones_normal_map : register(t12);//SmallStones_Normal
//Texture2D stones_metallic_map : register(t13);//SmallStones_Metallic

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
	float4 Shadow_param : SV_TARGET5;
};

// Reoriented Normal Mapping
		// http://blog.selfshadow.com/publications/blending-in-detail/
		// Altered to take normals (-1 to 1 ranges) rather than unsigned normal maps (0 to 1 ranges)
//half3 blend_rnm(float3 n1, float3 n2)
//{
//	n1.z += 1;
//	n2.xy = -n2.xy;
//
//	return n1 * dot(n1, n2) / n1.z - n2;
//}

//PS_OUT main(VS_OUT pin)
//{
//	//MRT
//	PS_OUT ret;
//
//	float2 UVY = pin.position.xz;
//	float4 Albedo01 = diffuse_map2.Sample(linear_sampler_state, UVY * _TextureScale01);
//	float4 Albedo02 = diffuse_map1.Sample(linear_sampler_state, UVY * _TextureScale02);
//	float4 Albedo03 = stones_diffuse_map.Sample(linear_sampler_state, UVY * _TextureScale03);
//
//	//TODO
//	float4 vertCol = pin.color;
//
//	float blend01 = smoothstep(vertCol.r, vertCol.r - Falloff01, 1 - Albedo01.a);
//	float blend02 = smoothstep(vertCol.g, vertCol.g - Falloff02, 1 - Albedo03.a);
//
//	float3x3 objectToTangent = float3x3(pin.tangent.xyz, cross(pin.normal.xyz, pin.tangent.xyz) * pin.tangent.w, pin.normal.xyz);
//	float3 E = camera_constants.position.xyz - pin.position.xyz;
//	float3 tangentViewDir = mul(objectToTangent, E);
//
//	float dist = length(
//		pin.position - camera_constants.position);
//	ret.Depth = float4(dist, 0, 0, 1);
//
//	tangentViewDir = normalize(tangentViewDir);
//	tangentViewDir.xy /= tangentViewDir.z;
//
//	float _ParallaxStrength = 0.05;//0~1//éãç∑
//	float2 UVY2 = pin.position.xz;
//	UVY2 += tangentViewDir * _ParallaxStrength * blend01;
//
//	Albedo02 = diffuse_map1.Sample(linear_sampler_state, UVY2 * _TextureScale02);
//
//	float4 AlbedoFinal = Albedo01;
//	AlbedoFinal = lerp(AlbedoFinal, Albedo02, blend01);
//	AlbedoFinal = lerp(AlbedoFinal, Albedo01, blend02);
//
//	// tangent space normal map
//	float3 Normal01 = normal_map2.Sample(linear_sampler_state, UVY * _TextureScale01);
//	float3 Normal02 = normal_map1.Sample(linear_sampler_state, UVY2 * _TextureScale02);
//	float3 Normal03 = stones_normal_map.Sample(linear_sampler_state, UVY * _TextureScale03);
//	//float3 Normal02Detail = normal_detail_map1.Sample(linear_sampler_state, UVY * 0.1);
//
//	Normal01 = Normal01 * 2 - 1;
//	Normal02 = Normal02 * 2 - 1;
//	Normal03 = Normal03 * 2 - 1;
//	//Normal02Detail = Normal02Detail * 2 - 1;
//
//	// flip normal maps' x axis to account for flipped UVs
//	float3 absVertNormal = abs(pin.normal);
//
//	// swizzle world normals to match tangent space and apply reoriented normal mapping blend
//	float3 tangentNormal = lerp(Normal01, Normal02, blend01);
//	tangentNormal = lerp(tangentNormal, Normal03, blend02);
//	tangentNormal = blend_rnm(float3(pin.normal.xz, absVertNormal.y), tangentNormal);
//	// sizzle tangent normals to match world normal and blend together
//	float3 worldNormal = normalize(tangentNormal.xzy);
//	// convert world space normals into tangent normals
//	float3 NormalFinal = worldNormal;
//	//-1 <---> +1 Ç 0 <---> 1 Ç…R8G8B8A8ëŒâû
//	NormalFinal = NormalFinal * 0.5 + 0.5;
//	ret.Normal = float4(NormalFinal, 1);
//
//	ret.Color = AlbedoFinal;
//	ret.Position = pin.position;
//
//	ret.RM = metasmooth_map1.Sample(point_sampler_state, pin.texcoord);
//
//	return ret;
//
//}

PS_OUT main(VS_OUT pin)
{
	//MRT
	PS_OUT ret;

	float4 color = diffuse_map1.Sample(anisotropic_sampler_state, pin.texcoord);

	float alpha = color.a;
	float3 N = normalize(pin.normal.xyz);
	float3 T = float3(1.0001, 0, 0);
	float3 B = normalize(cross(N, T));
	T = normalize(cross(B, N));

	float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	normal.w = 0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
	N = N*0.5 + 0.5;
	float3 L = normalize(-light_direction.direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L) * 0.5 + 0.5);

	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
	float3 specular = pow(max(0, dot(N, normalize(E + L))), 1);//specular coordination

	float3 finalcolor = diffuse;

	ret.Color = float4(color.rgb, alpha);
	ret.Normal = float4(N,1.0);
	ret.Position = pin.position;
	float dist = length(pin.position - camera_constants.position);
    ret.Depth = float4(dist, 0, 0, 1);
	ret.RM = float4(1.0,0.0,0.0,0.0);

	ret.Shadow_param = float4(255.0f * shadow_reciever, 255.0f * shadow_reciever, 255.0f * shadow_reciever, 255.0f * shadow_reciever);
	return ret;

}