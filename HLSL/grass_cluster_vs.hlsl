#include "grass_cluster.hlsli"
#include "scene_constants.hlsli"

VS_OUT main(VS_IN vin)
{
	VS_OUT vout;

	float4 position = mul(vin.position, vin.transform);
	vout.position = position;

	vin.normal.w = 0;
	vout.normal = normalize(mul(vin.normal, vin.transform));

	float wind_range = 50;
	float wind_bend = 0.6;
	if (vin.texcoord.y < 0.2f)
	{
		float distance = abs(dot(position - something.wind_position, something.wind_direction));
		if (distance < wind_range)
		{
			position += something.wind_direction * wind_bend * (wind_range - distance) / wind_range;
		}
		vin.texcoord.y += 0.02;
	}

	float player_range = 3.5;
	float3 player_bend = float3(1.0, 0.25, 1.0);
	if (vin.texcoord.y < 0.2f)
	{
		float4  direction = (position - player.position);
		float distance = length(direction);
		direction = normalize(direction);
		if (distance < player_range)
		{
			float intensity = (player_range - distance) / player_range;
			position.xz += direction.xz * player_bend.xz * intensity;
			position.y -= player_bend.y * intensity;
		}
	}

	vout.sv_position = mul(position, camera_constants.view_projection);

	vout.texcoord.x = vin.texcoord.x;
	vout.texcoord.y = vin.texcoord.y;

	return vout;
}

