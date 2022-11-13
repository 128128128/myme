#include "shadow_df.hlsli"

//texture
Texture2D DiffuseTexture : register(t0);
SamplerState DecalSampler : register(s0);

float4 main(PS_IN pin) : SV_TARGET
{
	float4 color = 0;
	float2 adjust = 0;
	float sum = 0;
	for (int y = -3; y <= 3; y++) {
		for (int x = -3; x <= 3; x++) {
			adjust = float2(x, y);

			float d = length(adjust) / 3.5;
			float rate = 1.0 - saturate(d);
			rate = pow(rate, 0.5); //0.5æ
			sum += rate;

			float4 tex = DiffuseTexture.Sample(DecalSampler,
				pin.texcoord + adjust * 0.0005);
			color += tex * rate;
		}
	}
	color /= sum; //average..•½‹Ï
	color.a = 1;
	return color;
}