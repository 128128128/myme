
struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
	float4 bone_weights : WEIGHTS;
	uint4 bone_indices : BONES;
};

struct VS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float3 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
};

static const int MAX_BONES = 128;
cbuffer OBJECT_CONSTANT_BUFFER : register(b0)
{
	row_major float4x4 world;
	row_major float4x4 bone_transforms[MAX_BONES];
};


cbuffer PBR_CONSTANTS:register(b3)
{
	float metallic_param;
	float smooth_param;
	float2 pad;
};