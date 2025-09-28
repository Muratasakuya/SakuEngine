#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>

// directX
#include <d3d12.h>
// c++
#include <vector>
#include <cstdint>
#include <optional>
#include <cassert>

//============================================================================
//	DxConstBuffer class
//============================================================================
template<typename T>
class DxConstBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxConstBuffer() = default;
	~DxConstBuffer() = default;

	void CreateBuffer(ID3D12Device* device);

	void TransferData(const T& data);

	//--------- accessor -----------------------------------------------------

	ID3D12Resource* GetResource() const { return resource_.Get(); }

	bool IsCreatedResource() const { return isCreated_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
	T* mappedData_ = nullptr;

	bool isCreated_ = false;
};

//============================================================================
//	DxConstBuffer templateMethods
//============================================================================

template<typename T>
inline void DxConstBuffer<T>::CreateBuffer(ID3D12Device* device) {

	DxUtils::CreateBufferResource(device, resource_, sizeof(T));

	// マッピング
	HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
	assert(SUCCEEDED(hr));

	isCreated_ = true;
}

template<typename T>
inline void DxConstBuffer<T>::TransferData(const T& data) {

	if (mappedData_) {

		std::memcpy(mappedData_, &data, sizeof(T));
	}
}