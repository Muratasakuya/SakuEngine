#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>

// directX
#include <d3d12.h>

//============================================================================
//	DxBlendState class
//============================================================================
class DxBlendState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxBlendState() = default;
	~DxBlendState() = default;

	void Create(BlendMode blendMode, D3D12_RENDER_TARGET_BLEND_DESC& blendDesc);
};