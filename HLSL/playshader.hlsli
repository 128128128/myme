struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

cbuffer FOR_PLAY:register(b4)
{
	float4 mouse; //xy..coord, zw..push buttom
	float coverage;
	float boolrain;
	float cloud_speed;
	float sky_state;
	float4 top_sky_color;
	float4 under_sky_color;
};



