#include "static_mesh.hlsli"
#include "scene_constants.hlsli"

Texture2D diffuse_map : register(t0);
Texture2D ambient_map : register(t1);
Texture2D specular_map : register(t2);
Texture2D normal_map : register(t3);

SamplerState point_sampler_state : register(s0);
SamplerState linear_sampler_state : register(s1);
SamplerState anisotropic_sampler_state : register(s2);

float4 main(VS_OUT pin) : SV_TARGET
{
	float4 color = diffuse_map.Sample(anisotropic_sampler_state, pin.texcoord);
	float alpha = color.a;
	float3 N = normalize(pin.normal.xyz);
	float3 T = float3(1.0001, 0, 0);
	float3 B = normalize(cross(N, T));
	T = normalize(cross(B, N));

	float4 normal = normal_map.Sample(linear_sampler_state, pin.texcoord);
	normal = (normal * 2.0) - 1.0;
	normal.w = 0;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

	float3 L = normalize(-light_direction.direction.xyz);
	float3 diffuse = color.rgb * max(0, dot(N, L) * 0.5 + 0.5);

	float3 E = normalize(camera_constants.position.xyz - pin.position.xyz);
	float3 specular = pow(max(0, dot(N, normalize(E + L))), 3);//specular coordination

	//----point light----//
	float3 lig_direction = pin.position - spot_light.position;
	lig_direction = normalize(lig_direction);
	//spot light diffuse colour
	float3 diff_spot = spot_light.color.rgb * max(0, dot(N, lig_direction));
	//spot light specular colour
	float3 spec_spot = pow(max(0, dot(N, normalize(E + lig_direction))), 3);
	float3 distance = length(pin.position - spot_light.position);
	//influence is in proportion to be small
	float affect = 1.0f - 1.0f / spot_light.range * distance;
	//no minus
	if (affect < 0.0f)
	{
		affect = 0.0f;
	}
	//like exponential function(Žw”ŠÖ”)
	affect = pow(affect, 3.0f);

	diff_spot *= affect;
	spec_spot *= affect;

	//incident light(“üŽËŒõ) & injection direction(ŽËo•ûŒü) angle
	float angle = dot(lig_direction, spot_light.direction);
	angle = abs(acos(angle));

	//influence is in proportion to angle is small
	affect = 1.0f - 1.0f / spot_light.angle * angle;

	//like exponential function(Žw”ŠÖ”)
	affect = pow(affect, 0.5f);

	diff_spot *= affect;
	spec_spot *= affect;

	return float4(diff_spot /*+ diffuse*/ + spec_spot/*+specular*/, alpha) /** pin.color*/;


}