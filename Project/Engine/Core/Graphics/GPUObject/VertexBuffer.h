#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>

// c++
#include <vector>

//============================================================================
//	VertexBuffer class
//============================================================================
template<typename T>
class VertexBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	VertexBuffer() = default;
	virtual ~VertexBuffer() = default;

	void CreateBuffer(ID3D12Device* device, UINT vertexCount);

	void TransferData(const std::vector<T>& data);

	//--------- accessor -----------------------------------------------------

	const D3D12_VERTEX_BUFFER_VIEW& GetVertexBufferView() const { return vertexBufferView_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
	T* mappedData_ = nullptr;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;
};

//============================================================================
//	VertexBuffer templateMethods
//============================================================================

template<typename T>
inline void VertexBuffer<T>::CreateBuffer(ID3D12Device* device, UINT vertexCount) {

	HRESULT hr;

	if (vertexCount > 0) {

		// 頂点データサイズ
		UINT sizeVB = static_cast<UINT>(sizeof(T) * vertexCount);

		// 定数バッファーのリソース作成
		DxUtils::CreateBufferResource(device, resource_, sizeVB);

		// 頂点バッファビューの作成
		vertexBufferView_.BufferLocation = resource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = sizeVB;
		vertexBufferView_.StrideInBytes = sizeof(T);

		// マッピング
		hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
		assert(SUCCEEDED(hr));
	}
}

template<typename T>
inline void VertexBuffer<T>::TransferData(const std::vector<T>& data) {

	if (mappedData_) {

		std::memcpy(mappedData_, data.data(), sizeof(T) * data.size());
	}
}