cbuffer RENDER_CONSTANTS:register (b3)
{
	float ambient_intensity;
	float specular_intensity;
	float specular_power;

	uint tone_number = 2;
}