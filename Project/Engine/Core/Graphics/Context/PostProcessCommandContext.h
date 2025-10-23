#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/PostProcess/PostProcessType.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>

//============================================================================
//	PostProcessCommandContext class
//	ポストプロセス用のコマンド設定/発行を管理する。
//============================================================================
class PostProcessCommandContext {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PostProcessCommandContext() = default;
	~PostProcessCommandContext() = default;

	// ポストプロセス種別に応じてディスパッチ/入出力を設定し実行する。
	void Execute(PostProcessType type, ID3D12GraphicsCommandList* commandList,
		class ComputePostProcessor* processor,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputTextureGPUHandle,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskTextureGPUHandle);
};