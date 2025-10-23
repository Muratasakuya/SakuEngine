#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>
#include <Engine/MathLib/Vector2.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>
#include <cassert>

//============================================================================
//	ComputePostProcessor class
//	計算シェーダ用の出力テクスチャ(UAV/SRV)を用意し、ポストプロセス入力/出力を仲介する。
//============================================================================
class ComputePostProcessor {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	ComputePostProcessor() = default;
	~ComputePostProcessor() = default;

	// UAV出力テクスチャを作成し、UAV/SRVディスクリプタを割り当てる
	void Init(ID3D12Device* device, class SRVDescriptor* srvDescriptor,
		uint32_t width, uint32_t height);

	//--------- accessor -----------------------------------------------------

	// 外部入力として利用するテクスチャ(SRV)のGPUハンドルを設定する
	void SetProcessTexureGPUHandle(const D3D12_GPU_DESCRIPTOR_HANDLE& gpuHandle);

	// 出力テクスチャのリソースを取得する
	ID3D12Resource* GetOutputTextureResource() const { return outputTextureResource_.Get(); }

	// 出力テクスチャのサイズを取得する
	const Vector2& GetTextureSize() const { return textureSize_; }

	// 出力テクスチャのUAV/SRVハンドルを取得する
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGPUHandle() const { return uavGPUHandle_; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVGPUHandle() const { return srvGPUHandle_; }
	// 入力に設定された処理元テクスチャのSRVハンドルを取得する
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetProcessTextureGPUHandle() const { return processTextureGPUHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	Vector2 textureSize_;

	ComPtr<ID3D12Resource> outputTextureResource_;

	D3D12_GPU_DESCRIPTOR_HANDLE uavGPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle_;

	// renderTexture以外に使用するtexture
	D3D12_GPU_DESCRIPTOR_HANDLE processTextureGPUHandle_;

	//--------- functions ----------------------------------------------------

	// UAVとして使用可能な2Dテクスチャを確保する
	void CreateTextureResource(ID3D12Device* device, uint32_t width, uint32_t height);
};