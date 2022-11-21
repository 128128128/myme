
cbuffer CONSTANT_BUFFER : register(b0)
{
	float glow_extraction_threshold;//輝度抽出閾値
	float blur_convolution_intensity;//ブラー強さ
	float lens_flare_threshold;//フレア閾値
	float lens_flare_ghost_dispersal;//lens ghost 飛散
	int number_of_ghosts;//ghost 数
	float lens_flare_intensity;//フレアの強さ
	int2 options;
}