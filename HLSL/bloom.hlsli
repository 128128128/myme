
cbuffer CONSTANT_BUFFER : register(b0)
{
	float glow_extraction_threshold;//�P�x���o臒l
	float blur_convolution_intensity;//�u���[����
	float lens_flare_threshold;//�t���A臒l
	float lens_flare_ghost_dispersal;//lens ghost ��U
	int number_of_ghosts;//ghost ��
	float lens_flare_intensity;//�t���A�̋���
	int2 options;
}