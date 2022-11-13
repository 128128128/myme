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
	/// 角度をDegree単位からRadian単位に変換する。
	/// </summary>
	/// <param name="deg">Degree単位の角度。</param>
	/// <returns>Radian単位の角度。</returns>
	constexpr static inline float deg_to_rad(float deg)
	{
		return deg * (PI / 180.0f);
	}
}
