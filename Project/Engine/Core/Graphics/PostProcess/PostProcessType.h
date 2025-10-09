#pragma once

//============================================================================
//	include
//============================================================================

// c++
#include <cstdint>

//============================================================================
//	PostProcessType enum class
//============================================================================

// postProcessの種類
enum class PostProcessType {

	CopyTexture,
	Bloom,
	HorizontalBlur,
	VerticalBlur,
	RadialBlur,
	GaussianFilter,
	BoxFilter,
	Dissolve,
	Random,
	Vignette,
	Grayscale,
	SepiaTone,
	LuminanceBasedOutline,
	DepthBasedOutline,
	Lut,
	Glitch,
	CRTDisplay,

	Count
};
static constexpr uint32_t kPostProcessCount = static_cast<uint32_t>(PostProcessType::Count);

// 特定のポストエフェクトのビット
enum class PostEffectBit :
	uint16_t {

	None = 0,
	Bloom = 1 << 0,
	HorizontalBlur = 1 << 1,
	VerticalBlur = 1 << 2,
	RadialBlur = 1 << 3,
	GaussianFilter = 1 << 4,
	BoxFilter = 1 << 5,
	Dissolve = 1 << 6,
	Random = 1 << 7,
	Vignette = 1 << 8,
	Grayscale = 1 << 9,
	SepiaTone = 1 << 10,
	LuminanceBasedOutline = 1 << 11,
	DepthBasedOutline = 1 << 12,
	Lut = 1 << 13,
	Glitch = 1 << 14,
	CRTDisplay = 1 << 15,
};