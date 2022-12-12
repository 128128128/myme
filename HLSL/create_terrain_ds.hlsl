#include "create_terrain.hlsli"
#include "common.hlsli"

Texture2D heightmap : register(t0);
Texture2D colorramp : register(t1);
Texture2D noise_map : register(t2);

[domain("tri")]
DS_OUT main(HS_CONSTANT_DATA_OUTPUT input, float3 domain : SV_DomainLocation, const OutputPatch<HS_CONTROL_POINT_OUTPUT, CONTROL_POINT_COUNT> patch)
{
	DS_OUT output;

	output.position = patch[0].position * domain.x + patch[1].position * domain.y + patch[2].position * domain.z;
	output.position.w = 1;
	output.texcoord = patch[0].texcoord * domain.x + patch[1].texcoord * domain.y + patch[2].texcoord * domain.z;

	uint mip_level = 0, width, height, number_of_levels;
	//heightmap.GetDimensions(mip_level, width, height, number_of_levels);
	
	//get texture size
	noise_map.GetDimensions(mip_level, width, height, number_of_levels);

	uint u = output.texcoord.x * width;
	uint v = output.texcoord.y * height;
	//float altitude = heightmap.Load(int3(u, v, 0)).x;
	float altitude = noise_map.Load(int3(u, v, 0)).x;
#if 0
	const uint offset = 4;
	const float aspect_ratio = width / height;
	altitude += heightmap.Load(int3(u - offset * aspect_ratio, v, 0)).x;
	altitude += heightmap.Load(int3(u + offset * aspect_ratio, v, 0)).x;
	altitude += heightmap.Load(int3(u, v + offset * aspect_ratio, 0)).x;
	altitude += heightmap.Load(int3(u, v - offset * aspect_ratio, 0)).x;
	altitude += heightmap.Load(int3(u + offset * aspect_ratio, v - offset, 0)).x;
	altitude += heightmap.Load(int3(u + offset * aspect_ratio, v + offset, 0)).x;
	altitude += heightmap.Load(int3(u - offset * aspect_ratio, v + offset, 0)).x;
	altitude += heightmap.Load(int3(u - offset * aspect_ratio, v - offset, 0)).x;
	altitude /= 9.0;
#endif
	output.position.y = altitude;//altitude...çÇìx

	return output;
}