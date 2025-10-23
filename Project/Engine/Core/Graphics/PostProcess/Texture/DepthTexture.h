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
//	DepthTexture class
//	深度テクスチャの生成(DSV/SRV)とハンドル提供を行う軽量ラッパー。
//============================================================================
class DepthTexture {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DepthTexture() = default;
	~DepthTexture() = default;

	// 指定サイズで深度リソースを作成し、DSVとSRVを生成する

	void Create(uint32_t width, uint32_t height,
		DSVDescriptor* dsvDescriptor, SRVDescriptor* srvDescriptor);

	//--------- accessor -----------------------------------------------------

	// 作成した深度リソースと各種ハンドルを取得する
	ID3D12Resource* GetResource() const { return resource_.Get(); }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCPUHandle() const { return dsvCPUHandle_; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return srvGPUHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	// 深度
	ComPtr<ID3D12Resource> resource_;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvCPUHandle_; // DSV
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle_; // SRV
};