#include "ComputePostProcessor.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>

//============================================================================
//	ComputePostProcessor classMethods
//============================================================================

void ComputePostProcessor::CreateTextureResource(
	ID3D12Device* device, uint32_t width, uint32_t height) {

	// RenderTargetで設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                      // 横幅
	resourceDesc.Height = height;                                    // 縦幅
	resourceDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;            // フォーマット設定
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS; // UAVとして利用
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;     // 2Dテクスチャで設定
	resourceDesc.SampleDesc.Count = 1;                               // サンプリングカウント。1固定
	resourceDesc.DepthOrArraySize = 1;                               // 配列サイズまたは深度を1に設定
	resourceDesc.MipLevels = 1;                                      // 1で設定、しなくてもdefaultで1になるらしい

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		nullptr,
		IID_PPV_ARGS(&outputTextureResource_));
	assert(SUCCEEDED(hr));
}

void ComputePostProcessor::Init(ID3D12Device* device, SRVDescriptor* srvDescriptor,
	uint32_t width, uint32_t height) {

	textureSize_.x = static_cast<float>(width);
	textureSize_.y = static_cast<float>(height);

	// uavResource作成
	CreateTextureResource(device, width, height);

	// UAV作成
	uint32_t uavIndex = 0;
	// Descの設定
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
	uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	srvDescriptor->CreateUAV(uavIndex, outputTextureResource_.Get(), uavDesc);
	uavGPUHandle_ = srvDescriptor->GetGPUHandle(uavIndex);

	// SRV作成
	uint32_t srvIndex = 0;
	// Descの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDescriptor->CreateSRV(srvIndex, outputTextureResource_.Get(), srvDesc);
	srvGPUHandle_ = srvDescriptor->GetGPUHandle(srvIndex);
}

void ComputePostProcessor::SetProcessTexureGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle) {

	processTextureGPUHandle_ = gpuHandle;
}