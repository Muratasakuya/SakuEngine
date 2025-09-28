#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
#include <Externals/DirectX12/d3dx12.h>
// c++
#include <cstdint>

//============================================================================
//	DxUtils namespace
//============================================================================
namespace DxUtils {

	//--------- functions ----------------------------------------------------

	void MakeDescriptorHeap(ComPtr<ID3D12DescriptorHeap>& descriptorHeap,
		ID3D12Device* device, const D3D12_DESCRIPTOR_HEAP_DESC& desc);

	void CreateBufferResource(ID3D12Device* device, ComPtr<ID3D12Resource>& resource, size_t sizeInBytes);
	void CreateUavBufferResource(ID3D12Device* device, ComPtr<ID3D12Resource>& resource, size_t sizeInBytes);
	void CreateReadbackBufferResource(ID3D12Device* device, ComPtr<ID3D12Resource>& resource, size_t sizeInBytes);

	bool CanAllocateIndex(uint32_t useIndex, uint32_t kMaxCount);

	UINT RoundUp(UINT round, UINT thread);
};