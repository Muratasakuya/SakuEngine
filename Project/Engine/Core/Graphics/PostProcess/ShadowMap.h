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
//	ShadowMap class
//============================================================================
class ShadowMap {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ShadowMap() = default;
	~ShadowMap() = default;

	void Create(uint32_t width, uint32_t height, DSVDescriptor* dsvDescriptor, SRVDescriptor* srvDescriptor);

	//--------- accessor -----------------------------------------------------

	ID3D12Resource* GetResource() const { return resource_.Get(); }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return cpuHandle_; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return gpuHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;
};