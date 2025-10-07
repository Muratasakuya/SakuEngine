#include "DepthTexture.h"

//============================================================================
//  include
//============================================================================
#include <Engine/Core/Graphics/Descriptors/DSVDescriptor.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>

//============================================================================
//	DepthTexture classMethods
//============================================================================

void DepthTexture::Create(uint32_t width, uint32_t height,
	DSVDescriptor* dsvDescriptor, SRVDescriptor* srvDescriptor) {

	// DSV作成
	dsvDescriptor->CreateDSV(width, height, dsvCPUHandle_, resource_,
		DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT);

	// SRV作成
	uint32_t srvIndex = 0;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Texture2D.MipLevels = 1;

	srvDescriptor->CreateSRV(srvIndex, resource_.Get(), srvDesc);
	srvGPUHandle_ = srvDescriptor->GetGPUHandle(srvIndex);
}