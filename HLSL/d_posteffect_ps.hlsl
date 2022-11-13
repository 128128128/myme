#include "d_posteffect.hlsli"
#include "scene_constants.hlsli"
#include "tone_map.hlsli"

Texture2D ShadowMap : register(t14);
SamplerState ShadowSampler : register(s14);

//	texture
Texture2D diffuse_texture : register(t0);
Texture2D depth_texture : register(t1);
Texture2D normal_texture : register(t2);
Texture2D position_texture : register(t3);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);


// Bloom
float3 bloom(float2 uv)
{
	float threshold = 0.5; //threshold(これ以下は消す)
	float ratio = (1.0 / (1.0 - threshold));

	float3 ret = 0; //start from darkness
	int num = 7; //Blur frequency
	for (int i = 0; i < num; i++) {
		float uv_x = cb_bloom * 0.003 * (i + 1);
		float uv_y = cb_bloom * 0.002 * (i + 1);
		float rate = (1.0 - (i * (1.0 / num)));
		// right
		float4 tex = diffuse_texture.Sample(sampler_states[0], uv + float2(uv_x, 0));
		// Leave above threshold 閾値以上を残す
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;

		// left
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(-uv_x, 0));
		//  Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;

		// top
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(0, -uv_y));
		//  Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;
		// bottom
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(0, uv_y));
		//  Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;

		// right-upper
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(uv_x, -uv_y) * 0.7);
		//  Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;
		// left-upper
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(-uv_x, -uv_y) * 0.7);
		// Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;
		// lower right
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(uv_x, uv_y) * 0.7);
		// Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;
		// lower left
		tex = diffuse_texture.Sample(sampler_states[0], uv + float2(-uv_x, uv_y) * 0.7);
		// Leave above threshold
		tex.rgb = (tex.rgb - threshold) * ratio;
		tex.rgb = max(0, tex.rgb) * rate; //0以下を0に 遠いほど暗い
		ret += tex.rgb;
	}
	// central pixel
	float4 center = diffuse_texture.Sample(sampler_states[0], uv);
	// Leave above threshold
	float4 tex = center;
	tex.rgb = (tex.rgb - threshold) * ratio;
	tex.rgb = max(0, tex.rgb); //0以下を0に
	ret += tex.rgb;

	ret /= num * 8 + 1; //normalize 足した回数で割る(正規化)
	ret *= cb_bloom;
	// simple Bloom = 元のピクセル + まぶしい光
	ret = center.rgb + ret;
	return ret;
}

// ColorGrading
float3 color_grading(float3 color)
{
	//float contrast = 1.5; //contrast
	//float saturation = 0.5; // saturation(彩度)
	//float3 balance = float3(0.9, 1.0, 1.05);
	
	// contrast(明暗の差)
	color = (color - 0.5) * contrast + 0.5;
	// saturation
	float gray = (color.r + color.g + color.b) / 3.0;
	color = (color - gray) * saturation + gray;

	color *= colorlv.rgb;
	return color;
}

// Vignette
float3 vignette(float3 color, float2 position)
{
	// 中心からの距離
	float rate = length(position - 0.5);
	rate *= 1.5; // 全体調整
	rate += (cb_vignette - 1) * 0.5 - 0.1; // 黒範囲調整
	rate = pow(rate, 3.0); // 黒→白の幅調整
	color = lerp(color, float3(0, 0, 0), saturate(rate));
	return color;
}

//Outline
float outline(float2 uv) {
	float depth = depth_texture.Sample(sampler_states[0], uv).r;
	float dR = depth_texture.Sample(sampler_states[0], uv + float2(0.001, 0));
	float dD = depth_texture.Sample(sampler_states[0], uv + float2(0, 0.0015)).r;
	float d = abs(depth - dR);
	float d2 = abs(depth - dD);
	if (d > 0.1 * depth) return 0;
	if (d2 > 0.1 * depth) return 0;
	// 法線に差がある→輪郭
	float3 N = normal_texture.
		Sample(sampler_states[0], uv).xyz;
	float3 NR = normal_texture.Sample(sampler_states[0], uv
		+ float2(0.001, 0)).xyz;
	N = N * 2.0 - 1.0; //0-1 => -1 - +1
	NR = NR * 2.0 - 1.0;
	if (dot(N, NR) < 0.1) return 0;

	return 1;
}

float4 main(VS_OUT pin) : SV_TARGET0{
	float4 tex = diffuse_texture.Sample(sampler_states[0], pin.texcoord);

	//float4 depth = depth_texture.Sample(sampler_states[0], input.tex);

	float4 color = tex;
	// Bloom
	color.rgb = bloom(pin.texcoord);

	// ColorGrading
	if(cb_color_grading>0.3)
	color.rgb = color_grading(color.rgb);

	//Outline
	if(cb_outline>0.3){
	color.rgb *= outline(pin.texcoord);
	float rate = outline(pin.texcoord);
	color.rgb = lerp(float3(1,1,0), color.rgb, rate);
	}
	// Vignette
	color.rgb = vignette(color.rgb, pin.texcoord);

	//color.rgb = depth;

	// tone mapping(0~1に収める)
	uint mip_level = 0, width, height, number_of_levels;
	diffuse_texture.GetDimensions(mip_level, width, height, number_of_levels);


	// Apply tone mapping.
	color.rgb = tonemap(color.rgb);
	// gamma corection
	pow(color.rgb, 1.0 / 2.2);

	return color;
}


