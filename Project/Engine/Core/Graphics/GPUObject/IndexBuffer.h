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
//	インデックスバッファ(IB)の作成/転送/ビュー提供を行う軽量ラッパー。
//============================================================================
class IndexBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	IndexBuffer() = default;
	virtual ~IndexBuffer() = default;

	// 指定数のインデックス用にリソースを確保し、ビュー情報を初期化する
	void CreateBuffer(ID3D12Device* device, UINT indexCount);

	// CPU側のインデックス配列をGPUへ転送する
	void TransferData(const std::vector<uint32_t>& data);

	//--------- accessor -----------------------------------------------------

	// IBVと内部リソースを取得する
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