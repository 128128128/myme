
struct material_info
{
	float4 basecolor;

	float ior;
	float perceptual_roughness; // roughness value, as authored by the model creator (input to shader)//���f���쐬�҂ɐ��삳�ꂽ���t�l�X�̒l
	float3 f0; // full reflectance color (n incidence angle)

	float alpha_roughness; // roughness mapped to a more linear change in the roughness (proposed by [2])//�e������蒼���I�ɕω�����悤�Ƀ}�b�s���O����Ă���
	float3 c_diff;

	float3 f90; // reflectance color at grazing angle////�����ސ��O�̊p�x�ł̃J���[
	float metallic;//metallic param

	float pure_white;

	float occlusion_factor;
	float occlusion_strength;

	float3 emissive_factor;

	float sheen_roughness_factor;
	float3 sheen_color_factor;

	float3 clearcoat_f0;
	float3 clearcoat_f90;
	float clearcoat_factor;
	float3 clearcoat_normal;
	float clearcoat_roughness;

	// KHR_materials_specular 
	float specular_weight; // product of specularFactor and specularTexture.a

	float transmission_factor;

	float thickness_factor;
	float3 attenuation_color;
	float attenuation_distance;
};