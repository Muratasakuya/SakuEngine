#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>

// directX
#include <d3d12.h>

//============================================================================
//	DxBlendState class
//	描画ターゲットごとのブレンド設定を生成し、D3D12のブレンド記述に反映する。
//============================================================================
class DxBlendState {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxBlendState() = default;
	~DxBlendState() = default;

	// 指定BlendModeに応じてD3D12_RENDER_TARGET_BLEND_DESCを組み立てる
	void Create(BlendMode blendMode, D3D12_RENDER_TARGET_BLEND_DESC& blendDesc);
};