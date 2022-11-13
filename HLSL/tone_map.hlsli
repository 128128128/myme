static const float gamma = 2.2;
static const float inv_gamma = 1.0 / 2.2;

// sRGB => XYZ => D65_2_D60 => AP1 => RRT_SAT
column_major const static matrix<float, 3, 3> aces_input_mat =
{
	0.59719, 0.07600, 0.02840,
	0.35458, 0.90834, 0.13383,
	0.04823, 0.01566, 0.83777
};

// ODT_SAT => XYZ => D60_2_D65 => sRGB
column_major const static matrix<float, 3, 3> aces_output_mat =
{
	1.60475, -0.10208, -0.00327,
	-0.53108, 1.10813, -0.07276,
	-0.07367, -0.00605, 1.07602
};

// ACES filmic tone map approximation
// see https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
float3 rrt_and_odt_fit(float3 color)
{
	float3 a = color * (color + 0.0245786) - 0.000090537;
	float3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
	return a / b;
}

// tone mapping 
float3 tonemap_aces_hill(float3 color)
{
	color = mul(color, aces_input_mat);

	// Apply RRT and ODT
	color = rrt_and_odt_fit(color);

	color = mul(color, aces_output_mat);

	// Clamp to [0, 1]
	color = clamp(color, 0.0, 1.0);

	return color;
}

// ACES tone map (faster approximation)
// see: https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
float3 tonemap_aces_narkowicz(float3 color)
{
	const float A = 2.51;
	const float B = 0.03;
	const float C = 2.43;
	const float D = 0.59;
	const float E = 0.14;
	return clamp((color * (A * color + B)) / (color * (C * color + D) + E), 0.0, 1.0);
}

//Tone map
//#define TONEMAP_ACES_NARKOWICZ
#define TONEMAP_ACES_HILL
//#define TONEMAP_ACES_HILL_EXPOSURE_BOOST
float3 tonemap(float3 color)
{
	color *= 1.0/*scene_data.exposure*/;

#ifdef TONEMAP_ACES_NARKOWICZ
	color = tonemap_aces_narkowicz(color);
#endif

#ifdef TONEMAP_ACES_HILL
	color = tonemap_aces_hill(color);
#endif

#ifdef TONEMAP_ACES_HILL_EXPOSURE_BOOST
	// boost exposure as discussed in https://github.com/mrdoob/three.js/pull/19621
	// this factor is based on the exposure correction of Krzysztof Narkowicz in his
	// implemetation of ACES tone mapping
	color /= 0.6;
	color = tonemap_aces_hill(color);
#endif

	return pow(color, inv_gamma);
}