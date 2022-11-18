#include "pbr_static_mesh.hlsli"
#include "scene_constants.hlsli"
#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);
Texture2D texture_maps[4] : register(t0);

#define PI 3.141592653

//struct PARTICLE
//{
//	float4 color;
//	float3 position;
//	float3 normal;
//	float3 velocity;
//	float age;
//	int state;
//};
//AppendStructuredBuffer<PARTICLE> particle_buffer : register(u1);
//
//void main(VS_OUT pin)
//{
//	float4 color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
//	float alpha = color.a;
//	
//
//#if 1
//	// Inverse gamma process
//	const float GAMMA = 2.2;
//	color.rgb = pow(color.rgb, GAMMA);
//#endif
//
//	float3 N = normalize(pin.normal.xyz);
//#if 1
//	float3 T = normalize(pin.tangent.xyz);
//	float sigma = pin.tangent.w;
//	T = normalize(T - dot(N, T));
//	float3 B = normalize(cross(N, T) * sigma);
//
//	float4 normal = texture_maps[1].Sample(sampler_states[LINEAR], pin.texcoord);
//	normal = (normal * 2.0) - 1.0;
//	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
//#endif
//	float3 L = normalize(-light_direction.direction.xyz);
//	float3 diffuse = color.rgb * max(0, dot(N, L));
//	float3 V = normalize(camera_constants.position.xyz - pin.position.xyz);
//	float3 specular = pow(max(0, dot(N, normalize(V + L))), 128);
//	float3 ambient = color.rgb * 0.2;
//
//	PARTICLE p;
//	p.color = float4(max(0, ambient + diffuse + specular), alpha);
//	p.position = pin.position.xyz;
//	p.normal = N.xyz;
//	p.velocity = 0;
//	p.age = 0;
//	p.state = 0;
//	particle_buffer.Append(p);
//}

struct husk_particle
{
	float4 color;
	float3 position;
	float3 normal;
};
AppendStructuredBuffer<husk_particle> husk_particle_buffer : register(u1);

float3 fresnel_schlick(float NoL, float3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - NoL, 5.0);
}

float3 rendering_equation(float3 L, float3 V, float3 N, float3 irradiance, float3 diffuse_reflection, float3 specular_reflection, float shininess)
{
#if 0
	float tones = 4;
	//irradiance = ceil(irradiance * tones) / tones;
	irradiance = floor(irradiance * tones) / tones;
#endif

#if 0
	float3 reflectance = 0.2;
#else
	float3 reflectance = saturate(fresnel_schlick(max(0, dot(N, L)), float3(0.02, 0.02, 0.02)));
#endif

	float3 diffuse_exitance = diffuse_reflection * irradiance * (1.0 - reflectance);
	float3 diffuse_radiance = diffuse_exitance / PI;

	float smoothness = shininess;
	float3 specular_exitance = specular_reflection * irradiance * reflectance;
#if 0
	float3 R = reflect(-L, N);
	float3 specular_radiance = pow(max(0, dot(V, R)), smoothness) * specular_exitance;
#else
	float3 H = normalize(L + V);
	float3 specular_radiance = (smoothness + 8) / (8 * PI) * pow(max(0, dot(N, H)), smoothness) * specular_exitance;
#endif

	float3 radiance = diffuse_radiance + specular_radiance;

	return radiance;
}


[earlydepthstencil]
void main(VS_OUT pin)
{
	float4 diffuse_color = texture_maps[0].Sample(sampler_states[ANISOTROPIC], pin.texcoord);

	const float GAMMA = 2.2;
	float3 diffuse_reflection = pow(diffuse_color.rgb, GAMMA)/* * diffuse.rgb*/;
	const float alpha = diffuse_color.a /** diffuse.w*/;

	const float3 specular_reflection = float3(1.0, 1.0, 1.0); /*specular.rgb*/;
	const float shininess = 1.0;/*specular.w*/;


	float3 N = normalize(pin.normal.xyz);
#if 1
	float3 T = normalize(pin.normal.xyz);
	float sigma = pin.tangent.w;
	T = normalize(T - dot(N, T));
	float3 B = normalize(cross(N, T) * sigma);
	// Transform to world space from tangent space
	//                 |Tx Ty Tz|
	// normal = |x y z||Bx By Bz|
	//                 |Nx Ny Nz|
	float4 normal = texture_maps[1].Sample(sampler_states[LINEAR], pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));
#endif
	float3 V = normalize(camera_constants.position.xyz - pin.position.xyz);
	float3 L = 0.0;

	diffuse_reflection = lerp(
		diffuse_reflection,
		1.0,
		saturate(
			max(0., dot(N, float3(0, 1, 0))) *1.0 /*snow_factor*/
		)
	);
	float4 ambient = float4(1.0, 1.0, 1.0, 1.0);
	const float3 ambient_reflection = diffuse_reflection * ambient.rgb * ambient.w;

	float3 radiance = 0.0;
	float3 irradiance = 0.0;

	// Calculate the irradiance from the directional-light.
	L = normalize(-light_direction.direction.xyz);

	irradiance = light_direction.color.rgb *light_direction.color.w * max(0, (dot(N, L) * 0.5) + 0.5);
	radiance += rendering_equation(L, V, N, irradiance, diffuse_reflection, specular_reflection, shininess);

#if 0
	float3 ambient_light = environment_map.Sample(sampler_states[ANISOTROPIC], pin.world_normal.xyz).rgb * directional_light_color[0].w;
	//float3 ambient_light = 1;
	irradiance = ambient_light;
	//radiance += irradiance * ambient_reflection * float3(1, 1, 0.3)/*ad hoc coefficient*/;
	radiance += irradiance * ambient_reflection;
#endif

	// Calculate the irradiance from the omni-light.
	//L = omni_light_position[0].xyz - pin.position.xyz;
	//float d = length(L); // distance between light_position and surface_position
	//L = normalize(L);
	///*
	//		Distance	Kc		Kl		Kq
	//		7			1		0.7		1.8
	//		13			1		0.35	0.44
	//		20			1		0.22	0.2
	//		32			1		0.14	0.07
	//		50			1		0.09	0.032
	//		65			1		0.07	0.017
	//		100			1		0.045	0.0075
	//		160			1		0.027	0.0028
	//		200			1		0.022	0.0019
	//		325			1		0.014	0.0007
	//		600			1		0.007	0.0002
	//		3250		1		0.0014	0.000007
	//*/
	//float Kc = 1.0; // attenuation_constant
	//float Kl = 0.7; // attenuation_linear
	//float Kq = 1.8; // attenuation_quadratic
	//float attenuation = saturate(1.0 / (Kc + Kl * d + Kq * (d * d)));
	//irradiance = attenuation * /*omni_light_color[0].rgb * omni_light_color[0].w **/ max(0, (dot(N, L) * 0.5) + 0.5);
	//radiance += rendering_equation(L, V, N, irradiance, diffuse_reflection, specular_reflection, shininess);

#if 0
	// Apply reflection processing.
	radiance += environment_map.Sample(sampler_states[ANISOTROPIC], reflect(-V, N)).rgb * reflection.rgb * directional_light_color.w;
#endif

	// Add the emissive to the radiance.
	radiance += diffuse_reflection/* * emissive.rgb * emissive.w*/;

#if 0
	// Apply rimlighting.
	float rim = 1 - saturate(dot(V, N));
	float3 rimlight_radiance = rimlight_color.rgb * rimlight_color.w * smoothstep(rimlight_factor, 1.0, rim) * diffuse_reflection;
	radiance += rimlight_radiance;
#endif	

	husk_particle p;
	p.color = float4(max(0, radiance), alpha) ;
	p.position = pin.position.xyz;
	p.normal = N.xyz;
	husk_particle_buffer.Append(p);
}