#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// driectX
#include <d3d12.h>
#include <dxgi1_6.h>
// c++
#include <array>
// front
class RTVDescriptor;
class DxDevice;
class DxCommand;

//============================================================================
//	DxSwapChain class
//============================================================================
class DxSwapChain {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxSwapChain() = default;
	~DxSwapChain() = default;

	void Create(class WinApp* winApp, IDXGIFactory7* factory,
		ID3D12CommandQueue* queue, RTVDescriptor* rtvDescriptor);

	//--------- accessor -----------------------------------------------------

	IDXGISwapChain4* Get() const { return swapChain_.Get(); }

	ID3D12Resource* GetCurrentResource() const;

	const DXGI_SWAP_CHAIN_DESC1& GetDesc() const { return desc_; }

	const RenderTarget& GetRenderTarget();
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	RenderTarget renderTarget_;

	// doubleBuffer
	static const constexpr uint32_t kBufferCount = 2;

	ComPtr<IDXGISwapChain4> swapChain_;
	DXGI_SWAP_CHAIN_DESC1 desc_{};

	std::array<ComPtr<ID3D12Resource>, kBufferCount> resources_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kBufferCount> rtvHandles_;
};