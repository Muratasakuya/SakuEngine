#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>
#include <Engine/MathLib/Vector2.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>
#include <cassert>

//============================================================================
//	ComputePostProcessor class
//============================================================================
class ComputePostProcessor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ComputePostProcessor() = default;
	~ComputePostProcessor() = default;

	void Init(ID3D12Device* device, class SRVDescriptor* srvDescriptor,
		uint32_t width, uint32_t height);

	//--------- accessor -----------------------------------------------------

	void SetProcessTexureGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

	ID3D12Resource* GetOutputTextureResource() const { return outputTextureResource_.Get(); }

	const Vector2& GetTextureSize() const { return textureSize_; }

	const D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGPUHandle() const { return uavGPUHandle_; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVGPUHandle() const { return srvGPUHandle_; }

	const D3D12_GPU_DESCRIPTOR_HANDLE& GetProcessTextureGPUHandle() const { return processTextureGPUHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Vector2 textureSize_;

	ComPtr<ID3D12Resource> outputTextureResource_;

	D3D12_GPU_DESCRIPTOR_HANDLE uavGPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle_;

	// renderTexture以外に使用するtexture
	D3D12_GPU_DESCRIPTOR_HANDLE processTextureGPUHandle_;

	//--------- functions ----------------------------------------------------

	void CreateTextureResource(ID3D12Device* device, uint32_t width, uint32_t height);
};