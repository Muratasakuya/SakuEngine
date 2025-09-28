#pragma once

//============================================================================
//	include
//============================================================================
#include <Engine/Core/Graphics/DxLib/DxUtils.h>

// directX
#include <d3d12.h>

//============================================================================
//	DxReadbackBuffer class
//============================================================================
template <typename T>
class DxReadbackBuffer {
public:
	//========================================================================
	//	public Methods
	//========================================================================

	DxReadbackBuffer() = default;
	~DxReadbackBuffer() = default;
	
	void CreateBuffer(ID3D12Device* device);

	//--------- accessor -----------------------------------------------------

	ID3D12Resource* GetResource() const { return resource_.Get(); }

	const T& GetReadbackData() { return *mappedData_; }
private:
	//========================================================================
	//	private Methods
	//========================================================================

	//--------- variables ----------------------------------------------------

	ComPtr<ID3D12Resource> resource_;
	T* mappedData_ = nullptr;
};

//============================================================================
//	DxReadbackBuffer templateMethods
//============================================================================

template<typename T>
inline void DxReadbackBuffer<T>::CreateBuffer(ID3D12Device* device) {

	DxUtils::CreateReadbackBufferResource(device, resource_, sizeof(T));

	// マッピング
	HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
	assert(SUCCEEDED(hr));
}