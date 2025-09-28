#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/ComPtr.h>

// directX
#include <d3d12.h>
// c++
#include <cstdint>

//============================================================================
//	DxUploadCommand class
//============================================================================
class DxUploadCommand {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxUploadCommand() = default;
	~DxUploadCommand() = default;

	void Create(ID3D12Device* device);

	void ExecuteCommands();

	//--------- accessor -----------------------------------------------------

	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12GraphicsCommandList> commandList_;
	ComPtr<ID3D12CommandAllocator> commandAllocator_;

	ComPtr<ID3D12CommandQueue> commandQueue_;

	ComPtr<ID3D12Fence> fence_;
	uint64_t fenceValue_;
	HANDLE fenceEvent_;

	//--------- functions ----------------------------------------------------

	void ResetCommand();
};