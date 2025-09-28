#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
// c++
#include <cassert>

//============================================================================
//	AccelerationStructureBuffer class
//============================================================================
class AccelerationStructureBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	AccelerationStructureBuffer() = default;
	~AccelerationStructureBuffer() = default;

	void Create(ID3D12Device* device, UINT64 sizeInBytes,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON,
		D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);

	//--------- accessor -----------------------------------------------------

	ID3D12Resource* GetResource()  const { return resource_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
};