#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector4.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>
#include <string>
#include <array>

//============================================================================
//	DxStructures
//============================================================================

// 描画先の情報
struct RenderTarget {

	uint32_t width;
	uint32_t height;
	Color clearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
};

enum BlendMode {

	kBlendModeNormal,   // 通常αブレンド
	kBlendModeAdd,      // 加算
	kBlendModeSubtract, // 減算
	kBlendModeMultiply, // 乗算
	kBlendModeScreen,   // スクリーン
	kBlendModeCount
};
constexpr std::array<BlendMode, static_cast<size_t>(BlendMode::kBlendModeCount)>
CreateBlendModeTypes() {
	std::array<BlendMode, static_cast<size_t>(BlendMode::kBlendModeCount)> types = {};
	for (uint32_t i = 0; i < static_cast<uint32_t>(BlendMode::kBlendModeCount); ++i) {
		types[i] = static_cast<BlendMode>(i);
	}
	return types;
}
static constexpr uint32_t blendModeNum = static_cast<uint32_t>(BlendMode::kBlendModeCount);
static constexpr
std::array<BlendMode, static_cast<size_t>(BlendMode::kBlendModeCount)>
blendModeTypes = CreateBlendModeTypes();

namespace Blend {

	// imgui選択
	void SelectBlendMode(BlendMode& blendMode, const std::string& label = "label");
}

enum class UVAddressMode :
	int32_t {

	WRAP = 0,
	CLAMP = 1
};

namespace UVAddress {

	// imgui選択
	void SelectUVAddressMode(UVAddressMode& adressMode, const std::string& label = "label");
}