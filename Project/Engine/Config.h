#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/MathLib/Vector4.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>

//============================================================================
//	Config namespace
//============================================================================
namespace  Config {

	// windowTitle
	const constexpr wchar_t* kWindowTitle = L"BLADESLASH";
	const constexpr char* kWindowTitleName = "BLADESLASH";
	// fullscreen
	const constexpr bool kFullscreenEnable = false;

	// windowSize
	// uint
	const constexpr uint32_t kWindowWidth = 1920;
	const constexpr uint32_t kWindowHeight = 1080;
	// float
	const constexpr float kWindowWidthf = static_cast<float>(kWindowWidth);
	const constexpr float kWindowHeightf = static_cast<float>(kWindowHeight);

	// clearColor
	const float kWindowClearColor[] = { Color::Convert(0x181818ff).r,
		Color::Convert(0x181818ff).g, Color::Convert(0x181818ff).b, 1.0f };
	// shadowMap...値が大きい方が精度が上がる
	const constexpr uint32_t kShadowMapSize = 128;

	// swapChainFormat
	const constexpr DXGI_FORMAT kSwapChainRTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// renderTargetFormat
	const constexpr DXGI_FORMAT kRenderTextureRTVFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;

	// instanceMax
	const constexpr uint32_t kMaxInstanceNum = 1024;
};