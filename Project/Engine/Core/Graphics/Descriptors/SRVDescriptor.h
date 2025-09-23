#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Descriptors/BaseDescriptor.h>

//============================================================================
//	SRVDescriptor class
//============================================================================
class SRVDescriptor :
	public BaseDescriptor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	SRVDescriptor() :BaseDescriptor(256) {};
	~SRVDescriptor() = default;

	void CreateSRV(uint32_t& srvIndex, ID3D12Resource* resource,
		const D3D12_SHADER_RESOURCE_VIEW_DESC& desc);

	void CreateUAV(uint32_t& uavIndex, ID3D12Resource* resource,
		const D3D12_UNORDERED_ACCESS_VIEW_DESC& desc);

	void IncrementIndex();

	//--------- accessor -----------------------------------------------------

	uint32_t GetMaxSRVCount() const { return maxDescriptorCount_; }
};