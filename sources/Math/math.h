#pragma once
#include <algorithm>

static constexpr float PI = 3.14159265358979323846f;

namespace math
{
    inline const float& clamp(const float& val, const float& lo, const float& hi)
    {
        return  (std::min)((std::max)(val, lo), hi);
    }
	/// <summary>
	/// �p�x��Degree�P�ʂ���Radian�P�ʂɕϊ�����B
	/// </summary>
	/// <param name="deg">Degree�P�ʂ̊p�x�B</param>
	/// <returns>Radian�P�ʂ̊p�x�B</returns>
	constexpr static inline float deg_to_rad(float deg)
	{
		return deg * (PI / 180.0f);
	}
}
