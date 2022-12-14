
//scene something constants
struct SOMETHING
{
	float iTime;
	float triple_speed_iTime;
	float elapsedTime;
	float shadow_map_num;
	float4x4 mlvp[1][3];
};

struct PLAYER
{
	float4 position;
	float4 direction;
};

//point light constants
struct POINT_LIGHT
{
	float4 color;
	float3 position;
	float range;
};

//spot constants buffer
struct SPOT_LIGHT
{
	float3 position;
	float pad;
	float3 color;
	float range;
	float3 direction;
	float angle;
};

struct HEMISPHERE_LIGHT
{
	float4 sky_color;
	float4 ground_color;
	float4 ground_normal;
};

//camera constants buffer
struct CAMERA_CONSTANTS
{
	float4 position;
	float4 direction;
	row_major float4x4 view;
	row_major float4x4 projection;
	row_major float4x4 view_projection;
	row_major float4x4 inverse_view;
	row_major float4x4 inverse_view_projection;
};

//light constants buffer
struct LIGHT_CONSTANTS
{
	float4 direction;
	float4 color;
};

cbuffer CAMERA_CONSTANT_BUFFER:register(b1)
{
	CAMERA_CONSTANTS camera_constants;
}

cbuffer LIGHT_CONSTANT_BUFFER : register(b2)
{
	LIGHT_CONSTANTS light_direction;
}

cbuffer SCENE_CONSTANT_BUFFER : register(b3)
{
	SOMETHING something;
	PLAYER player;
};


cbuffer LIGHT_CONSTANT_BUFFER : register(b4)
{
	POINT_LIGHT point_light;
	SPOT_LIGHT spot_light;
	HEMISPHERE_LIGHT hemisphere_light;
};