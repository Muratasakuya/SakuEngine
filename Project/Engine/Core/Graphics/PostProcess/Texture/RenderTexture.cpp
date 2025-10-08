#include "RenderTexture.h"

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/Descriptors/RTVDescriptor.h>
#include <Engine/Core/Graphics/Descriptors/SRVDescriptor.h>

//============================================================================
//	RenderTexture classMethods
//============================================================================

int RenderTexture::textureCount_ = 0;

void RenderTexture::CreateTextureResource(ComPtr<ID3D12Resource>& resource,
	uint32_t width, uint32_t height,
	const Color& color, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, ID3D12Device* device) {

	// RenderTargetで設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                   // 横幅
	resourceDesc.Height = height;                                 // 縦幅
	resourceDesc.Format = format;                                 // フォーマット設定
	resourceDesc.Flags = flags;                                   // フラグ設定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2Dテクスチャで設定
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント。1固定
	resourceDesc.DepthOrArraySize = 1;                            // 配列サイズまたは深度を1に設定
	resourceDesc.MipLevels = 1;                                   // 1で設定、しなくてもdefaultで1になるらしい

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// 色
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = format;
	clearValue.Color[0] = color.r;
	clearValue.Color[1] = color.g;
	clearValue.Color[2] = color.b;
	clearValue.Color[3] = color.a;

	// クリアカラーを使用するか
	const bool needsClearValue =
		(flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 ||
		(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		needsClearValue ? D3D12_RESOURCE_STATE_RENDER_TARGET : D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
		needsClearValue ? &clearValue : nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
}

void RenderTexture::Create(uint32_t width, uint32_t height, const Color& color,
	DXGI_FORMAT format, ID3D12Device* device, RTVDescriptor* rtvDescriptor,
	SRVDescriptor* srvDescriptor, D3D12_RESOURCE_FLAGS flags) {

	// renderTargetの設定
	renderTarget_.width = width;
	renderTarget_.height = height;
	// リニア変換した値を設定
	renderTarget_.clearColor = color.ToLinear();

	// texture作成
	CreateTextureResource(resource_, width, height, renderTarget_.clearColor, format, flags, device);

	// renderTargetとして使用するときのみ
	const bool useRenderTarget =
		(flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET) != 0 ||
		(flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) != 0;
	if (useRenderTarget) {

		resource_->SetName((L"renderTexture" + std::to_wstring(textureCount_)).c_str());
		// RTV作成
		// Descの設定
		D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
		rtvDesc.Format = format;
		rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rtvDescriptor->Create(renderTarget_.rtvHandle, resource_.Get(), rtvDesc);

		++textureCount_;
	}

	// SRV作成
	uint32_t srvIndex = 0;
	// Descの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDescriptor->CreateSRV(srvIndex, resource_.Get(), srvDesc);
	srvGPUHandle_ = srvDescriptor->GetGPUHandle(srvIndex);

	// 使用用途がRenderTarget以外の場合
	if (!useRenderTarget) {

		// UAVを作成する
		uint32_t uavIndex = 0;
		// Descの設定
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = format;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;
		srvDescriptor->CreateUAV(uavIndex, resource_.Get(), uavDesc);
		uavGPUHandle_ = srvDescriptor->GetGPUHandle(uavIndex);
	}
}

//============================================================================
//	GuiRenderTexture classMethods
//============================================================================

int GuiRenderTexture::textureCount_ = 0;

void GuiRenderTexture::CreateTextureResource(ComPtr<ID3D12Resource>& resource,
	uint32_t width, uint32_t height, DXGI_FORMAT format, ID3D12Device* device) {

	// RenderTargetで設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                   // 横幅
	resourceDesc.Height = height;                                 // 縦幅
	resourceDesc.Format = format;                                 // フォーマット設定
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;                // SRVとして利用
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;  // 2Dテクスチャで設定
	resourceDesc.SampleDesc.Count = 1;                            // サンプリングカウント。1固定
	resourceDesc.DepthOrArraySize = 1;                            // 配列サイズまたは深度を1に設定
	resourceDesc.MipLevels = 1;                                   // 1で設定、しなくてもdefaultで1になるらしい

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
}

void GuiRenderTexture::Create(uint32_t width, uint32_t height,
	DXGI_FORMAT format, ID3D12Device* device, SRVDescriptor* srvDescriptor) {

	// texture作成
	CreateTextureResource(resource_, width, height, format, device);

	// SRV作成
	uint32_t srvIndex = 0;
	// Descの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDescriptor->CreateSRV(srvIndex, resource_.Get(), srvDesc);
	gpuHandle_ = srvDescriptor->GetGPUHandle(srvIndex);

	++textureCount_;
}