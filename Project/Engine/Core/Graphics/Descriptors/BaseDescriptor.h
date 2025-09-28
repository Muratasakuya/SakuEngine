#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>

//============================================================================
//	structure
//============================================================================

// descriptorの種類
struct DescriptorType {

	D3D12_DESCRIPTOR_HEAP_TYPE heapType;
	D3D12_DESCRIPTOR_HEAP_FLAGS heapFlags;
};
enum class DescriptorHeapType {

	SRV,
	RTV,
	DSV
};

//============================================================================
//	BaseDescriptor class
//============================================================================
class BaseDescriptor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	BaseDescriptor(uint32_t maxDescriptorCount);
	virtual ~BaseDescriptor() = default;

	void Init(ID3D12Device* device, const DescriptorType& descriptorType);

	//--------- accessor -----------------------------------------------------

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

	ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap_.Get(); }

	uint32_t GetUseDescriptorCount() const { return useIndex_; }
	uint32_t GetMaxDescriptorCount() const { return maxDescriptorCount_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	uint32_t descriptorSize_;
protected:
	//========================================================================
	//	protected Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ID3D12Device* device_;

	uint32_t maxDescriptorCount_;
	uint32_t useIndex_;

	ComPtr<ID3D12DescriptorHeap> descriptorHeap_;

	//--------- functions ----------------------------------------------------

	uint32_t Allocate();
};