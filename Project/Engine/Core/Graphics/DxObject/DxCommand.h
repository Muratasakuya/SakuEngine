#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxStructures.h>
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
#include <dxgi1_6.h>
// c++
#include <cstdint>
#include <vector>
#include <array>
#include <optional>
#include <chrono>
#include <thread>
#include <future>

//============================================================================
//	DxCommand class
//============================================================================
class DxCommand {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxCommand() = default;
	~DxCommand() = default;

	void Create(ID3D12Device* device);

	void ExecuteCommands(IDXGISwapChain4* swapChain);

	void WaitForGPU();

	void Finalize(HWND hwnd);

	void SetDescriptorHeaps(const std::vector<ID3D12DescriptorHeap*>& descriptorHeaps);

	void SetRenderTargets(const std::optional<RenderTarget>& renderTarget,
		const std::optional<D3D12_CPU_DESCRIPTOR_HANDLE>& dsvHandle = std::nullopt);

	void ClearDepthStencilView(const D3D12_CPU_DESCRIPTOR_HANDLE& dsvHandle);

	void SetViewportAndScissor(uint32_t width, uint32_t height);

	void TransitionBarriers(const std::vector<ID3D12Resource*>& resources,
		D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter);

	void UAVBarrier(ID3D12Resource* resource);
	void UAVBarrierAll();

	void CopyTexture(ID3D12Resource* dstResource, D3D12_RESOURCE_STATES dstState,
		ID3D12Resource* srcResource, D3D12_RESOURCE_STATES srcState);

	//--------- accessor -----------------------------------------------------

	ID3D12CommandQueue* GetQueue() const { return commandQueue_.Get(); }

	ID3D12GraphicsCommandList6* GetCommandList() const { return commandList_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12GraphicsCommandList6> commandList_;
	ComPtr<ID3D12CommandAllocator> commandAllocator_;

	ComPtr<ID3D12CommandQueue> commandQueue_;

	ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_;
	HANDLE fenceEvent_;

	std::chrono::steady_clock::time_point reference_;

	//--------- functions ----------------------------------------------------

	void ExecuteGraphicsCommands(IDXGISwapChain4* swapChain);

	void FenceEvent();

	void UpdateFixFPS();
};