#include "SRVDescriptor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>
#include <Engine/Core/Debug/Assert.h>

//============================================================================
//	SRVDescriptor classMethods
//============================================================================

void SRVDescriptor::CreateSRV(uint32_t& srvIndex, ID3D12Resource* resource,
	const D3D12_SHADER_RESOURCE_VIEW_DESC& desc) {

	srvIndex = Allocate(); // 参照で飛び出し側のIndexも設定する
	device_->CreateShaderResourceView(resource, &desc, GetCPUHandle(srvIndex));
}

void SRVDescriptor::CreateUAV(uint32_t& uavIndex, ID3D12Resource* resource,
	const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc) {

	uavIndex = Allocate(); // 参照で飛び出し側のIndexも設定する
	device_->CreateUnorderedAccessView(resource, nullptr, &desc, GetCPUHandle(uavIndex));
}

void SRVDescriptor::IncrementIndex() {

	if (!DxUtils::CanAllocateIndex(useIndex_, maxDescriptorCount_)) {
		ASSERT(FALSE, "Cannot allocate more DescriptorCount, maximum count reached");
	}
	useIndex_++;
}