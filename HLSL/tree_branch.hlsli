struct VS_OUT
{
	float4 sv_position : SV_POSITION;
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float4 color: COLOR0;
    float4 vertexToFrag332: COLOR1;
};

struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float4 tangent : TANGENT;
	float4 binormal : BINORMAL;
	float2 texcoord : TEXCOORD;
	float4 color:COLOR;
};

cbuffer OBJECT_CONSTANTS : register(b0)
{
	column_major float4x4 world;
	float4 color;
}

cbuffer WIND_CONSTANTS:register(b5)
{
	float max_wind_strength;
	float wind_strength;//���̋���
	float wind_speed;//���̑���
	float mask;//�}�X�N
	float3 wind_direction;//���̌���
	float wind_amplitude;//���̐U�ꕝ
	float wind_amplitude_multiplier;
	float flat_lighting;//�t���b�g���C�e�B���O
	float wind_debug;//debug�p//wind noise texture display
	float pad;
}

Texture2D wind_vectors:register(t4);

#define _AmbientOcclusion 0
#define _HueVariation float4(1, 0.5, 0, 0.184)
#define _TransmissionColor float4(1,1,1,0)
#define _AmbientOcclusion 1
#define _GradientBrightness 1
#define _Smoothness 0
#define _LightColor0 float4(1,1,1,1)