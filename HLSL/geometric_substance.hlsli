#include "common.hlsli"

struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 w_position : POSITION;
	float4 w_normal : NORMAL;
	float4 w_tangent : TANGENT;
	float2 texcoord : TEXCOORD;
};

struct primitive_constants
{
	row_major float4x4 world;
	float4 tweak_color;
	int material;
	int skin;
	int2 pads;
};
cbuffer primitive_constants : register(b8)
{
	primitive_constants primitive_data;
}

static const uint PRIMITIVE_MAX_JOINTS = 512;
struct primitive_joint_constants
{
	row_major float4x4 matrices[PRIMITIVE_MAX_JOINTS];
};
cbuffer primitive_joint_constants : register(b9)
{
	primitive_joint_constants primitive_joint_data;
};



