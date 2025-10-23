#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// front
class RTVDescriptor;
class SRVDescriptor;

//============================================================================
//	RenderTexture class
//	RTV/SRV(UAV)を持つ汎用レンダーターゲット。作成とハンドル提供を行う。
//============================================================================
class RenderTexture {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	RenderTexture() = default;
	~RenderTexture() = default;

	// 指定サイズ/形式/初期クリア色でテクスチャを作成し、RTVとSRV(UAV)を生成する
	void Create(uint32_t width, uint32_t height, const Color& color,
		DXGI_FORMAT format, ID3D12Device* device, RTVDescriptor* rtvDescriptor, SRVDescriptor* srvDescriptor,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);

	//--------- accessor -----------------------------------------------------

	// リソース/RTV情報/GPUハンドルを取得する

	ID3D12Resource* GetResource() const { return resource_.Get(); }

	const RenderTarget& GetRenderTarget() const { return renderTarget_; }

	const D3D12_GPU_DESCRIPTOR_HANDLE& GetSRVGPUHandle() const { return srvGPUHandle_; }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetUAVGPUHandle() const { return uavGPUHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	RenderTarget renderTarget_;

	ComPtr<ID3D12Resource> resource_;
	D3D12_GPU_DESCRIPTOR_HANDLE srvGPUHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE uavGPUHandle_;

	static int textureCount_;

	//--------- functions ----------------------------------------------------

	// 内部ヘルパ: リソースを確保し、必要に応じクリア値を設定する
	void CreateTextureResource(ComPtr<ID3D12Resource>& resource, uint32_t width, uint32_t height,
		const Color& color, DXGI_FORMAT format, D3D12_RESOURCE_FLAGS flags, ID3D12Device* device);
};

//============================================================================
//	GuiRenderTexture class
//	GUI描画結果の転送先となるSRV専用テクスチャ。コピー用に使用する。
//============================================================================
class GuiRenderTexture {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	GuiRenderTexture() = default;
	~GuiRenderTexture() = default;

	// 指定サイズ/形式でSRV専用のテクスチャを作成する
	void Create(uint32_t width, uint32_t height, DXGI_FORMAT format,
		ID3D12Device* device, SRVDescriptor* srvDescriptor);

	//--------- accessor -----------------------------------------------------

	// リソースとSRVハンドルを取得する
	ID3D12Resource* GetResource() const { return resource_.Get(); }
	const D3D12_GPU_DESCRIPTOR_HANDLE& GetGPUHandle() const { return gpuHandle_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;

	static int textureCount_;

	//--------- functions ----------------------------------------------------

	// 内部ヘルパ: SRV前提のテクスチャを確保する
	void CreateTextureResource(ComPtr<ID3D12Resource>& resource, uint32_t width, uint32_t height,
		DXGI_FORMAT format, ID3D12Device* device);
};