#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>

// front
class DSVDescriptor;
class SRVDescriptor;

//============================================================================
//	DepthTexture class
//============================================================================
class DepthTexture {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DepthTexture() = default;
	~DepthTexture() = default;

	void Create(uint32_t width, uint32_t height,
		DSVDescriptor* dsvDescriptor, SRVDescriptor* srvDescriptor);

	//--------- accessor -----------------------------------------------------

	ID3D12Resource* GetResource() const { return resource_.Get(); }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return dsvCPUHandle_; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return srvGPUHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 深度
	ComPtr<ID3D12Resource> resource_;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle_; // DSV
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle_; // SRV
};