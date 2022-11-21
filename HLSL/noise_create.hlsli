struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

cbuffer cb_noise:register(b4)
{
	float cb_kind_of_noise;
	float cb_octaves;//...octaves : �m�C�Y���d�˂鐔
	float cb_amplitude;//...amplitude : �U�ꕝ
	float cb_param;
	float2 cb_mat0;
	float2 cb_mat1;
};
