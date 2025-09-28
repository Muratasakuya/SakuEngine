#include "BaseDescriptor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Core/Debug/Assert.h>

//============================================================================
//	BaseDescriptor classMethods
//============================================================================

BaseDescriptor::BaseDescriptor(uint32_t maxDescriptorCount) :
	maxDescriptorCount_(maxDescriptorCount) {
}

void BaseDescriptor::Init(ID3D12Device* device, const DescriptorType& descriptorType) {

	device_ = nullptr;
	device_ = device;

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};

	heapDesc.Type = descriptorType.heapType;
	heapDesc.NumDescriptors = maxDescriptorCount_;
	heapDesc.Flags = descriptorType.heapFlags;

	// descriptor生成
	DxUtils::MakeDescriptorHeap(descriptorHeap_, device, heapDesc);
	descriptorSize_ = device->GetDescriptorHandleIncrementSize(descriptorType.heapType);
}

D3D12_CPU_DESCRIPTOR_HANDLE BaseDescriptor::GetCPUHandle(uint32_t index) const {

	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_.Get()->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize_ * index);
	return handleCPU;
}

D3D12_GPU_DESCRIPTOR_HANDLE BaseDescriptor::GetGPUHandle(uint32_t index) const {

	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_.Get()->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize_ * index);
	return handleGPU;
}

uint32_t BaseDescriptor::Allocate() {

	if (!DxUtils::CanAllocateIndex(useIndex_, maxDescriptorCount_)) {
		ASSERT(FALSE, "Cannot allocate more DescriptorCount, maximum count reached");
	}

	int index = useIndex_;
	useIndex_++;

	return index;
}