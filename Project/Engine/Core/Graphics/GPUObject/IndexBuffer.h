#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>

// c++
#include <vector>
#include <cassert>

//============================================================================
//	IndexBuffer class
//============================================================================
class IndexBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IndexBuffer() = default;
	virtual ~IndexBuffer() = default;

	void CreateBuffer(ID3D12Device* device, UINT indexCount);

	void TransferData(const std::vector<uint32_t>& data);

	//--------- accessor -----------------------------------------------------

	const D3D12_INDEX_BUFFER_VIEW& GetIndexBufferView() const { return indexBufferView_; }

	ID3D12Resource* GetResource() const { return resource_.Get(); }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
	uint32_t* mappedData_ = nullptr;

	D3D12_INDEX_BUFFER_VIEW indexBufferView_;
};