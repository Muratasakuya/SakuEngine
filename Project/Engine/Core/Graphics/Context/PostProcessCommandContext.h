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
//============================================================================
class PostProcessCommandContext {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	PostProcessCommandContext() = default;
	~PostProcessCommandContext() = default;

	void Execute(PostProcessType type, ID3D12GraphicsCommandList* commandList,
		class ComputePostProcessor* processor,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputTextureGPUHandle,
		const D3D12_GPU_DESCRIPTOR_HANDLE& inputMaskTextureGPUHandle);
};