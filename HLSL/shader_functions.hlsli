static const float PI = 3.1415926f;         // ��

//------lim light------
//N...normal
//E...camera dir Vec(normalize)
//L...light dir(normalize)
//C...light color
//limpower...limlight power
float3 Calc_lim_light(float3 N,float3 E,float3 L,float3 C,float limpower=3.0f)
{
    float rim = 1.0f - saturate(dot(N, -E));
    return C * pow(rim, limpower) * saturate(dot(L, -E));
}
//Calculate the Beckmann distribution
// Beckmann���z���v�Z����
float Beckmann(float m, float t)
{
    float t2 = t * t;
    float t4 = t * t * t * t;
    float m2 = m * m;
    float D = 1.0f / (4.0f * m2 * t4);
    D *= exp((-1.0f / m2) * (1.0f - t2) / t2);
    return D;
}

//Calculate Fresnel, using Schlick approximation
// �t���l�����v�Z�BSchlick�ߎ����g�p
float SpcFresnel(float f0, float u)
{
    // from Schlick
    return f0 + (1 - f0) * pow(1 - u, 5);
}


/// Calculate specular reflections for Cook-Torrance model : Cook-Torrance���f���̋��ʔ��˂��v�Z
/// <"L">Vector toward light source : �����Ɍ������x�N�g��
/// <"V">Vector toward the viewpoint : ���_�Ɍ������x�N�g��
/// <"N">normal : �@���x�N�g��
/// <"metallic">�����x
float CookTorranceSpecular(float3 L, float3 V, float3 N, float metallic)
{
    float microfacet = 0.76f;

    //Metallicity is treated as Fresnel reflectance at perpendicular incidence
    //The higher the metallicity, the larger the Fresnel reflection.
    // �����x�𐂒����˂̎��̃t���l�����˗��Ƃ��Ĉ���
    // �����x�������قǃt���l�����˂͑傫���Ȃ�
    float f0 = metallic;

    //Find the half vector of the vector toward the light and the vector toward the line of sight
    // ���C�g�Ɍ������x�N�g���Ǝ����Ɍ������x�N�g���̃n�[�t�x�N�g�������߂�
    float3 H = normalize(L + V);

    //Use inner product to determine how similar various vectors are
    // �e��x�N�g�����ǂꂭ�炢���Ă��邩����ς𗘗p���ċ��߂�
    float NdotH = saturate(dot(N, H));
    float VdotH = saturate(dot(V, H));
    float NdotL = saturate(dot(N, L));
    float NdotV = saturate(dot(N, V));

    //Calculate the D term using the Beckman distribution
    // D�����x�b�N�}�����z��p���Čv�Z����
    float D = Beckmann(microfacet, NdotH);

    //Compute the F term using the Schlick approximation
    // F����Schlick�ߎ���p���Čv�Z����
    float F = SpcFresnel(f0, VdotH);

    //Find the G term
    // G�������߂�
    float G = min(1.0f, min(2 * NdotH * NdotV / VdotH, 2 * NdotH * NdotL / VdotH));

    // Find the m term
    float m = PI * NdotV * NdotH;

    //Using the values obtained up to this point, we obtain the specular reflection of the Cook-Torrance model
    // �����܂ŋ��߂��A�l�𗘗p���āACook-Torrance���f���̋��ʔ��˂����߂�
    return max(F * D * G / m, 0.0);
}


/// �t���l�����˂��l�������g�U���˂��v�Z
/// ���̊֐��̓t���l�����˂��l�������g�U���˗����v�Z���܂�
/// �t���l�����˂́A�������̂̕\�ʂŔ��˂��錻�ۂ̂Ƃ��ŁA���ʔ��˂̋����ɂȂ�܂�
/// ����g�U���˂́A�������̂̓����ɓ����āA�����������N�����āA�g�U���Ĕ��˂��Ă������̂���
/// �܂�t���l�����˂��ア�Ƃ��ɂ́A�g�U���˂��傫���Ȃ�A�t���l�����˂������Ƃ��́A�g�U���˂��������Ȃ�܂�
/// <param name="N">normal</param>
/// <param name="L">to light source�Bopposite light direction(���̕����Ƌt�����̃x�N�g��) </param>
/// <param name="V">to eye vector(�����Ɍ������x�N�g��)
//float CalcDiffuseFromFresnel(float3 N, float3 L, float3 E)
//{
//    //�t���l�����˂��l�������g�U���ˌ������߂�
//
//    // �@���ƌ����Ɍ������x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
//    float dotNL = saturate(dot(N, L));
//
//    // �@���Ǝ����Ɍ������x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
//    float dotNV = saturate(dot(N, E));
//
//    // �@���ƌ����ւ̕����Ɉˑ�����g�U���˗��ƁA�@���Ǝ��_�x�N�g���Ɉˑ�����g�U���˗���
//    // ��Z���čŏI�I�Ȋg�U���˗������߂Ă���BPI�ŏ��Z���Ă���̂͐��K�����s������
//    return (dotNL * dotNV);
//}
float CalcDiffuseFromFresnel(float3 N, float3 L, float3 V)
{
    //Serious implementation of Disney-based diffuse reflection with Fresnel reflection.
    // Find the half vector of vectors toward the light source and toward the line of sight
    // �f�B�Y�j�[�x�[�X�̃t���l�����˂ɂ��g�U���˂�^�ʖڂɎ�������B
    // �����Ɍ������x�N�g���Ǝ����Ɍ������x�N�g���̃n�[�t�x�N�g�������߂�
    float3 H = normalize(L + V);

    //Roughness is fixed at 0.5.
    // �e����0.5�ŌŒ�B
    float roughness = 0.5f;

    float energy_bias = lerp(0.0f, 0.5f, roughness);
    float energy_factor = lerp(1.0, 1.0 / 1.51, roughness);

    //Find the inner product of how similar the vector toward the light source and the half vector are.
    // �����Ɍ������x�N�g���ƃn�[�t�x�N�g�����ǂꂾ�����Ă��邩����ςŋ��߂�
    float dotLH = saturate(dot(L, H));

    //The vector toward the light source and the half vector, the
    // the amount of diffuse reflection when the light is incident parallel to the light source
    // �����Ɍ������x�N�g���ƃn�[�t�x�N�g���A
    // �������s�ɓ��˂����Ƃ��̊g�U���˗ʂ����߂Ă���
    float Fd90 = energy_bias + 2.0 * dotLH * dotLH * roughness;

    //Find the diffuse reflectance using the normal and the vector w toward the light source.
    // �@���ƌ����Ɍ������x�N�g��w�𗘗p���Ċg�U���˗������߂�
    float dotNL = saturate(dot(N, L));
    float FL = (1 + (Fd90 - 1) * pow(1 - dotNL, 5));

    //Calculate diffuse reflectance using normals and vectors toward the viewpoint
    // �@���Ǝ��_�Ɍ������x�N�g���𗘗p���Ċg�U���˗������߂�
    float dotNV = saturate(dot(N, V));
    float FV = (1 + (Fd90 - 1) * pow(1 - dotNV, 5));

    //The diffuse reflectance, which depends on the normal and the direction to the light source, and the diffuse reflectance, which depends on the normal and the viewpoint vector, are
    //The final diffuse reflectance is obtained by multiplying the diffuse reflectance that depends on the normal and the direction to the light source by the diffuse reflectance that depends on the normal and the viewpoint vector.
    //Translated with www.DeepL.com / Translator(free version)
    // �@���ƌ����ւ̕����Ɉˑ�����g�U���˗��ƁA�@���Ǝ��_�x�N�g���Ɉˑ�����g�U���˗���
    // ��Z���čŏI�I�Ȋg�U���˗������߂Ă���BPI�ŏ��Z���Ă���̂͐��K�����s������
    return (FL * FV * energy_factor);
}
